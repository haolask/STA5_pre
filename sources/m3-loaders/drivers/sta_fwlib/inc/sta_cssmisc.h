/**
 * @file sta_cssmisc.h
 * @brief This file provides all the µCSS definitions.
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: ADG-MID team
 */

#ifndef _STA_CSSMISC_H_
#define _STA_CSSMISC_H_

#define OTP_SECR_VR(x)	(0x00 + ((x & 7) * 0x10))
#define OTP_SECR_VR_MAX 8

#define CSS_MISC		0xC0
#define SQI_SEL			0
#define FSMC_SEL		1

#define CSS_SPARE0		0xD0
#define CSS_SPARE1		0xE0
#define CSS_SPARE2		0xF0
#define CSS_SPARE3		0x100

struct otp_vr4_reg {
	uint32_t product_id       : 8;
	uint32_t erom_version     : 8;
	uint32_t trimming         : 8;
	uint32_t ehsm_rom_version : 8;
};

struct otp_vr5_reg {
	uint32_t cut_rev : 4; /* 0 => cut1.0 */
	uint32_t year    : 4;
	uint32_t week    : 6;
	uint32_t lot     : 18;
};

#endif /*_STA_CSSMISC_H_ */

