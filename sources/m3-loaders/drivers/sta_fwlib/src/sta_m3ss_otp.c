/**
 * @file sta_m3ss_otp.c
 * @brief This file provides some OTP functions to handle M3 sub-system fuses
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: ADG-MID team
 */
#include <errno.h>
#include <stdbool.h>

#define WAIT_SAFEMEM_BUSY() {while (safemem->otp_status.bit.busy == 1); }
#define WAIT_SAFEMEM_ON() {while ((safemem->otp_status.bit.fuse_ok == 0) || \
				  (safemem->otp_status.bit.pwron == 0)); }

typedef struct {
	union {
		struct {
			unsigned int boot_safemem_bypass: 1;
			unsigned int boot_bypass_sequence: 1;
			unsigned int boot_peripheral: 4;

			union {
				struct {
					unsigned char nand_bus_width: 1;
					unsigned char nand_page_type: 1;
					unsigned char nand_gpio_config: 2;
					unsigned char nand_unused: 4;

				} nand_boot_option;

				struct {
					unsigned char sdmmc_boot_partition: 1;
					unsigned char sdmmc_bus_width: 2;
					unsigned char sdmmc_speed: 1;
					unsigned char sdmmc_gpio_config: 2;
					unsigned char sdmmc_unused: 2;

				} sdmmc_boot_option;

				struct {
					unsigned char serial_nor_gpio_config: 2;
					unsigned char serial_nor_unused: 6;

				} serial_nor_boot_option;

			} boot_peripheral_options;

			unsigned int boot_security_level: 4;
			unsigned int boot_unused: 14;

		} bit;

		unsigned int word;

	} safemem_boot_config;

} t_safemem_boot_cfg;

typedef struct {
	unsigned int  shadow[0x100];
	union {
		struct {
			unsigned int pwrup: 1;
			unsigned int frc: 2;
			unsigned int sw_frc_en: 1;
			unsigned int firewall: 1;
			unsigned int reserved: 27;
		} bit;
		unsigned int reg;
	} otp_config;

	union {
		struct {
			unsigned int addr: 8;
			unsigned int prog: 1;
			unsigned int lock: 1;
			unsigned int disturbcheck: 1;
			unsigned int reserved: 21;
		} bit;
		unsigned int reg;
	} otp_ctrl;

	union {
		struct {
			unsigned int pwron: 1;
			unsigned int busy: 1;
			unsigned int flag: 3;
			unsigned int fuse_ok: 1;
			unsigned int prog_fail: 1;
			unsigned int bist1_lock: 1;
			unsigned int bist2_lock: 1;
			unsigned int reserved: 23;
		} bit;
		unsigned int reg;
	} otp_status;

	unsigned int otp_wrdata;

	unsigned int otp_dist_check[0x8];

	unsigned int otp_error_flag[0x8];

	union {
		struct {
			unsigned int val: 16;
			unsigned int reserved: 16;
		} bit;
		unsigned int reg;
	} otp_wr_loked[0x10];

} t_safemem_register;

/* Global variables */
static unsigned int otp_boot_opt_word;
static t_safemem_boot_cfg *p_safemem_boot_cfg;
#define SAFEMEM_REG_START_ADDR 0x49100000
static t_safemem_register *safemem =
	(t_safemem_register *)SAFEMEM_REG_START_ADDR;

#define OTP_PRV_BOOT_INFO_SIZE 16
#define OTP_SPECIAL_BIT_WORD 63
#define OTP_SPECIAL_BIT_MASK 0x8000
#define OTP_UPPER_BOUNDARY_WORD 8
static bool otp_word_has_ecc(unsigned int word_addr)
{
	unsigned int word_read = 0;
	bool ecc_on = false;

	if (word_addr < OTP_PRV_BOOT_INFO_SIZE)
		return true;

	/* PowerOn OTP registers */
	safemem->otp_config.bit.pwrup = 1;

	/* Set the freq range */
	safemem->otp_config.bit.frc = 1;

	/* Check the cell is powered on from status register */
	WAIT_SAFEMEM_ON();

	/* Check SAFEMEM status register, SAFEMEM should not be busy  */
	WAIT_SAFEMEM_BUSY();

	/* Get the special bit */
	word_read = 0xFFFF & safemem->shadow[OTP_SPECIAL_BIT_WORD];

	if (word_read & OTP_SPECIAL_BIT_MASK) {
	/* Special bit is on, let's check current chunk protection */
		if (word_addr < OTP_UPPER_BOUNDARY_WORD)
			ecc_on = true;
		else
			ecc_on = word_read & (1 << (word_addr / 4 - 2));
	} else {
		ecc_on = false;
	}

	return ecc_on;
}

#define OTP_WORD_NR 64
static int otp_read_word(unsigned int word_addr, unsigned int *word_to_read)
{
	bool ecc_on;

	/* Allowed range is 0 - 63 */
	if (word_addr > (OTP_WORD_NR - 1) || !word_to_read)
		return -EINVAL;

	/* PowerOn OTP registers */
	safemem->otp_config.bit.pwrup = 1;

	/* Set the freq range */
	safemem->otp_config.bit.frc = 1;

	/* Check the cell is powered on from status register */
	WAIT_SAFEMEM_ON();

	/* Check SAFEMEM status register, SAFEMEM should not be busy  */
	WAIT_SAFEMEM_BUSY();

	/* Get the special bit */
	ecc_on = otp_word_has_ecc(word_addr);

	if (ecc_on)
		*word_to_read = safemem->shadow[word_addr];
	else
		*word_to_read = 0xFFFF & safemem->shadow[word_addr];

	return 0;
}

#define OTP_HI_BOOT_OPT_WORD 62
#define OTP_LO_BOOT_OPT_WORD 61
static int otp_get_boot_info(void)
{
	int err;
	unsigned int otp_boot_opt_half_word = 0;

	/* If word 62 had ECC, just read word 62, At contrary read 62 and 61 */
	if (otp_word_has_ecc(OTP_HI_BOOT_OPT_WORD)) {
		/*
		 * Read last two 16 bits words are used to store boot
		 * information
		 */
		err = otp_read_word(OTP_HI_BOOT_OPT_WORD, &otp_boot_opt_word);
		if (err)
			return err;
	} else {
		/*
		 * Read last two 16 bits words are used to store boot
		 * information
		 */
		err = otp_read_word(OTP_HI_BOOT_OPT_WORD,
				    &otp_boot_opt_half_word);
		if (err)
			return err;
		otp_boot_opt_word = (otp_boot_opt_half_word << 16);
		otp_boot_opt_half_word = 0;
		err = otp_read_word(OTP_LO_BOOT_OPT_WORD,
				    &otp_boot_opt_half_word);
		if (err)
			return err;
		otp_boot_opt_word |= (0xFFFF & otp_boot_opt_half_word);
	}
	p_safemem_boot_cfg = (t_safemem_boot_cfg *)&otp_boot_opt_word;
	return 0;
}

enum t_boot_security_level {
	BOOT_SECURITY_LEVEL_0 = 0,
	BOOT_SECURITY_LEVEL_1 = 1,
	BOOT_SECURITY_LEVEL_2 = 2,
	BOOT_SECURITY_LEVEL_3 = 3,
	BOOT_SECURITY_LEVEL_4 = 4,
	BOOT_SECURITY_LEVEL_5 = 5,
	BOOT_SECURITY_LEVEL_6 = 6,
};

int m3ss_otp_get_security_level(enum t_boot_security_level *sl)
{
	int err;

	if (!sl)
		return -EINVAL;
	err = otp_get_boot_info();
	if (err)
		return err;
	*sl = (enum t_boot_security_level)
		p_safemem_boot_cfg->safemem_boot_config.bit.boot_security_level;
	return 0;
}

