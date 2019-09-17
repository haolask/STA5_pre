/*
 *  arch/arm/mach-sta/smp.h
 *
 * Copyright (C) 2016 STMicroelectronics (R&D) Limited.
 *		http://www.st.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __MACH_STA_SMP_H
#define __MACH_STA_SMP_H

extern const struct smp_operations sta_smp_ops;

void sta_secondary_startup(void);
void sta_smp_prepare_cpus(unsigned int max_cpus);

#endif
