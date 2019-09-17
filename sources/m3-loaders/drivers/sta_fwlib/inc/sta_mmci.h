/**
 * @file sta_mmci.h
 * @brief This file provides useful definitions for the ARM PL180 MMCI IP
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: ADG-MID team
 */

#ifndef _STA_MMCI_H_
#define _STA_MMCI_H_

/**
 * @brief	MMC fast read utility routine, see MMC_Transfer.s
 */
uint32_t *mmc_fast_read(uint32_t *outbuf, void *mmci_regs_fifo);

/**
 * @brief	MMC fast write utility routine, see MMC_Transfer.s
 */
uint32_t *mmc_fast_write(uint32_t *inbuf, void *mmci_regs_fifo);

/**
 * @brief	Initialize ARM PL160 MMCI driver
 */
void mmci_init(struct t_mmc_ctx *mmc, uint32_t regs_base);

#endif /*_STA_MMCI_H_*/
