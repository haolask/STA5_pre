/**
 * @file sta_ks.h
 * @brief Key storage proxy service
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: ADG-MID team
 */
#if !defined STA_KS_H
#define STA_KS_H

/**
 * @brief	Key storage Interrupt Service Routine
 * @param	None
 * @return	None
 */
void ks_irq_handler(void);

/**
 * @brief	Main function for key storage proxy management
 * @param	None
 * @return	None
 */
void key_storage_task(void);

#endif
