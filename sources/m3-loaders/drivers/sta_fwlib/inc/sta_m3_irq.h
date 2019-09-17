/**
 * @file sta_m3_irq.h
 * @brief M3 irq firmware header file
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#ifndef __STA_M3_IRQ_H__
#define __STA_M3_IRQ_H__

#include "sta_map.h"

typedef enum {
	EXT_IRQ0,
	EXT_IRQ1,
	EXT_IRQ2,
	EXT_IRQ3,
	EXT_IRQ4,
	EXT_IRQ5,
	EXT_IRQ6,
	EXT_IRQ7,
	EXT_IRQ8,
	EXT_IRQ9,
	EXT_IRQ10,
	EXT_IRQ11,
	EXT_IRQ12,
	EXT_IRQ13,
	EXT_IRQ14,
	EXT_IRQ15,
	EXT_EMPTY,
} m3irq_ext_int_channel;

typedef enum {
	_0_DMA0,
	_0_DMA1,
	_0_USBHOST,
	_0_USBOTG,
	_0_SRC_R4,
	_0_MTU0,
	_0_MTU1,
	_0_SI,
	_0_Res0,
	_0_PMU_R4,
	_0_DSP0,
	_0_ADC_TSC,
	_0_MSP0,
	_0_DSP2,
	_0_CLCD,
	_0_SGA,
	_0_GPIO0,
	_0_GPIO1,
	_0_I2C1,
	_0_I2C2,
	_0_SSP1,
	_0_SSP2,
	_0_UART1,
	_0_UART2,
	_0_UART3,
	_0_SD_MMC_0,
	_0_SD_MMC_1,
	_0_FSMC_CR,
	_0_FSMC_FF,
	_0_Res1,
	_0_Res2,
	_0_HSEM_IRQ_B,
	_1_DMA0,
	_1_DMA1,
	_1_USBHOST,
	_1_USBOTG,
	_1_SRC_R4,
	_1_MTU0,
	_1_MTU1,
	_1_SI,
	_1_Res0,
	_1_SDRAM_CTRL,
	_1_DSP1,
	_1_MSP1,
	_1_MSP2,
	_1_CD,
	_1_CLCD,
	_1_SGA,
	_1_GPIO0,
	_1_GPIO1,
	_1_I2C1,
	_1_I2C2,
	_1_SSP1,
	_1_SSP2,
	_1_UART1,
	_1_UART2,
	_1_UART3,
	_1_SD_MMC0,
	_1_SD_MMC1,
	_1_FSMC_CR,
	_1_FSMC_FF,
	_1_Res1,
	_1_Res2,
	_1_HSEM_IRQ_B,
	_2_DMA0,
	_2_DMA1,
	_2_USBHOST,
	_2_USBOTG,
	_2_SRC_R4,
	_2_MTU0,
	_2_MTU1,
	_2_SI,
	_2_SQI,
	_2_PMU_R4,
	_2_EFT0,
	_2_EFT1,
	_2_IPBUS_SYNCH0,
	_2_VIP_LINE,
	_2_VIP_FRM,
	_2_GPIO2,
	_2_GPIO3,
	_2_I2C1,
	_2_I2C2,
	_2_GPIO4,
	_2_SSP1,
	_2_SSP2,
	_2_UART1,
	_2_UART2,
	_2_UART3,
	_2_SD_MMC0,
	_2_SD_MMC1,
	_2_Res0,
	_2_Res1,
	_2_Res2,
	_2_HSEM_IRQ_B,
	_2_Res3,
	_3_DMA0,
	_3_DMA1,
	_3_USBHOST,
	_3_USBOTG,
	_3_SRC_R4,
	_3_MTU0,
	_3_MTU1,
	_3_SI,
	_3_SQI,
	_3_PMU_R4,
	_3_EFT2,
	_3_IPBUS_SYNCH0,
	_3_IPBUS_SYNCH1,
	_3_VIP_LINE,
	_3_VIP_FRM,
	_3_GPIO2,
	_3_GPIO3,
	_3_I2C1,
	_3_I2C2,
	_3_GPIO4,
	_3_SSP1,
	_3_SSP2,
	_3_UART1,
	_3_UART2,
	_3_UART3,
	_3_SD_MMC0,
	_3_SD_MMC1,
	_3_FSMC_FF,
	_3_Res0,
	_3_Res1,
	_3_HSEM_IRQ_B,
	_3_Res3,
	EMPTY
} apirq_int_channel;

/**
 * struct m3irq - M3 IRQ controller device definition
 * @id: index of the controller
 * @base: the base address of the controller
 */
struct m3irq {
	int id;
	uint32_t base;
};

/**
 * struct m3irq_line - M3 IRQ line definition
 * @id: the current line index
 * @irq_handler: the handler callback
 */
struct m3irq_line {
	unsigned int id;
	void (*irq_handler)(void);
};

/**
 * @brief	Clears a bit from the mask of a given controller
 * @param	dev: The device
 * @param	i: The line index
 * @return	0 if no error, not 0 otherwise
 */
int m3irq_clear_mask(struct m3irq *dev, int i);

/**
 * @brief	Sets a bit from the mask of a given controller
 * @param	dev: The device
 * @param	i: The line index
 * @return	0 if no error, not 0 otherwise
 */
int m3irq_set_mask(struct m3irq *dev, int i);

/**
 * @brief	Clears all masks of a given controller
 * @param	dev: The device
 * @return	0 if no error, not 0 otherwise
 */
int m3irq_clear_all_masks(struct m3irq *dev);

/**
 * @brief	Sets all masks of a given controller
 * @param	dev: The device
 * @return	0 if no error, not 0 otherwise
 */
int m3irq_set_all_masks(struct m3irq *dev);

/**
 * @brief	Provides an M3 IRQ controller according to its index
 * @param	i: The device index (from 0 to M3IRQ_MAX_CONTROLLER)
 * @return	The corresponding device if found, NULL otherwise
 */
struct m3irq *m3irq_get_device_at_index(int i);

/**
 * @brief	Initializes the m3 irq controllers
 * @return	0 if no error, not 0 otherwise
 */
int m3irq_init(void);

#endif /* __STA_M3_IRQ_H__ */
