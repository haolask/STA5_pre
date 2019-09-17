/*
 * Copyright (C) ST Microelectronics 2015
 *
 * License terms:
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

.section ".data"

.balign 4

.global dsp_x_start
dsp_x_start:
   .word X_START
.global dsp_x_size
dsp_x_size:
   .word (X_END-X_START)

.global dsp_y_start
dsp_y_start:
   .word Y_START
.global dsp_y_size
dsp_y_size:
   .word (Y_END-Y_START)

.global dsp_p_start
dsp_p_start:
   .word P_START
.global dsp_p_size
dsp_p_size:
   .word (P_END-P_START)

.balign 4

X_START:
   .incbin "dspfw/emerald-firmware.X.noheader"
X_END:

.balign 4

Y_START:
   .incbin "dspfw/emerald-firmware.Y.noheader"
Y_END:

.balign 4

P_START:
   .incbin "dspfw/emerald-firmware.P.noheader"
P_END:

.end

