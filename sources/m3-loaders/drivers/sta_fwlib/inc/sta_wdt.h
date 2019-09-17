/**
 * @file sta_wdt.h
 * @brief Watchdog driver for M3 watchdog modules
 * Copyright (C) 2017 ST Microelectronics
 * @author: Jean-Nicolas GRAUX <jean-nicolas.graux@st.com>
 */

#ifndef _STA_WDT_H_
#define _STA_WDT_H_

/**
 * @brief watchdog platoform data structure
 * @timeout: watchdog timeout value in seconds
 * @reload_interval: time in seconds in between each reload
 * @reload_auto: if set, watchdog is automatically reloaded
 * @clk_rate: watchdog input clock rate
 * @reg_base: peripheral base address
 */
struct sta_wdt_pdata {
	unsigned int timeout;
	unsigned int reload_interval;
	bool reload_auto;
	unsigned long long clk_rate;
	volatile t_wdt *reg_base;
};

int wdt_init(struct sta_wdt_pdata *);
int wdt_enable(void);
int wdt_disable(void);
void wdt_task(void *);
void wdt_reload_auto(bool);
int wdt_reload(unsigned int);

#endif

