/**
 * @file sta_regmap_firewall.c
 * @brief functions to filter access to critical peripherals.
 *
 * Copyright (C) ST-Microelectronics SA 2018
 */
#include "sta_map.h"
#include "sta_regmap.h"
#include "sta_src.h"
#include "sta_src_a7.h"
#include "utils.h"

/**
 * struct regmap - register map descriptor
 * @start: physical base address
 * @size: size of map in bytes
 * @access_rights: pointer to array of registers access rights
 *		   For each registers, a mask has to be provided to say
 *		   whether bits in a register can be written or not.
 */
struct regmap {
	const uint32_t start;
	const uint32_t size;
	const uint32_t *access_rights;
};

#define SRCM3_PCKEN1_AR \
	(BIT(SSPCLK) | BIT(UART0CLK) | BIT(SDICLK) | BIT(I2C0CLK) | \
	 BIT(I2C1CLK) | BIT(UART1CLK) | BIT(SQI_CLK) | BIT(AUDIO_SS_512) | \
	 BIT(AUDIO_SS_MCLK) | BIT(SSP2CLK) | BIT(UART2CLK) | BIT(SDICLK1) | \
	 BIT(SSP1CLK) | BIT(I2C2CLK) | BIT(UART3CLK) | BIT(MSP0CLK) | \
	 BIT(MSP1CLK) | BIT(MSP2CLK) | BIT(SDICLK2) | BIT(CANFDCLK))

#define SRCM3_PCKEN2_AR \
	(BIT(ETHPTPCLK - MAX_SRCM3_1_CLK) | BIT(FDCANPCLK - MAX_SRCM3_1_CLK) | \
	 BIT(WLANCLK - MAX_SRCM3_1_CLK))

static t_src_m3 srcm3_access_rights = {
	.ctrl.reg = 0x0,
	.imctrl.reg = 0x0,
	.imstat.reg = 0x0,
	.xtalcr.reg = 0x0,
	.pllctrl.reg = 0x400,
	.a7axprot.reg = 0x0,
	.pll2fctrl.reg = 0x0,
	.pll3fctrl.reg = 0x1FFF00FF,
	.pll3fctrl2.reg = 0xFFFF,
	.resstat.reg = 0x0,
	.unused_1 = 0x0,
	.unused_2 = 0x0,
	.pcken1 = SRCM3_PCKEN1_AR,
	.pckdis1 = SRCM3_PCKEN1_AR,
	.pckensr1 = 0x0,
	.pcksr1 =  0x0,
	.clkocr.reg = 0x0,
	.clkdivcr.reg = 0x3FFF7F,
	.debug.reg = 0x0,
	.plldfctrl.reg = 0x0,
	.pll4fctrl.reg = 0x1FFF00FF,
	.scpll4sscgctrl.reg = 0xFFFFFFFF,
	.eccfailadd.reg = 0x0,
	.eccforceadd.reg = 0x0,
	.unused1 = 0x0,
	.pcken2 = SRCM3_PCKEN2_AR,
	.pckdis2 = SRCM3_PCKEN2_AR,
	.pckensr2 = 0x0,
	.pcksr2 = 0x0,
	.clkocr1.reg = 0x0,
};

static struct regmap regmap_srcm3 = {
	.start = SRCM3_BASE,
	.size = 0x80,
	.access_rights = (const uint32_t *)&srcm3_access_rights,
};

#define SRCA7_PCKEN0_AR \
	(BIT(HCLKFSMC) | BIT(HCLKSSP0) | BIT(HCLKSSP0) | BIT(PCLKSSP1) | \
	 BIT(PCLKSSP2) | BIT(PCLKSDI0) | BIT(PCLKSDMMC1) | BIT(PCLKI2C1) | \
	 BIT(PCLKI2C2) | BIT(PCLKUART1) | BIT(PCLKUART2) | BIT(PCLKUART3) | \
	 BIT(PCLKHSEM) | BIT(HCLKUSB) | BIT(PCLKSARADC) | BIT(PCLKSMSP0) | \
	 BIT(PCLKSMSP1) | BIT(PCLKSMSP2) | BIT(PCLKETH) | BIT(PCLKI2C0) | \
	 BIT(PCLKUART0))

#define SRCA7_PCKEN1_AR (BIT(PCLKSDMMC2 - MAX_SRCA7_0_CLK))

static t_src_a7 srca7_access_rights = {
	.scimctrl_lo.reg = 0x0,
	.scimstat_lo.reg = 0x0,
	.unused_1 = 0x0,
	.unused_2 = 0x0,
	.unused_3 = 0x0,
	.unused_4 = 0x0,
	.scpll3fctrl.reg = 0x3FF,
	.resstat.reg = 0xE0,
	.scclkdivcr.reg = 0x180,
	.unused_5 = 0x0,
	.pcken0 = SRCA7_PCKEN0_AR,
	.pckdis0 = SRCA7_PCKEN0_AR,
	.pckensr0 = 0x0,
	.pcksr0 = 0x0,
	.scdebug = 0x0,
	.scplldfctrl.reg = 0x0,
	.scresctrl1.reg = 0x4000,
	.pcken1 = SRCA7_PCKEN1_AR,
	.pckdis1 = SRCA7_PCKEN1_AR,
	.pckensr1 = 0x0,
	.pcksr1 = 0x0,
};

static struct regmap regmap_srca7 = {
	.start = SRCAP_BASE,
	.size = 0x54,
	.access_rights = (const uint32_t *)&srca7_access_rights,
};

static t_a7_ssc a7ssc_access_rights = {
	.chclkreq.reg = 0x0,
	.dummy1[0] = 0x0,
	.dummy1[1] = 0x0,
	.dummy1[2] = 0x0,
	.brm[0] = 0x0,
	.brm[1] = 0x0,
	.brm[2] = 0x0,
	.brm[3] = 0x0,
	.rmwm[0] = 0x0,
	.rmwm[1] = 0x0,
	.rmwm[2] = 0x0,
	.rmwm[3] = 0x0,
	.unused_1[0] = 0x0,
	.unused_1[1] = 0x0,
	.unused_1[2] = 0x0,
	.unused_1[3] = 0x0,
	.unused_1[4] = 0x0,
	.unused_1[5] = 0x0,
	.unused_1[6] = 0x0,
	.unused_1[7] = 0x0,
	.unused_1[8] = 0x0,
	.unused_1[9] = 0x0,
	.unused_1[10] = 0x0,
	.unused_1[11] = 0x0,
	.unused_1[12] = 0x0,
	.unused_1[13] = 0x0,
	.unused_1[14] = 0x0,
	.unused_1[15] = 0x0,
	.unused_1[16] = 0x0,
	.unused_1[17] = 0x0,
	.unused_1[18] = 0x0,
	.unused_1[19] = 0x0,
	.pllarm_freq.reg = 0x100FC00,
	.dummy2[0] = 0x0,
	.dummy2[1] = 0x0,
	.pllarm_ctrl.reg = 0x0,
	.dummy3[0] = 0x0,
	.dummy3[1] = 0x0,
	.dummy3[2] = 0x0,
	.pllarm_lockp.reg = 0x0,
	.dummy4[0] = 0x0,
	.dummy4[1] = 0x0,
	.dummy4[2] = 0x0,
	.pllarm_other_modes[0] = 0x0,
	.pllarm_other_modes[1] = 0x0,
	.pllarm_other_modes[2] = 0x0,
	.pllarm_other_modes[3] = 0x0,
	.unused_2[0] = 0x0,
	.unused_2[1] = 0x0,
	.dbgpwr[0] = 0x0,
	.dbgpwr[1] = 0x0,
	.dbgpwr[2] = 0x0,
	.dbgpwr[3] = 0x0,
	.dbgctrl[0] = 0x0,
	.dbgctrl[1] = 0x0,
	.dbgctrl[2] = 0x0,
	.dbgctrl[3] = 0x0,
	.unused_3[0] = 0x0,
	.unused_3[1] = 0x0,
	.unused_3[2] = 0x0,
	.unused_3[3] = 0x0,
	.cpu0_mgt.reg = 0x0,
	.cpu1_mgt.reg = 0x0,
	.cpu2_mgt[0] = 0x0,
	.cpu2_mgt[1] = 0x0,
	.cpu2_mgt[2] = 0x0,
	.cpu2_mgt[3] = 0x0,
	.cpu3_mgt[0] = 0x0,
	.cpu3_mgt[1] = 0x0,
	.cpu3_mgt[2] = 0x0,
	.cpu3_mgt[3] = 0x0,
	.top_l2_mgt.reg = 0x0,
	.rstack[0] = 0x0,
	.rstack[1] = 0x0,
	.rstack[2] = 0x0,
	.rstack[3] = 0x0,
	.mask_req.reg = 0x0,
	.dummy5[0] = 0x0,
	.dummy5[1] = 0x0,
	.dummy5[2] = 0x0,
};

static struct regmap regmap_a7ssc = {
	.start = A7_SSC_BASE,
	.size = 0xB4,
	.access_rights = (const uint32_t *)&a7ssc_access_rights,
};

t_misc a7mscr_access_rights = {
	.io_driver_reg.reg = 0x0,
	.dma_sel.reg = 0xFFFFFF,
	.misc_reg1.reg = 0x0,
	.misc_reg2.reg = 0x0,
	.misc_reg3.reg = 0xE0000000,
	.misc_reg4.reg = 0x0,
	.misc_reg5.reg = 0x0,
	.misc_reg6.reg = 0x0,
	.misc_reg7.reg = 0x0,
	.misc_reg8.reg = 0x80000800,
	.misc_reg9.reg = 0x0,
	.misc_reg10.reg = 0xFF,
	.ddr_int_en.reg = 0x0,
	.eth_int_en.reg = 0xFFFFFFFE,
	.clcd_int_en.reg = 0x0,
	.cpu_int_en.reg = 0x0,
	.misc_reg11.reg = 0x3,
	.misc_reg12.reg = 0xFF,
	.misc_reg13.reg = 0x0,
	.misc_reg14.reg = 0xFFEFF00,
	.misc_reg15.reg = 0x0,
	.misc_reg16.reg = 0x100,
	.misc_reg17.reg = 0x0,
	.misc_reg18.reg = 0x0,
	.unused1[0 ... 41] = 0x0,
	.misc_reg66.reg = 0x0,
	.misc_reg67.reg = 0x300000F0,
	.unused2[0 ... 6] = 0x0,
	.misc_reg75.reg = 0x0,
	.misc_reg76.reg = 0x0,
	.misc_reg77.reg = 0x0,
	.misc_reg78.reg = 0x0,
	.misc_reg79.reg = 0x0,
};

static struct regmap regmap_a7mscr = {
	.start = MISC_A7_BASE,
	.size = 0x140,
	.access_rights = (const uint32_t *)&a7mscr_access_rights,
};

static struct regmap *regmaps[] = {
	&regmap_srcm3,
	&regmap_srca7,
	&regmap_a7ssc,
	&regmap_a7mscr,
	NULL,
};

int regmap_find_register_access_rights(uint32_t reg, uint32_t *access_rights)
{
	struct regmap **rm_p = regmaps;
	struct regmap *rm;
	int index;

	/* register address must be aligned on 32 bits */
	if (reg % sizeof(uint32_t) != 0)
		return STA_REGMAP_INVALID_PARAMS;

	while (rm_p) {
		rm = *rm_p;
		if ((reg >= rm->start) && (reg < (rm->start + rm->size)))
			break;
		rm_p++;
	}
	if (!rm_p)
		return STA_REGMAP_INVALID_PARAMS;

	index = (reg - rm->start) / sizeof(uint32_t);
	*access_rights = rm->access_rights[index];
	return 0;
}
