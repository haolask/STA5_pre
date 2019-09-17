/**
 * @file sta_cot.h
 * @brief This file provides the Chain Of Trust header
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: ADG-MID team
 */

#ifndef _STA_COT_H_
#define _STA_COT_H_

/**
 * @brief	Init the Chain of Trust
 * @return	0 if no error, not 0 otherwise
 */
int cot_init(void);

/**
 * @brief	Check AP XL digest
 * @return	0 if no error, not 0 otherwise
 */
int cot_check_ap_xl(struct xl1_part_info_t *part_info);

#endif /* _STA_COT_H_ */
