/**
 * @file sta_nic_security.c
 * @brief nic and security functions
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include "utils.h"

#include "sta_map.h"
#include "sta_type.h"
#include "sta_common.h"
#include "sta_mem_map.h"
#include "sta_nic_security.h"

#define ESRAM_SECURITY_REGION_SIZE	0x20000		/* 128KB */
#define DRAM_SECURITY_REGION_SIZE	0x4000000	/* 64MB */

/*
 * NIC security configuration applied at early startup.
 * By default all IPs accesses are configured in NOT Secured (NS)
 */
static const struct nic_security_acr nic_security_default[] = {
	/* REMAP Config (No config selected by default) */
	NIC_SECURITY_REGION(acr_remap, SOC_TC3P, REMAP_REGION_MAX,
			    NO_REMAP_CFG),
	/* eSRAM (S0) */
	NIC_SECURITY_REGION(acr_security0, SOC_ALL, SECURITY0_REGION_MAX,
			    REG0(NS) | REG1(NS) | REG2(NS) | REG3(NS)
			    | REG4(NS) | REG5(NS) | REG6(NS) | REG7(NS)),
	/* A7 PLL Config Space (S14) */
	NIC_SECURITY_REGION(acr_security1, SOC_ALL, SECURITY1_REGION_MAX,
			    A7_PLL(NS)),
	/* DDR port 3 (S73) */
	NIC_SECURITY_REGION(acr_security2, SOC_ALL, SECURITY2_REGION_MAX,
			    REG0(NS) | REG1(NS) | REG2(NS) | REG3(NS)
			    | REG4(NS) | REG5(NS) | REG6(NS) | REG7(NS)
			    | REG8(NS) | REG9(NS) | REG10(NS) | REG11(NS)
			    | REG12(NS) | REG13(NS) | REG14(NS) | REG15(NS)),
	/* FSMC (S3) */
	NIC_SECURITY_REGION(acr_security3, SOC_ALL, SECURITY3_REGION_MAX,
			    REG0(NS) | REG1(NS) | REG2(NS) | REG3(NS)
			    | REG4(NS) | REG5(NS) | REG6(NS) | REG7(NS)
			    | REG8(NS) | REG9(NS) | REG10(NS) | REG11(NS)
			    | REG12(NS) | REG13(NS) | REG14(NS) | REG15(NS)),
	/* SQIO AHB 0 (S4) */
	NIC_SECURITY_REGION(acr_security4, SOC_ALL, SECURITY4_REGION_MAX,
			    REG0(NS) | REG1(NS) | REG2(NS) | REG3(NS)
			    | REG4(NS) | REG5(NS) | REG6(NS) | REG7(NS)
			    | REG8(NS) | REG9(NS) | REG10(NS) | REG11(NS)
			    | REG12(NS) | REG13(NS) | REG14(NS) | REG15(NS)),
	/* C3 APB4 (S83) */
	NIC_SECURITY_REGION(acr_security5, SOC_A5, SECURITY5_REGION_MAX,
			    C3_APB(NS)),
	/* APB1 IPs (S5 to S12) */
	NIC_SECURITY_REGION(acr_security6, SOC_ALL, SECURITY6_REGION_MAX,
			    RTC(NS) | EFT3(NS) | EFT4(NS) | CAN1(NS)
			    | GPIO_S(NS) | UART0(NS) | I2C0(NS) | SSP0(NS)),
	/* Safety IPs (S114 to S123) */
	NIC_SECURITY_REGION(acr_security7, SOC_TC3P, SECURITY7_REGION_MAX,
			    CMU0(NS) | CMU1(NS) | CMU2(NS) | CMU3(NS)
			    | CMU5(NS) | CMU6(NS) | FCCU(NS) | CRC(NS)
			    | OTFDEC(NS)),
	/* APB0 IPs (S25 to S32, S78, S79) */
	NIC_SECURITY_REGION(acr_security9, SOC_TC3P, SECURITY9_REGION_MAX,
			    UART1(NS) | UART2(NS) | UART3(NS) | I2C2(NS)
			    | I2C1(NS) | SSP2(NS) | SSP1(NS) | SDMMC1(NS)
			    | ETH0(NS) | ETH1(NS)),
	/* APB3 IPs (S34 to S44) */
	NIC_SECURITY_REGION(acr_security10, SOC_TC3P, SECURITY10_REGION_MAX,
			    A7_SRC(NS) | ADC(NS) | GPIO(NS) | A7_WDG(NS)
			    | MTU1(NS) | MTU0(NS) | EFT2(NS) | EFT1(NS)
			    | EFT0(NS) | MB_A7_M3(NS) | MB_M3_A7(NS)),
	/* MSP-0/1/2 (S51 to S53) */
	NIC_SECURITY_REGION(acr_security11, SOC_TC3P, SECURITY11_REGION_MAX,
			    MSP0(NS) | MSP1(NS) | MSP2(NS)),
	/* HSEM (S45) */
	NIC_SECURITY_REGION(acr_security12, SOC_TC3P, SECURITY12_REGION_MAX,
			    HSEM(NS)),
	/* All IPs CAN SubSystem (S13) */
	NIC_SECURITY_REGION(acr_security13, SOC_ALL, SECURITY13_REGION_MAX,
			    CANSS(NS)),
	/* A7 MISC Regs (S56) */
	NIC_SECURITY_REGION(acr_security14, SOC_ALL, SECURITY14_REGION_MAX,
			    A7_MISC(NS)),
	/* DMA S0 (S16) */
	NIC_SECURITY_REGION(acr_security16, SOC_TC3P, SECURITY16_REGION_MAX,
			    DMA_S0(NS)),
	/* USB0 OTG (S17) */
	NIC_SECURITY_REGION(acr_security17, SOC_TC3P, SECURITY17_REGION_MAX,
			    USB0(NS)),
	/* USB1 OTG (S18) */
	NIC_SECURITY_REGION(acr_security18, SOC_TC3P, SECURITY18_REGION_MAX,
			    USB1(NS)),
	/* STM 500 (S103) */
	NIC_SECURITY_REGION(acr_security19, SOC_TC3P, SECURITY19_REGION_MAX,
			    STM(NS)),
	/* TS OS port 0 & 1 (S105, S106) */
	NIC_SECURITY_REGION(acr_security20, SOC_TC3P, SECURITY20_REGION_MAX,
			    TS_OS0(NS) | TS_OS1(NS)),
	/* TS Debug (S107) */
	NIC_SECURITY_REGION(acr_security21, SOC_TC3P, SECURITY21_REGION_MAX,
			    TS_DBG(NS)),
	/* DMA S1 (S60) */
	NIC_SECURITY_REGION(acr_security23, SOC_TC3P, SECURITY23_REGION_MAX,
			    DMA_S1(NS)),
	/* SDIO 0 (S33) */
	NIC_SECURITY_REGION(acr_security24, SOC_TC3P, SECURITY24_REGION_MAX,
			    SDIO0(NS)),
	/* Flash Cache IP (S109) */
	NIC_SECURITY_REGION(acr_security25, SOC_TC3P, SECURITY25_REGION_MAX,
			    FLASH_CACHE(NS)),
	/* A7 Debug Port (S54) */
	NIC_SECURITY_REGION(acr_security28, SOC_ALL, SECURITY28_REGION_MAX,
			    A7_DP(NS)),
	/* DDR Ctrl (S55) */
	NIC_SECURITY_REGION(acr_security29, SOC_ALL, SECURITY29_REGION_MAX,
			    DDR_CTRL(NS)),
	/* SQIO AHB 1 (S67) */
	NIC_SECURITY_REGION(acr_security30, SOC_A5, SECURITY30_REGION_MAX,
			    REG0(NS) | REG1(NS)),
	/* DDR port 0 (S70) */
	NIC_SECURITY_REGION(acr_security31, SOC_ALL, SECURITY31_REGION_MAX,
			    REG0(NS) | REG1(NS) | REG2(NS) | REG3(NS)
			    | REG4(NS) | REG5(NS) | REG6(NS) | REG7(NS)
			    | REG8(NS) | REG9(NS) | REG10(NS) | REG11(NS)
			    | REG12(NS) | REG13(NS) | REG14(NS) | REG15(NS)),
	/* DDR port 1 (S71) */
	NIC_SECURITY_REGION(acr_security32, SOC_ALL, SECURITY32_REGION_MAX,
			    REG0(NS) | REG1(NS) | REG2(NS) | REG3(NS)
			    | REG4(NS) | REG5(NS) | REG6(NS) | REG7(NS)
			    | REG8(NS) | REG9(NS) | REG10(NS) | REG11(NS)
			    | REG12(NS) | REG13(NS) | REG14(NS) | REG15(NS)),
	/* DDR port 2 (S72) */
	NIC_SECURITY_REGION(acr_security33, SOC_ALL, SECURITY33_REGION_MAX,
			    REG0(NS) | REG1(NS) | REG2(NS) | REG3(NS)
			    | REG4(NS) | REG5(NS) | REG6(NS) | REG7(NS)
			    | REG8(NS) | REG9(NS) | REG10(NS) | REG11(NS)
			    | REG12(NS) | REG13(NS) | REG14(NS) | REG15(NS)),
	/* FDCAN (S74) and DDR PHY (S81) */
	NIC_SECURITY_REGION(acr_security34, SOC_ALL, SECURITY34_REGION_MAX,
			    DDR_PHY(NS) | FDCAN(NS)),
	/* UART_4_5 (S86) */
	NIC_SECURITY_REGION(acr_security35, SOC_ALL, SECURITY35_REGION_MAX,
			    UART_4_5(NS)),
	/* APB3 IPs (S95, S96, S99, S100, S104) */
	NIC_SECURITY_REGION(acr_security36, SOC_TC3P, SECURITY36_REGION_MAX,
			    MB_A7_HSM(NS) | MB_HSM_A7(NS) | MB_M3_HSM(NS)
			    | MB_HSM_M3(NS) | THSENS(NS)),
	/* C3 SAFMEM (S82) */
	NIC_SECURITY_REGION(acr_security38, SOC_A5, SECURITY38_REGION_MAX,
			    SAFMEM(NS)),
	/* C3 AHB (S79) */
	NIC_SECURITY_REGION(acr_security39, SOC_A5, SECURITY39_REGION_MAX,
			    C3_AHB(NS)),
	/* FLEXRAY (S89) */
	NIC_SECURITY_REGION(acr_security40, SOC_TC3P, SECURITY40_REGION_MAX,
			    FLEXRAY(NS)),
};

/*
 * Set platform NIC security to Secure (S) configuration for the expected IPs.
 *
 * WARNING: For each ACR security register updated, all associated regions
 * must be defined by mixing Secure (S) and Not Secure (NS) setting.
 * Refer to nic_security_default[] declaration above to retrieve the right
 * ACR register definition and replace (NS) by (S) to configure the expected
 * region Secure.
 */
/* Set expected NIC security config for SoC TC3P ( */
static const struct nic_security_acr nic_security_tc3p_cfg[] = {
	/* A7 PLL Config Space (S14) */
	NIC_SECURITY_REGION(acr_security1, SOC_ALL, SECURITY1_REGION_MAX,
			    A7_PLL(S)),
	/* APB1 IPs (S5 to S12) */
	NIC_SECURITY_REGION(acr_security6, SOC_ALL, SECURITY6_REGION_MAX,
			    RTC(NS) | EFT3(S) | EFT4(S) | CAN1(NS)
			    | GPIO_S(NS) | UART0(S) | I2C0(S) | SSP0(S)),
	/* Safety IPs (S114 to S123) */
	NIC_SECURITY_REGION(acr_security7, SOC_TC3P, SECURITY7_REGION_MAX,
			    CMU0(S) | CMU1(S) | CMU2(S) | CMU3(S)
			    | CMU5(S) | CMU6(S) | FCCU(S) | CRC(S)
			    | OTFDEC(S)),
	/* APB0 IPs (S25 to S32, S78, S79) */
	NIC_SECURITY_REGION(acr_security9, SOC_TC3P, SECURITY9_REGION_MAX,
			    UART1(NS) | UART2(NS) | UART3(NS) | I2C2(S)
			    | I2C1(S) | SSP2(NS) | SSP1(NS) | SDMMC1(NS)
			    | ETH0(NS) | ETH1(NS)),
	/* APB3 IPs (S34 to S44) */
	NIC_SECURITY_REGION(acr_security10, SOC_TC3P, SECURITY10_REGION_MAX,
			    A7_SRC(S) | ADC(NS) | GPIO(NS) | A7_WDG(NS)
			    | MTU1(NS) | MTU0(S) | EFT2(S) | EFT1(S)
			    | EFT0(S) | MB_A7_M3(S) | MB_M3_A7(S)),
	/* All IPs CAN SubSystem (S13) */
	NIC_SECURITY_REGION(acr_security13, SOC_ALL, SECURITY13_REGION_MAX,
			    CANSS(S)),
	/* A7 MISC Regs (S56) */
	NIC_SECURITY_REGION(acr_security14, SOC_ALL, SECURITY14_REGION_MAX,
			    A7_MISC(S)),
	/* TS OS port 0 & 1 (S105, S106) */
	NIC_SECURITY_REGION(acr_security20, SOC_TC3P, SECURITY20_REGION_MAX,
			    TS_OS0(S) | TS_OS1(S)),
	/* TS Debug (S107) */
	NIC_SECURITY_REGION(acr_security21, SOC_TC3P, SECURITY21_REGION_MAX,
			    TS_DBG(S)),
	/* Flash Cache IP (S109) */
	NIC_SECURITY_REGION(acr_security25, SOC_TC3P, SECURITY25_REGION_MAX,
			    FLASH_CACHE(S)),
	/* A7 Debug Port (S54) */
	NIC_SECURITY_REGION(acr_security28, SOC_ALL, SECURITY28_REGION_MAX,
			    A7_DP(S)),
	/* DDR Ctrl (S55) */
	NIC_SECURITY_REGION(acr_security29, SOC_ALL, SECURITY29_REGION_MAX,
			    DDR_CTRL(S)),
	/* FDCAN (S74) and DDR PHY (S81) */
	NIC_SECURITY_REGION(acr_security34, SOC_ALL, SECURITY34_REGION_MAX,
			    DDR_PHY(S) | FDCAN(NS)),
	/* APB3 IPs (S95, S96, S99, S100, S104) */
	NIC_SECURITY_REGION(acr_security36, SOC_TC3P, SECURITY36_REGION_MAX,
			    MB_A7_HSM(S) | MB_HSM_A7(S) | MB_M3_HSM(S)
			    | MB_HSM_M3(S) | THSENS(S)),
};

/* Set expected NIC security config for SoC A5 */
static const struct nic_security_acr nic_security_a5_cfg[] = {
	/* TODO */
};

/**
 * @brief  Initialize a set of bits as secured or not.
 * @param  reg: pointer to the register to set
 * @param  len: number of regions to be set as secured or not
 * @param  sec_cfg: Bitfields with security config: 0 Secure / 1 Non-secure
 * @retval None
 *
 * @warning: reg is a Write Only register so all its associated regions must
 *           be assigned in the sec_cfg bitfield
 */
static void nic_set_all_security_regions(volatile uint32_t *reg,
					 uint8_t len, uint32_t sec_cfg)
{
	uint32_t msk, i;
	uint32_t reg_temp = 0;

	for (i = 0; i < len; i++) {
		msk = BIT(i);
		if (sec_cfg & msk)
			reg_temp |= msk;	/* 1: Non-secure */
		else
			reg_temp &= ~msk;	/* 0: Secure */
	}
	*reg = reg_temp;
}

/**
 * @brief  Initialize NIC security with default not secure accesses setting
 *	   for all peripherals and memories
 * @param  None
 * @retval None
 */
void nic_security_init(void)
{
	const struct nic_security_acr *acr;
	unsigned int i;
	uint8_t soc = SOC_A5;

	if (get_soc_id() == SOCID_STA1385 && get_cut_rev() >= CUT_20)
		soc = SOC_TC3P;

	for (i = 0; i < NELEMS(nic_security_default); i++) {
		acr = &nic_security_default[i];

		if (acr->soc & soc) {
			/* Apply default settings */
			nic_set_all_security_regions(acr->reg, acr->region_max,
						     acr->sec_config);
		}
	}

	/* Enable A7 Secure transaction to all legitimate slaves */
	src_m3_regs->a7axprot.bit.a7_axprot = 0;
}

/**
 * @brief  Set platform security peripheral access configuration
 *         Rely on static table config defined according to the SoC
 *         - For TC3P (>=cut2.0)   : nic_security_tc3p_cfg[]
 *         - For A5 (and all other): nic_security_a5_cfg[]
 * @param  None
 * @retval None
 */
void nic_set_security_periph_cfg(void)
{
	unsigned int i;
	const struct nic_security_acr *acr = &nic_security_a5_cfg[0];
	uint8_t soc = SOC_A5;
	unsigned int nbelem = NELEMS(nic_security_a5_cfg);

	if (get_soc_id() == SOCID_STA1385 && get_cut_rev() >= CUT_20) {
		soc = SOC_TC3P;
		acr = &nic_security_tc3p_cfg[0];
		nbelem = NELEMS(nic_security_tc3p_cfg);
	}

	/* Set peripheral secure access */
	for (i = 0; i < nbelem; i++) {
		if (acr->soc & soc) {
			/* Apply nic settings */
			nic_set_all_security_regions(acr->reg, acr->region_max,
						     acr->sec_config);
			TRACE_INFO("NIC security setting for %s:\t0x%x\n",
				   acr->name, acr->sec_config);
		}
		acr++;
	}
}

/**
 * @brief  Set platform security memory access configuration
 *         Rely on the platform memory mapping definition generated from xml
 *         memory mapping file
 * @param  None
 * @retval None
 */
void nic_set_security_mem_cfg(void)
{
#if defined(ESRAM_A7_ATF_TRUSTED_ZONE_BASE) || \
	defined(DDRAM_ATF_TRUSTED_ZONE_BASE)
	unsigned int i;
	uint32_t addr;
	uint32_t sec_cfg;
#endif

#if defined(ESRAM_A7_ATF_TRUSTED_ZONE_BASE)
	/* Set A7 eSRAM memory secure access */
	addr = ESRAM_A7_BASE;
	sec_cfg = 0;	/* All regions set to secure by default */

	for (i = 0; i < (ESRAM_A7_SIZE / ESRAM_SECURITY_REGION_SIZE); i++) {
		if (addr < ESRAM_A7_ATF_TRUSTED_ZONE_BASE ||
		    addr > (ESRAM_A7_ATF_TRUSTED_ZONE_BASE +
			    ESRAM_A7_ATF_TRUSTED_ZONE_SIZE - 1)) {
			/* Set to not secure */
			sec_cfg	|= SET_REGION(i, NS);
			sec_cfg	|= SET_REGION((i + (SECURITY0_REGION_MAX / 2)),
					      NS);
		}
		addr += ESRAM_SECURITY_REGION_SIZE;
	}
	/* Apply nic settings for eSRAM */
	nic_set_all_security_regions(&nic_regs->acr_security0.reg,
				     SECURITY0_REGION_MAX, sec_cfg);
	TRACE_INFO("NIC security setting for A7 eSRAM:\t\t0x%x\n", sec_cfg);
#endif

#if defined(DDRAM_ATF_TRUSTED_ZONE_BASE)
	/* Set DDRAM memory secure access */
	addr = DDRAM_BASE;
	sec_cfg = 0;	/* All regions set to secure by default */

	for (i = 0; i < (DDRAM_SIZE / DRAM_SECURITY_REGION_SIZE); i++) {
		if (addr < DDRAM_ATF_TRUSTED_ZONE_BASE ||
		    addr > (DDRAM_ATF_TRUSTED_ZONE_BASE +
			    DDRAM_ATF_TRUSTED_ZONE_SIZE - 1))
			/* Set to not secure */
			sec_cfg	|= SET_REGION(i, NS);

		addr += DRAM_SECURITY_REGION_SIZE;
	}
	/* Apply nic settings */
	/* DDR port 0 */
	nic_set_all_security_regions(&nic_regs->acr_security31.reg,
				     SECURITY31_REGION_MAX, sec_cfg);
	/* DDR port 1 */
	nic_set_all_security_regions(&nic_regs->acr_security32.reg,
				     SECURITY32_REGION_MAX, sec_cfg);
	/* DDR port 2 */
	nic_set_all_security_regions(&nic_regs->acr_security33.reg,
				     SECURITY33_REGION_MAX, sec_cfg);
	/* DDR port 3 */
	nic_set_all_security_regions(&nic_regs->acr_security2.reg,
				     SECURITY2_REGION_MAX, sec_cfg);

	TRACE_INFO("NIC security setting for DDR port 0/1/2/3:\t0x%x\n",
		   sec_cfg);
#endif
}

/**
 * @brief  Reset platform security memory access configuration
 *         All memory accesses are configured in NOT Secured (NS)
 *         memory mapping file
 * @param  None
 * @retval None
 */
void nic_reset_security_mem_cfg(void)
{
	uint32_t sec_cfg;

	/* All regions set to NOT secure */
	sec_cfg = 0xFFFF;

	/* Apply nic settings for eSRAM */
	nic_set_all_security_regions(&nic_regs->acr_security0.reg,
				     SECURITY0_REGION_MAX, sec_cfg);
	TRACE_INFO("NIC security setting for A7 eSRAM:\t\t0x%x\n", sec_cfg);

	/* Apply nic settings */
	/* DDR port 0 */
	nic_set_all_security_regions(&nic_regs->acr_security31.reg,
				     SECURITY31_REGION_MAX, sec_cfg);
	/* DDR port 1 */
	nic_set_all_security_regions(&nic_regs->acr_security32.reg,
				     SECURITY32_REGION_MAX, sec_cfg);
	/* DDR port 2 */
	nic_set_all_security_regions(&nic_regs->acr_security33.reg,
				     SECURITY33_REGION_MAX, sec_cfg);
	/* DDR port 3 */
	nic_set_all_security_regions(&nic_regs->acr_security2.reg,
				     SECURITY2_REGION_MAX, sec_cfg);

	TRACE_INFO("NIC security setting for DDR port 0/1/2/3:\t0x%x\n",
		   sec_cfg);
}

/**
 * @brief  Set platform security remap configuration
 *	   Isolate M3 sub-system and some others peripherals (according to the
 *	   selected config) from external secure and not secure accesses.
 *         REMAP_CFG constant must be set to one of the following value:
 *         - NO_REMAP_CFG
 *         - REMAP_CFG0, REMAP_CFG1, REMAP_CFG2 or REMAP_CFG3
 * @param  None
 * @retval None
 */
#define REMAP_CFG	REMAP_CFG3
void nic_set_security_remap_cfg(void)
{
	/* Set REMAP Config. Only supported for TC3P */
	if (get_soc_id() == SOCID_STA1385 && get_cut_rev() >= CUT_20) {
		nic_set_all_security_regions(&nic_regs->acr_remap.reg,
					     REMAP_REGION_MAX, REMAP_CFG);
	}
}

