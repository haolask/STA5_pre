/**
 * @file sta_dsp.h
 * @brief This file provides all the DSP firmware header
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#ifndef _STA_DSP_H_
#define _STA_DSP_H_

enum DSP_E {
	DSP0 = 0,
	DSP1,
	DSP2,
	DSP_MAX_NO
};

/**
 * @brief	Remove DSP from reset (start the DSP)
 * @param	port: the DSP port to initialize
 * @return	0 if no error, not 0 otherwise
 */
int dsp_start(int port);

/**
 * @brief	Enable clock of the given DSP
 * @param	port: the DSP port
 * @return	0 if no error, not 0 otherwise
 */
int dsp_clk_enable(int port);

/**
 * @brief	Disable clock of the given DSP
 * @param	port: the DSP port
 * @return	0 if no error, not 0 otherwise
 */
int dsp_clk_disable(int port);

#endif /* _STA_DSP_H_ */
