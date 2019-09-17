/**
 * @file sta_adc.h
 * @brief This file provides all the Analog Digital Converter header file
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */
#ifndef __STA_ADC_H__
#define __STA_ADC_H__

#include "sta_map.h"

#define MAX_ADC_CHANNEL 10

/**
 * @brief		read (blocking mode) data on a given channel
 * @param[in]	adc: compulsory adc description
 * @param[in]	chnl: the given channel on which perform the measurement
 * @param[out]	data: the read data
 * @retval 0 if no error, not 0 otherwise
 */
int sta_adc_read_data(t_adc *adc, uint8_t chnl, uint32_t *data);

/**
 * @brief		read ADC version
 * @param[in]	adc: compulsory adc description
 * @retval id read from register, 0 on error
 */
uint32_t sta_adc_version(t_adc *adc);

/**
 * @brief		basic ADC initialize
 * @param[in]	adc: compulsory adc description
 * @retval 0 if no error, not 0 otherwise
 */
int sta_adc_init(t_adc *adc);

#endif /* __STA_ADC_H__ */

