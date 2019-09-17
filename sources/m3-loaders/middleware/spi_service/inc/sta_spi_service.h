/**
 *                         (C) 2009 STMicroelectronics
 *  Reproduction and Communication of this document is strictly prohibited
 *    unless specifically authorized in writing by STMicroelectronics.
 *-----------------------------------------------------------------------------
 *                              APG / CRM / SA&PD
 *             Software Development Group - SW platform & HW Specific
 *
 *
 * @file sta_spi_service.h
 * @brief This module provides the API for SSP svc_mcu.
 */

#ifndef SVC_SSP_H
#define SVC_SSP_H

#include "sta_map.h"
#include "sta_spi.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <stdlib.h>

/*****************************************************************************
   defines and macros
*****************************************************************************/

#define SVC_SSP_AVAIL_PORTS       3

typedef struct svc_ssp_com_handler_s svc_ssp_com_handler_t;

typedef struct svc_spi_comconfig_s
{
	uint32_t out_clk;
	LLD_SSP_DataSizeTy data_size;  /**< size of data elements (4 to 32 bits)         */
	LLD_SSP_ClkPhaseTy clk_phase;  /**< Motorola SPI interface Clock phase           */
	LLD_SSP_ClkPolarityTy clk_pol; /**< Motorola SPI interface Clock polarity        */
} svc_spi_comconfig_t;

typedef void (*svc_ssp_hook_t)(void *);
typedef void *svc_ssp_hook_param_t;

/*****************************************************************************
   function prototypes
*****************************************************************************/

int svc_ssp_init(uint32_t bus_speed);

int svc_ssp_open_port(t_spi *ssp_port, int irq_pri);

svc_ssp_com_handler_t *svc_ssp_create_com(t_spi *ssp_port,
					  LLD_SSP_InterfaceTy ssp_mode,
					  uint32_t out_clk,
					  LLD_SSP_DataSizeTy data_size,
					  svc_ssp_hook_t pre_cb,
					  svc_ssp_hook_param_t pre_cb_param,
					  svc_ssp_hook_t post_cb,
					  svc_ssp_hook_param_t post_cb_param);

int svc_ssp_com_setmode(svc_ssp_com_handler_t *ssp_com_hdlr,
			LLD_SSP_InterfaceTy ssp_mode);

int svc_ssp_com_getmode(svc_ssp_com_handler_t *ssp_com_hdlr,
			LLD_SSP_InterfaceTy *ssp_mode_ptr);

int svc_ssp_write(svc_ssp_com_handler_t *ssp_com_hdlr,
		  void *out_buf, uint32_t len, void *in_buf);

bool svc_ssp_is_busy(svc_ssp_com_handler_t *ssp_com_hdlr);

void svc_ssp_lock(svc_ssp_com_handler_t *ssp_com_hdlr);

void svc_ssp_release(svc_ssp_com_handler_t *ssp_com_hdlr);

void SPI0_IRQHandler(void);

void SPI1_IRQHandler(void);

void SPI2_IRQHandler(void);

#endif /* SVC_SSP_H */
