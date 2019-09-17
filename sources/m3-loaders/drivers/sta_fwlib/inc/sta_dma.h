/**
 * @file sta_dma.h
 * @brief This file provides all the Audio driver definitions.
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#ifndef _DMA_H_
#define _DMA_H_

#include "sta_map.h"

#define DMA_CHANNELS	8

/* DMA linked list structure */
typedef struct {
	uint32_t DMACCSrcAddr;
	uint32_t DMACCDestAddr;
	DMACCLLITy DMACCLLI;
	DMA_ChannelControlRegisterTy DMACCControl;
} DMA_LinkedListTy;

typedef enum {
	DMA_LITTLE_ENDIAN = 0x0,
	DMA_BIG_ENDIAN    = 0x1
} DMA_EndianessTy;

/* DMA master */
typedef enum {
	DMA_MASTER1 = 0,
	DMA_MASTER2 = 1
} DMA_MasterTy;

/* DMA burst size */
typedef enum {
	DMA_BURST_SIZE_1   = 0x0,
	DMA_BURST_SIZE_4   = 0x1,
	DMA_BURST_SIZE_8   = 0x2,
	DMA_BURST_SIZE_16  = 0x3,
	DMA_BURST_SIZE_32  = 0x4,
	DMA_BURST_SIZE_64  = 0x5,
	DMA_BURST_SIZE_128 = 0x6,
	DMA_BURST_SIZE_256 = 0x7
} DMA_BurstSizeTy;

/* DMA transfer width */
typedef enum {
	DMA_BYTE_WIDTH     = 0x0,
	DMA_HALFWORD_WIDTH = 0x1,
	DMA_WORD_WIDTH     = 0x2
} DMA_TransferWidthTy;

/* DMA flow control */
typedef enum {
	MEMORY_TO_MEMORY_DMA_CONTROLLER                             = 0x0,
	MEMORY_TO_PERIPHERAL_DMA_CONTROLLER                         = 0x1,
	PERIPHERAL_TO_MEMORY_DMA_CONTROLLER                         = 0x2,
	PERIPHERAL_TO_PERIPHERAL_DMA_CONTROLLER                     = 0x3,
	PERIPHERAL_TO_PERIPHERAL_DESTINATION_PERIPHERAL_CONTROLLER  = 0x4,
	MEMORY_TO_PERIPHERAL_PERIPHERAL_CONTROLLER                  = 0x5,
	PERIPHERAL_TO_MEMORY_PERIPHERAL_CONTROLLER                  = 0x6,
	PERIPHERAL_TO_PERIPHERAL_SOURCE_PERIPHERAL_CONTROLLER       = 0x7
} DMA_FlowControlTy;

/* DMA 0: Request Line DMA Source */
typedef enum DMA0Req {
	DMA0_EFT0	    = 0,
	DMA0_EFT1	    = 1,
	DMA0_EFT2           = 2,
	DMA0_RES0           = 3,
	DMA0_RES1           = 4,
	DMA0_RES2           = 5,
	DMA0_SSP1_RX        = 6,
	DMA0_SSP2_RX        = 7,
	DMA0_RES3           = 8,
	DMA0_SSP1_TX        = 9,
	DMA0_SSP2_TX        = 10,
	DMA0_RES4           = 11,
	DMA0_UART1_RX       = 12,
	DMA0_UART2_RX       = 13,
	DMA0_UART3_RX       = 14,
	DMA0_RES5           = 15,
	DMA0_UART1_TX       = 16,
	DMA0_UART2_TX       = 17,
	DMA0_UART3_TX       = 18,
	DMA0_MSP0_TX        = 19,
	DMA0_MSP1_TX        = 20,
	DMA0_MSP2_TX        = 21,
	DMA0_MSP0_RX        = 22,
	DMA0_MSP1_RX        = 23,
	DMA0_MSP2_RX        = 24,
	DMA0_RES6           = 25,
	DMA0_I2C1           = 26,
	DMA0_I2C2           = 27,
	DMA0_SDMMC0         = 28,
	DMA0_SDMMC1         = 29,
	DMA0_VIP_REQ_EVEN   = 30,
	DMA0_VIP_REQ_ODD    = 31,
} DMA0Req;

/* DMA 1: Request Line DMA Source */
typedef enum DMA1Req {
	DMA1_DSPFIFO1         = 0,
	DMA1_DSPFIFO2         = 1,
	DMA1_RES0             = 2,
	DMA1_CD_CHITF         = 3,
	DMA1_MSP0_TX          = 4,
	DMA1_MSP1_TX          = 5,
	DMA1_MSP2_TX          = 6,
	DMA1_MSP0_RX          = 39,
	DMA1_MSP1_RX          = 40,
	DMA1_MSP2_RX          = 41,
	DMA1_RES7             = 42,
	DMA1_SSP1_RX          = 43,
	DMA1_TSC_FIFO         = 44,
	DMA1_RES8             = 45,
	DMA1_SSP1_TX          = 46,
	DMA1_SQI0             = 47,
	DMA1_VIP_REQ_EVEN     = 48,
	DMA1_VIP_REQ_ODD      = 49,
	DMA1_SDMMC0           = 50,
	DMA1_SDMMC1           = 51,
	DMA1_RES9             = 52,
	DMA1_UART1_RX         = 53,
	DMA1_UART2_RX         = 54,
	DMA1_UART3_RX         = 55,
	DMA1_RES10            = 56,
	DMA1_UART1_TX         = 57,
	DMA1_UART2_TX         = 58,
	DMA1_UART3_TX         = 59,
	DMA1_FSMC_FILL_FIFO   = 60,
	DMA1_EXT_REQ          = 61,
	DMA1_SQI1             = 62,
	DMA1_CD_SPDIF         = 63
} DMA1Req;

typedef  void (*dma_callback_t)(int);

void dma0_irq_handler(void);
int dma0_request(int chan, dma_callback_t callback);
#endif  /* _DMA_H_ */
