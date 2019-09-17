/**
 * @file sta_pinmux.h
 * @brief This file provides all pinmux headers
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#ifndef _STA_PINMUX_H_
#define _STA_PINMUX_H_

#include "sta_common.h"
#include "sta_gpio.h"

#if (SOC_ID == SOCID_STA1195) || (SOC_ID == SOCID_STA1295) || (SOC_ID == SOCID_STA1275)
#define A7_GPIO_PIN_OFFSET 0
#define A7_GPIO_PIN_MAX 127
#define S_GPIO_PIN_OFFSET 128
#define S_GPIO_PIN_MAX 143
#define M3_GPIO_PIN_OFFSET 144
#define M3_GPIO_PIN_MAX 159
/* On STA1x95 SoCs there are no explicitly named "Always-On" GPIOs, so
 * consider them as M3_GPIO */
#define AO_GPIO_PIN_OFFSET M3_GPIO_PIN_OFFSET
#define AO_GPIO_PIN_MAX M3_GPIO_PIN_MAX
#elif (SOC_ID == SOCID_STA1385)
#define A7_GPIO_PIN_OFFSET 0
#define A7_GPIO_PIN_MAX 87
#define S_GPIO_PIN_OFFSET 88
#define S_GPIO_PIN_MAX 103
#define M3_GPIO_PIN_OFFSET 104
#define M3_GPIO_PIN_MAX 119
#define AO_GPIO_PIN_OFFSET 120
#define AO_GPIO_PIN_MAX 125
#else
#error "No SoC defined !"
#endif

#define A7_GPIO(x)	(x + A7_GPIO_PIN_OFFSET)
#define M3_GPIO(x)	(x + M3_GPIO_PIN_OFFSET)
#define S_GPIO(x)	(x + S_GPIO_PIN_OFFSET)
#define AO_GPIO(x)	(x + AO_GPIO_PIN_OFFSET)

/**
 * struct pinmux - pin muxing definition
 * @name: the pinmux reference name, to be used by drivers to request mux
 * @mux: the mux definition
 * @num_mux: the number of mux defined in muxing
 */
struct pinmux {
	const char *name;
	const struct gpio_mux *mux;
	uint32_t num_mux;
};

#define __GPIOMUX(name, soc) static const struct gpio_mux name ## soc[]

#define DECLARE_STA1295_GPIOMUX(n) __GPIOMUX(n, sta1295)
#define DECLARE_STA1385_GPIOMUX(n) __GPIOMUX(n, sta1385)

#define _PINMUX(n, soc) { \
	.name = #n, \
	.mux = n ## soc, \
	.num_mux = NELEMS(n ## soc), \
}

#define STA1295_GPIOMUX(n) _PINMUX(n, sta1295)
#define STA1385_GPIOMUX(n) _PINMUX(n, sta1385)

/**
 * @brief	get and request a pinmux from its name
 *			in the case the same pinmux is defined twice (or more), the
 *			function will only get and apply the first.
 * @param	name of the pinmux
 * @param	pinmux if found, NULL otherwise
 * @return	0 if no error, not 0 otherwise
 */
int pinmux_request(const char *name);

#endif /* _STA_PINMUX_H_ */
