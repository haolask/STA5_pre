/**
 * @file sta_adc.c
 * @brief This file provides all the Analog Digital Converter utilities functions
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include <errno.h>

#include "utils.h"
#include "sta_adc.h"

#define ADC_VERSION 1

#define ADC_REF_EXTERNAL 0
#define ADC_REF_INTERNAL 1

#define ADC_HCLK_BY16	0
#define ADC_HCLK_BY8	1

/**
 * @brief		read (blocking mode) data on a given channel
 * @param[in]	adc: compulsory adc description
 * @param[in]	chnl: the given channel on which perform the measurement
 * @param[out]	data: the read data
 * @retval 0 if no error, not 0 otherwise
 */
int sta_adc_read_data(t_adc *adc, uint8_t chnl, uint32_t *data)
{
	uint32_t eoc;

	if (!adc || chnl > MAX_ADC_CHANNEL)
		return -EINVAL;

	adc->adcctrl.bit.ref_sel = ADC_REF_EXTERNAL;
	/* The highest possible operating frequency for ADC is 20 MHz. */
	adc->adcctrl.bit.freq =	ADC_HCLK_BY16;

	/* start ADC capture and wait for the capture to be over */
	adc->adccapture.bit.chnl |= BIT(chnl);
	do {
		eoc = (adc->adccapture.bit.chnl >> chnl) & 1;
	} while (!eoc);

	/* get the value */
	*data = adc->adcdata[chnl].bit.data;

	return 0;
}

/**
 * @brief		read ADC version
 * @param[in]	adc: compulsory adc description
 * @retval id read from register, 0 on error
 */
uint32_t sta_adc_version(t_adc *adc)
{
	if (adc)
		return adc->id;

	return 0;
}

/**
 * @brief		basic ADC initialize
 * @param[in]	adc: compulsory adc description
 * @retval 0 if no error, not 0 otherwise
 */
int sta_adc_init(t_adc *adc)
{
	if (!adc)
		return -EINVAL;

	/* disable interrupts */
	adc->ier.bit.ie0 = 0;
	adc->ier.bit.ie1 = 0;
	adc->ier.bit.ie2 = 0;
	adc->ier.bit.ie3 = 0;
	adc->ier.bit.ie4 = 0;
	adc->ier.bit.ie5 = 0;
	/* disable DMA request */
	adc->ier.bit.dmaen = 0;

	/* read ID */
	if (sta_adc_version(adc) != ADC_VERSION)
		return -ENODEV;

	return 0;
}

