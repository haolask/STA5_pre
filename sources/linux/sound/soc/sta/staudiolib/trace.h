/*
 * Copyright (C) ST Microelectronics 2017
 *
 * Author:	Gian Antonio Sampietro <gianantonio.sampietro@st.com>,
 *		for STMicroelectronics.
 *
 * License terms:
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */
#ifndef ST_TRACE_DSP_H
#define ST_TRACE_DSP_H

#define TRACE_PRINT dsp_trace_print
int dsp_trace_init(void);
void dsp_trace_enable(bool status);
void dsp_trace_print(u32 addr, u32 value);

#endif
