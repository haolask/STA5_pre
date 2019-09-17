/**
 * @file sta_nic_security.h
 * @brief NIC and security header file
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#ifndef _STA_NIC_SECURITY_H_
#define _STA_NIC_SECURITY_H_

#include "sta_type.h"


/* Max region per NIC security Access Control Register (ACR) */
#define REMAP_REGION_MAX	8
#define SECURITY0_REGION_MAX	8
#define SECURITY1_REGION_MAX	1
#define SECURITY2_REGION_MAX	16
#define SECURITY3_REGION_MAX	16
#define SECURITY4_REGION_MAX	16
#define SECURITY5_REGION_MAX	1
#define SECURITY6_REGION_MAX	8
#define SECURITY7_REGION_MAX	12
#define SECURITY8_REGION_MAX	0
#define SECURITY9_REGION_MAX	10
#define SECURITY10_REGION_MAX	11
#define SECURITY11_REGION_MAX	3
#define SECURITY12_REGION_MAX	1
#define SECURITY13_REGION_MAX	1
#define SECURITY14_REGION_MAX	1
#define SECURITY15_REGION_MAX	0
#define SECURITY16_REGION_MAX	1
#define SECURITY17_REGION_MAX	1
#define SECURITY18_REGION_MAX	1
#define SECURITY19_REGION_MAX	1
#define SECURITY20_REGION_MAX	2
#define SECURITY21_REGION_MAX	1
#define SECURITY22_REGION_MAX	0
#define SECURITY23_REGION_MAX	1
#define SECURITY24_REGION_MAX	1
#define SECURITY25_REGION_MAX	1
#define SECURITY26_REGION_MAX	0
#define SECURITY27_REGION_MAX	0
#define SECURITY28_REGION_MAX	1
#define SECURITY29_REGION_MAX	1
#define SECURITY30_REGION_MAX	2
#define SECURITY31_REGION_MAX	16
#define SECURITY32_REGION_MAX	16
#define SECURITY33_REGION_MAX	16
#define SECURITY34_REGION_MAX	2
#define SECURITY35_REGION_MAX	1
#define SECURITY36_REGION_MAX	9
#define SECURITY37_REGION_MAX	0
#define SECURITY38_REGION_MAX	1
#define SECURITY39_REGION_MAX	1
#define SECURITY40_REGION_MAX	3

/* SoC applicability of the NIC security ACR */
#define SOC_A5			BIT(0)		/* All A5 & TC3P Cut1 */
#define SOC_TC3P		BIT(1)		/* TC3P Cut2 and higher */
#define SOC_ALL			(SOC_A5 | SOC_TC3P)

/* NIC security region setting */
#define S			0
#define NS			1
#define SET_REGION(reg, sec)	((sec) << (reg))
#define REG0(sec)		SET_REGION(0, sec)
#define REG1(sec)		SET_REGION(1, sec)
#define REG2(sec)		SET_REGION(2, sec)
#define REG3(sec)		SET_REGION(3, sec)
#define REG4(sec)		SET_REGION(4, sec)
#define REG5(sec)		SET_REGION(5, sec)
#define REG6(sec)		SET_REGION(6, sec)
#define REG7(sec)		SET_REGION(7, sec)
#define REG8(sec)		SET_REGION(8, sec)
#define REG9(sec)		SET_REGION(9, sec)
#define REG10(sec)		SET_REGION(10, sec)
#define REG11(sec)		SET_REGION(11, sec)
#define REG12(sec)		SET_REGION(12, sec)
#define REG13(sec)		SET_REGION(13, sec)
#define REG14(sec)		SET_REGION(14, sec)
#define REG15(sec)		SET_REGION(15, sec)

#define NO_REMAP_CFG		0
#define REMAP_CFG0		BIT(0)
#define REMAP_CFG1		BIT(1)
#define REMAP_CFG2		BIT(2)
#define REMAP_CFG3		BIT(3)

#define A7_PLL(sec)		REG0(sec)

#define C3_APB(sec)		REG0(sec)

#define RTC(sec)		REG0(sec)
#define EFT3(sec)		REG1(sec)
#define EFT4(sec)		REG2(sec)
#define CAN1(sec)		REG3(sec)
#define GPIO_S(sec)		REG4(sec)
#define UART0(sec)		REG5(sec)
#define I2C0(sec)		REG6(sec)
#define SSP0(sec)		REG7(sec)

#define CMU0(sec)		REG0(sec)
#define CMU1(sec)		REG1(sec)
#define CMU2(sec)		REG2(sec)
#define CMU3(sec)		REG3(sec)
#define CMU5(sec)		REG5(sec)
#define CMU6(sec)		REG6(sec)
#define FCCU(sec)		REG8(sec)
#define CRC(sec)		REG9(sec)
#define OTFDEC(sec)		REG11(sec)

#define UART1(sec)		REG0(sec)
#define UART2(sec)		REG1(sec)
#define UART3(sec)		REG2(sec)
#define I2C2(sec)		REG3(sec)
#define I2C1(sec)		REG4(sec)
#define SSP2(sec)		REG5(sec)
#define SSP1(sec)		REG6(sec)
#define SDMMC1(sec)		REG7(sec)
#define ETH0(sec)		REG8(sec)
#define ETH1(sec)		REG9(sec)

#define A7_SRC(sec)		REG0(sec)
#define ADC(sec)		REG1(sec)
#define GPIO(sec)		REG2(sec)
#define A7_WDG(sec)		REG3(sec)
#define MTU1(sec)		REG4(sec)
#define MTU0(sec)		REG5(sec)
#define EFT2(sec)		REG6(sec)
#define EFT1(sec)		REG7(sec)
#define EFT0(sec)		REG8(sec)
#define MB_A7_M3(sec)		REG9(sec)
#define MB_M3_A7(sec)		REG10(sec)

#define MSP0(sec)		REG0(sec)
#define MSP1(sec)		REG1(sec)
#define MSP2(sec)		REG2(sec)

#define HSEM(sec)		REG0(sec)

#define CANSS(sec)		REG0(sec)

#define A7_MISC(sec)		REG0(sec)

#define DMA_S0(sec)		REG0(sec)

#define USB0(sec)		REG0(sec)

#define USB1(sec)		REG0(sec)

#define STM(sec)		REG0(sec)

#define TS_OS0(sec)		REG0(sec)
#define TS_OS1(sec)		REG1(sec)

#define TS_DBG(sec)		REG0(sec)

#define DMA_S1(sec)		REG0(sec)

#define SDIO0(sec)		REG0(sec)

#define FLASH_CACHE(sec)	REG0(sec)

#define A7_DP(sec)		REG0(sec)

#define DDR_CTRL(sec)		REG0(sec)

#define DDR_PHY(sec)		REG0(sec)
#define FDCAN(sec)		REG1(sec)

#define UART_4_5(sec)		REG0(sec)

#define MB_A7_HSM(sec)		REG2(sec)
#define MB_HSM_A7(sec)		REG3(sec)
#define MB_M3_HSM(sec)		REG6(sec)
#define MB_HSM_M3(sec)		REG7(sec)
#define THSENS(sec)		REG8(sec)

#define SAFMEM(sec)		REG0(sec)

#define C3_AHB(sec)		REG0(sec)

#define FLEXRAY(sec)		REG1(sec)

#define NIC_SECURITY_REGION(acr, soc, region, sec_cfg) \
	{ \
		#acr, \
		&nic_regs->acr.reg, \
		sec_cfg, \
		region, \
		soc \
	}

struct nic_security_acr {
	char *name;
	volatile uint32_t *reg;
	uint32_t sec_config;
	const uint8_t region_max;
	const uint8_t soc;
};


void nic_security_init(void);
void nic_set_security_periph_cfg(void);
void nic_set_security_mem_cfg(void);
void nic_reset_security_mem_cfg(void);
void nic_set_security_remap_cfg(void);

#endif /* _STA_NIC_SECURITY_H_ */
