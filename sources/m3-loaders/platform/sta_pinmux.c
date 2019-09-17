/**
 * @file sta_pinmux.c
 * @brief This file provides all the STA specific pinmux functions
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */
#include <stdint.h>

#include "sta_common.h"
#include "sta_pinmux.h"

#include "sta1295_pinmux.h"
#include "sta1385_pinmux.h"

/**
 * @brief	get and request a pinmux from its name
 *			in the case the same pinmux is defined twice (or more), the
 *			function will only get and apply the first.
 * @param	name of the pinmux
 * @param	pinmux if found, NULL otherwise
 * @return	0 if no error, not 0 otherwise
 */
int pinmux_request(const char *name)
{
	unsigned int i;
	unsigned int nelems;
	const struct pinmux *p;

	switch(get_soc_id()) {
		case SOCID_STA1195:
		case SOCID_STA1295:
		case SOCID_STA1275:
			p = sta1295_pinmux;
			nelems = NELEMS(sta1295_pinmux);
			break;
		case SOCID_STA1385:
			p = sta1385_pinmux;
			nelems = NELEMS(sta1385_pinmux);
			break;
		default:
			TRACE_ERR("%s: no compatible SoC found\n", __func__);
			return -ENODEV;
	}

	for (i = 0; i < nelems; i++) {
		if (!strcmp(p[i].name, name)) {
			gpio_request_mux(p[i].mux, p[i].num_mux);
			return 0;
		}
	}

	TRACE_ERR("%s: no entry found for pinmux: %s\n", __func__, name);
	return -ENODEV;
}

