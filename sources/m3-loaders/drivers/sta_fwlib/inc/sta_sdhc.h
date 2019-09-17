/**
 * @file sta_sdhc.h
 * @brief This file provides useful definitions for Arasan SDHC IP
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: ADG-MID team
 */

#ifndef STA_SDHC_H
#define STA_SDHC_H

/**
 * @brief	Initialize Arasan SDHC driver
 */
void sdhc_init(struct t_mmc_ctx *mmc, uint32_t regs_base);

#endif /* STA_SDHC_H */
