/**
 * @file sta_dsp.c
 * @brief This file provides all the DSP firmware functions
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include <errno.h>

#include "utils.h"

#include "sta_map.h"

#include "sta_dsp.h"

/**
 * @brief	Remove DSP from reset (start the DSP)
 * @param	port: the DSP port to initialize
 * @return	0 if no error, not 0 otherwise
 */
int dsp_start(int port)
{
	if (port != DSP0 && port != DSP1 && port != DSP2)
		return -EINVAL;

    /* Start the DSP */
	audsscr_regs->aco_cr.bit.dsp_reset |= BIT(port);

	return 0;
}

/**
 * @brief	Enable clock of the given DSP
 * @param	port: the DSP port
 * @return	0 if no error, not 0 otherwise
 */
int dsp_clk_enable(int port)
{
	if (port != DSP0 && port != DSP1 && port != DSP2)
		return -EINVAL;

	audsscr_regs->aco_cr.bit.dsp_enableclk |= BIT(port);

	return 0;
}

/**
 * @brief	Disable clock of the given DSP
 * @param	port: the DSP port
 * @return	0 if no error, not 0 otherwise
 */
int dsp_clk_disable(int port)
{
	if (port != DSP0 && port != DSP1 && port != DSP2)
		return -EINVAL;

	audsscr_regs->aco_cr.bit.dsp_enableclk &= ~BIT(port);

	return 0;
}

