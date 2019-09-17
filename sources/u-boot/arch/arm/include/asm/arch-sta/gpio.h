/*
 * (C) Copyright 2016 ST APG
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

struct sta_gpio_platdata {
	u32 base;		/* Registers base address */
	const char *port_name;	/* Name of port gpioX */
};

