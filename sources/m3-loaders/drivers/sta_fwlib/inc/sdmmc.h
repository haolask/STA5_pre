/**
 * @file sdmmc.h
 * @brief This file provides the generic high layer interface for SD MMC IP
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: APG-MID team
 */

#ifndef _SDMMC_H_
#define _SDMMC_H_

#include "sta_map.h"

#define MMC_BLOCK_POWER		9
#define MMC_BLOCK_SIZE		BIT(MMC_BLOCK_POWER)
#define MMC_TRANSFERT_MAX_SIZE	(65535 * MMC_BLOCK_SIZE)

#define USER_PART	0
#define BOOT1_PART	1
#define BOOT2_PART	2

/* SD/MMC Commands ID */
#define SDMMC_CMD0	0
#define SDMMC_CMD1	1
#define SDMMC_CMD2	2
#define SDMMC_CMD3	3
#define SDMMC_CMD4	4
#define SDMMC_CMD6	6
#define SDMMC_CMD7	7
#define SDMMC_CMD8	8
#define SDMMC_CMD9	9
#define SDMMC_CMD10	10
#define SDMMC_CMD12	12
#define SDMMC_CMD13	13
#define SDMMC_CMD16	16
#define SDMMC_CMD17	17
#define SDMMC_CMD18	18
#define SDMMC_CMD21	21
#define SDMMC_CMD23	23
#define SDMMC_CMD24	24
#define SDMMC_CMD25	25
#define SDMMC_CMD26	26
#define SDMMC_CMD27	27
#define SDMMC_CMD35	35
#define SDMMC_CMD36	36
#define SDMMC_CMD38	38
#define SDMMC_CMD41	41
#define SDMMC_CMD42	42
#define SDMMC_CMD48	48
#define SDMMC_CMD51	51
#define SDMMC_CMD55	55

#define MMC_ERRORBITS_MSK	0xFDFFE008

enum mmc_card_type {
	MMC_CARD = 0x10,
	MMC_MULTIMEDIA_CARD = MMC_CARD,
	MMC_MULTIMEDIA_HC_CARD,

	SD_CARD = 0x20,
	MMC_SECURE_DIGITAL_CARD = SD_CARD,
	MMC_SECURE_DIGITAL_HC_CARD,

	SDIO_CARD = 0x40,
	MMC_SECURE_DIGITAL_IO_CARD = SDIO_CARD,
	MMC_SECURE_DIGITAL_IO_COMBO_CARD,
};

/**
 * The harwdware partition numero:
 */
enum e_hw_part_num {
	MMC_PART_USER	= 0,	/* Standard User partition */
	MMC_PART_BOOT1	= 1,	/* Boot partition 1 */
	MMC_PART_BOOT2	= 2,	/* Boot partition 2 */
	MMC_PART_RPMB	= 3,	/* RPMB partition */
	MMC_PART_GP1	= 4,	/* General purpose partition 1 */
	MMC_PART_GP2	= 5,	/* General purpose partition 2 */
	MMC_PART_GP3	= 6,	/* General purpose partition 3 */
	MMC_PART_GP4	= 7,	/* General purpose partition 4 */
};

#define IS_SDCARD(mmc)		((mmc)->cardtype & SD_CARD)
#define IS_MMC(mmc)		((mmc)->cardtype & MMC_CARD)
#define IS_SDIO(mmc)		((mmc)->cardtype & SDIO_CARD)

/* Possible voltages */
#define IO_VOLT_180		0x1  /* 1.8V IO voltage */
#define IO_VOLT_330		0x2  /* 3.3V IO voltage */

/* MMC errors codes */
#define MMC_GENERIC_ERROR	-255
#define MMC_TIMEOUT_ERROR	-256
#define MMC_CRC_ERROR		-257
#define MMC_IO_ERROR		-258
#define MMC_START_BIT_ERROR	-259
#define MMC_UNALIGNED_ERROR	-260
#define MMC_STATUS_ERROR	-261
#define MMC_NOT_SUPPORTED_ERROR	-262
/* RPMB specific errors */
#define MMC_RPMB_BAD_RESP		-512
#define MMC_RPMB_GENERAL_FAILURE_ERROR	-513
#define MMC_RPMB_AUTH_FAILURE_ERROR	-514
#define MMC_RPMB_COUNTER_FAILURE_ERROR	-515
#define MMC_RPMB_ADDR_FAILURE_ERROR	-516
#define MMC_RPMB_WRITE_FAILURE_ERROR	-517
#define MMC_RPMB_READ_FAILURE_ERROR	-518
#define MMC_RPMB_KEY_NOT_PROGRAMMED	-519

/* Possible response types */
#define MMC_RSP_PRES		BIT(0)
#define MMC_NOCRC		BIT(1)
#define MMC_LONG_RSP		BIT(2)
#define MMC_BUSY_RSP		BIT(4)
#define MMC_OPCODE_RSP		BIT(5)

#define MMC_RESPONSE_R1		(MMC_RSP_PRES | MMC_OPCODE_RSP)
#define MMC_RESPONSE_R1B	(MMC_RSP_PRES | MMC_BUSY_RSP | MMC_OPCODE_RSP)
#define MMC_RESPONSE_R2		(MMC_RSP_PRES | MMC_LONG_RSP)
#define MMC_RESPONSE_R3		(MMC_RSP_PRES | MMC_NOCRC)
#define MMC_RESPONSE_R4		(MMC_RSP_PRES | MMC_NOCRC)
#define MMC_RESPONSE_R5		(MMC_RSP_PRES | MMC_OPCODE_RSP)
#define MMC_RESPONSE_R6		(MMC_RSP_PRES | MMC_OPCODE_RSP)
#define MMC_RESPONSE_R7		(MMC_RSP_PRES | MMC_OPCODE_RSP)

struct mmc_ops; /* forward decl. */

/* MMC system context */
struct t_mmc_ctx {
	const struct mmc_ops	*ops;
	uint32_t		port;
	uint32_t		base;
	uint32_t		cid[4];
	uint32_t		csd[4];
	uint32_t		ocr;
	uint32_t		rca;
	uint32_t		resp[4];
	uint32_t		bus_width;
	uint32_t		blksize;
	uint32_t		cur_blksize;
	uint8_t			extcsd[512];
	bool			extcsd_valid;
	enum mmc_card_type	cardtype;
	int			rst_pin;
};

enum t_mmc_port {
	SDMMC0_PORT,
	SDMMC1_PORT,
	SDMMC2_PORT,
	SDMMC_NOPORT = -1
};

/* Forced MMC port to boot on, by default disabled (SDMMC_NOPORT) */
#define SDMMC_FORCED_PORT SDMMC_NOPORT

/**
 * @brief	MMC operations definition
 */
struct mmc_ops {
	int (*hw_init)(struct t_mmc_ctx *mmc, uint32_t io_volt);
	int (*send_cmd)(struct t_mmc_ctx *mmc, uint32_t cmd, uint32_t arg,
			uint32_t resp_type, uint32_t size);
	int (*data_cmd)(struct t_mmc_ctx *mmc, uint32_t cmd,
			uint32_t sector, void *buffer, uint32_t length);
	int (*read_extcsd)(struct t_mmc_ctx *mmc, uint8_t *p_extcsd);
	int (*change_frequency)(struct t_mmc_ctx *mmc, uint32_t clock);
	void (*set_bus_width)(struct t_mmc_ctx *mmc, uint32_t bus_width);
	int (*init_boot)(struct t_mmc_ctx *mmc, uint32_t size,
			 uint32_t *buffer, uint32_t clock);

};

enum t_mmc_bus_width {
	MMC_1_BIT_WIDE = 0,
	MMC_4_BIT_WIDE,
	MMC_8_BIT_WIDE
};

/** @brief ExtCSD registers structure definition */
struct mmc_ext_csd_t {
	uint8_t ext_sec_err;		/** @brief 1byte 505 */
	uint8_t s_cmd_set;		/** @brief 1byte 504 */
	uint8_t hpi_features;		/** @brief 1byte 503 */
	uint8_t bkops_support;		/** @brief 1byte 502 */
	uint8_t max_packed_reads;	/** @brief 1byte 501 */
	uint8_t max_packed_writes;	/** @brief 1byte 500 */
	uint8_t data_tag_support;	/** @brief 1byte 499 */
	uint8_t tag_unit_size;		/** @brief 1byte 498 */
	uint8_t tag_res_size;		/** @brief 1byte 497 */
	uint8_t context_capab;		/** @brief 1byte 496 */
	uint8_t large_unit_size_m1;	/** @brief 1byte 495 */
	uint8_t ext_support;		/** @brief 1byte 494 */
	uint8_t supported_modes;	/** @brief 1byte 493 */
	uint8_t ffu_features;		/** @brief 1byte 492 */
	uint8_t op_codes_timeout;	/** @brief 1byte 491 */
	uint32_t ffu_arg;		/** @brief 4bytes 490 -487 */
	uint8_t cmdq_support;		/** @brief 1byte 308 */
	uint8_t cmdq_depth;		/** @brief 1byte 307 */
	uint32_t rcvd_sector_cnt;	/** @brief 4bytes 305 -302 */
	uint8_t vendor_prop_health_report[32];  /** @brief 32 bytes 301-270 */
	uint8_t dev_lifetime_typeB;	/** @brief 1byte 269 */
	uint8_t dev_lifetime_typeA;	/** @brief 1byte 268 */
	uint8_t pre_eol_info;		/** @brief 1byte 267 */
	uint8_t optimal_rd_size;	/** @brief 1byte 266 */
	uint8_t optimal_wr_size;	/** @brief 1byte 265 */
	uint8_t optimal_trim_unit_size; /** @brief 1byte 264 */
	uint16_t dev_version;		/** @brief 2byte 263-262 */
	uint8_t fw_version[8];		/** @brief 8bytes 261-254 */
	uint8_t pwr_cl_200_360;		/** @brief 1byte 253 */
	uint32_t cache_size;		/** @brief 4byte 252-249 */
	uint8_t generic_cmd6_time;	/** @brief 1 byte 248 */
	uint8_t pwr_off_long_time;	/** @brief 1byte 247 */
	uint8_t bkops_status;		/** @brief 1byte 246 */
	uint32_t correctly_prg_sectors; /** @brief 4bytes 245-242 */
	uint8_t ini_timeout_ap;		/** @brief 1byte 241 */
	uint8_t pwr_cl_ddr_52_360;	/** @brief 1byte 239 */
	uint8_t pwr_cl_ddr_52_195;	/** @brief 1byte 238 */
	uint8_t pwr_cl_200_195;		/** @brief 1byte 237 */
	uint8_t pwr_cl_200_130;		/** @brief 1byte 236 */
	uint8_t min_perf_ddr_w8_52;	/** @brief 1byte 235 */
	uint8_t min_perf_ddr_r8_52;	/** @brief 1byte 234 */
	uint8_t trim_mult;		/** @brief 1byte 232 */
	uint8_t sec_support;		/** @brief 1byte 231 */
	uint8_t sec_erase_mult;		/** @brief 1byte 230 */
	uint8_t sec_trim_mult;		/** @brief 1byte 229 */
	uint8_t boot_info;		/** @brief 1byte 228 */
	uint8_t boot_size_mult;		/** @brief 1byte 226 */
	uint8_t access_size;		/** @brief 1byte 225 */
	uint8_t hc_erase_grp_size;	/** @brief 1byte 224 */
	uint8_t erase_timeout_multi;	/** @brief 1byte 223 */
	uint8_t rel_wr_sec_cnt;		/** @brief 1byte 222 */
	uint8_t hc_wp_grp_size;		/** @brief 1byte 221 */
	uint8_t sleep_cur_vcc;		/** @brief 1byte 220 */
	uint8_t sleep_cur_vccq;		/** @brief 1byte 219 */
	uint8_t psa_timeout;		/** @brief 1byte 218 */
	uint8_t slp_awk_timeout;	/** @brief 1byte 217 */
	uint8_t slp_notify_time;	/** @brief 1byte 216 */
	uint32_t sec_count;		/** @brief 4byte 215-212 */
	uint8_t min_pref_w_8_52;	/** @brief 1byte 210 */
	uint8_t min_pref_r_8_52;	/** @brief 1byte 209 */
	uint8_t min_pref_w_8_26_4_52;	/** @brief 1byte 208 */
	uint8_t min_pref_r_8_26_4_52;	/** @brief 1byte 207 */
	uint8_t min_pref_w_4_26;	/** @brief 1byte 206 */
	uint8_t min_pref_r_4_26;	/** @brief 1byte 205 */
	uint8_t pwr_cl_26_360;		/** @brief 1byte 203 */
	uint8_t pwr_cl_52_360;		/** @brief 1byte 202 */
	uint8_t pwr_cl_26_195;		/** @brief 1byte 201 */
	uint8_t pwr_cl_52_195;		/** @brief 1byte 200 */
	uint8_t part_switch_time;	/** @brief 1byte 199 */
	uint8_t out_of_intr_time;	/** @brief 1byte 198 */
	uint8_t driver_strength;	/** @brief 1byte 197 */
	uint8_t card_type;		/** @brief 1byte 196 */
	uint8_t csd_structure;		/** @brief 1byte 194 */
	uint8_t ext_csd_rev;		/** @brief 1byte 192 */

	/** @briefModes segment */

	uint8_t cmd_Set;		/** @brief 1byte 191  R/W */
	uint8_t cmd_set_rev;		/** @brief 1byte 189 */
	uint8_t power_class;		/** @brief 1byte 187 */
	uint8_t hs_timing;		/** @brief 1byte 185 */
	uint8_t strobe_support;		/** @brief 1byte 184 */
	uint8_t bus_width;		/** @brief 1byte 183 */
	uint8_t erased_mem_cont;	/** @brief 1byte 181 */

	union {
		struct {
			uint8_t  part_access :3;
			uint8_t  boot_parten :3;
			uint8_t  boot_ack    :1;
			uint8_t  reserved0   :1;
		} bit;
		uint16_t reg;
	} boot_config;			/** @brief 1byte 179 */
	uint8_t boot_config_prot;	/** @brief 1byte 178 */
	uint8_t boot_bus_width;         /** @brief 1byte 177 */
	uint8_t erase_grp_defn;         /** @brief 1byte 175 */
	uint8_t boot_wp_status;		/** @brief 1byte 174 */
	uint8_t boot_wp;		/** @brief 1byte 173 */
	uint8_t user_wp;		/** @brief 1byte 171 */
	uint8_t fw_config;		/** @brief 1byte 169 */
	uint8_t rpmb_size_mult;		/** @brief 1byte 168 */
	uint8_t wr_rel_set;		/** @brief 1byte 167 */
	uint8_t wr_rel_param;		/** @brief 1byte 166 */
	uint8_t sanitize_start;		/** @brief 1byte 165 */
	uint8_t bkops_start;		/** @brief 1byte 164 */
	uint8_t bkops_en;		/** @brief 1byte 163 */
	uint8_t rst_n_func;		/** @brief 1byte 162 */
	uint8_t hpi_mgmt;		/** @brief 1byte 161 */
	uint8_t part_support;		/** @brief 1byte 160 */
	uint32_t max_enh_size_mult;	/** @brief 3bytes 159-157 */
	uint8_t part_attrb;		/** @brief 1byte 156 */
	uint8_t part_set_complete;	/** @brief 1byte 155 */
	uint32_t GP_size_mult0;		/** @brief 3byte 143-145 */
	uint32_t GP_size_mult1;		/** @brief 3byte 146-148 */
	uint32_t GP_size_mult2;		/** @brief 3byte 149-151 */
	uint32_t GP_size_mult3;		/** @brief 3byte 152-154 */
	uint32_t enh_size_mult;		/** @brief 3byte 142-140 */
	uint32_t enh_start_addr;	/** @brief 4byte 139-136 */
	uint8_t sec_bad_blk_mngt;	/** @brief 1byte 134 */
	uint8_t prdt_state_awareness;	/** @brief 1byte 133 */
	uint8_t tcase_support;		/** @brief 1byte 132 */
	uint8_t periodic_wakeup;	/** @brief 1byte 131 */
	uint8_t prg_cid_csd_ddr;	/** @brief 1byte 130 */
	uint8_t vendor_specific[64];	/** @brief 64bytes 127-64 */
	uint8_t native_sector_size;	/** @brief 1byte 63 */
	uint8_t use_native_sector;	/** @brief 1byte 62 */
	uint8_t data_sector_size;	/** @brief 1byte 61 */
	uint8_t ini_timeout_emu;	/** @brief 1byte 60 */
	uint8_t class_6_ctrl;		/** @brief 1byte 59 */
	uint8_t dyncap_needed;		/** @brief 1byte 58 */
	uint16_t expt_evts_ctrl;	/** @brief 2bytes 57-56 */
	uint16_t expt_evts_status;	/** @brief 2bytes 55-54 */
	uint16_t ext_part_attrib;	/** @brief 2bytes 53-52 */
	uint8_t context_conf[15];	/** @brief 15bytes 51-37 */
	uint8_t packed_cmd_status;	/** @brief 1byte 36 */
	uint8_t packed_fail_index;	/** @brief 1byte 35 */
	uint8_t pwr_off_notify;		/** @brief 1byte 34 */
	uint8_t cache_ctrl;		/** @brief 1byte 33 */
	uint8_t flush_cache;		/** @brief 1 byte 32 */

	uint8_t mode_config;		/** @brief 1byte 30 */
	uint8_t mode_op_codes;		/** @brief 1byte 29 */

	uint8_t ffu_status;		/** @brief 1byte 26 */
	uint32_t preload_data_size;	/** @brief 4bytes 25-22 */
	uint32_t max_preload_data_size;	/** @brief 4bytes 21-18 */
	uint8_t psa_enablement;		/** @brief 1byte 17 */
	uint8_t sec_removal_type;	/** @brief 1byte 16 */
	uint8_t cmdq_mode_en;		/** @brief 1byte 15 */
};

/*
 * ---------------------------------------------------------------------------
 * Public Functions Prototype
 * --------------------------------------------------------------------------
 */

/**
 * @brief	Initialize an MMC instance
 * @param	port - the MMC port number to initialize
 * @param	force - to force full reinit
 * @return	mmc context or 0 if error
 */
struct t_mmc_ctx *sdmmc_init(enum t_mmc_port port, bool force);

/**
 * @brief	Free the given MMC instance
 * @param	mmc - the MMC instance to free
 * @return	None
 */
void sdmmc_remove(struct t_mmc_ctx *mmc);

int sdmmc_reset(enum t_mmc_port port);

int sdmmc_read(struct t_mmc_ctx *mmc, uint32_t sector,
	       void *buffer, uint32_t length);

int sdmmc_write(struct t_mmc_ctx *mmc, uint32_t sector,
		void *buffer, uint32_t length);

int sdmmc_init_boot(enum t_mmc_port port, uint8_t *buffer, uint32_t boot_size,
		    const char *pinmux, uint32_t bus_wide, bool high_speed);

/**
 * @brief	This function send commands to set the configuration partition
 * @param	mmc - the MMC instance to use
 * @param	part: hardware partition number to access or -1 if no change
 * @param	part_enable: boot partition to enable or -1 if no change
 * @param	bootack: ack required in boot operation or -1 if no change
 * @return	0 if no error, not 0 otherwise
 */
int sdmmc_set_part_config(struct t_mmc_ctx *mmc, int part,
			  int part_enable, int bootack);

int sdmmc_set_boot_part(uint32_t boot_mode,
			uint32_t rst_boot_bus_w, uint32_t boot_bus_width);

/**
 * RPMB intefaces
 */

/**
 * @brief	Write a RPMB frame
 *		First, you have to select RPMB partition
 * @param	mmc: mmc context to access
 * @param	frame: the rpmb frame to write
 * @param	reliable_write: set to true for an authenticated write
 * @return	0 if no error, not 0 otherwise
 */
int rpmb_request(struct t_mmc_ctx *mmc, const void *frame, bool reliable_write);

/**
 * @brief	Writing key (one time programming)
 *		First, you have to select RPMB partition
 * @param	mmc: mmc context to access
 * @param	key - The key to write
 * @return	0 if no error, not 0 otherwise
 */
int sdmmc_rpmb_write_key(struct t_mmc_ctx *mmc, const uint8_t *key);

/**
 * @brief	Reading counter number in mmc
 * @param	mmc: mmc context to access
 * @param	p_counter: returned counter
 * @return	0 if no error, not 0 otherwise
 */
int sdmmc_rpmb_read_counter(struct t_mmc_ctx *mmc, uint32_t *p_counter);

/**
 * @brief	Autenticated Read buffers
 * @param	mmc: mmc context to access
 * @param	address: RPMB block address to read
 * @param	buffer: output read buffer
 * @param	length: length to read
 * @param	key: The RPMB key to use (optionnal)
 * @return	0 if no error else error code
 */
int sdmmc_rpmb_read(struct t_mmc_ctx *mmc, uint32_t address,
		    void *buffer, uint32_t length, const uint8_t *key);
/**
 * @brief	Autenticated Write buffers
 * @param	mmc: mmc context to access
 * @param	address: RPMB block address to write
 * @param	buffer: input buffer to write
 * @param	length: length to write
 * @param	key: The RPMB key to use
 * @return	0 if no error else error code
 */
int sdmmc_rpmb_write(struct t_mmc_ctx *mmc, uint32_t address,
		     void *buffer, uint32_t length, const uint8_t *key);

/**
 * @brief  Basic SQI_tests
 */
void sdmmc_tests(void);

#endif

