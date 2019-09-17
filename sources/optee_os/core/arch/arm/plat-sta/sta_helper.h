/*
 * Copyright (c) 2016, Linaro Limited
 * Copyright (c) 2014, STMicroelectronics International N.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef STA_HELPER_H
#define STA_HELPER_H

#include <sys/cdefs.h>
#include <stdint.h>
#include <util.h>
#include <arm.h>

/* PMCR definitions */
#define PMCR_N_SHIFT		11
#define PMCR_N_MASK		0x1f
#define PMCR_N_BITS		(PMCR_N_MASK << PMCR_N_SHIFT)

/* HCR definitions */
#define HCR_AMO_BIT		BIT(5)
#define HCR_IMO_BIT		BIT(4)
#define HCR_FMO_BIT		BIT(3)
#define HCR_RESET_VAL		0x0

/* CNTHCTL definitions */
#define CNTHCTL_RESET_VAL	0x0
#define PL1PCEN_BIT		BIT(1)
#define PL1PCTEN_BIT		BIT(0)

/* HCPTR definitions */
#define HCPTR_RES1		(BIT(13) | BIT(12) | 0x3ff)
#define TCPAC_BIT		BIT(31)
#define TTA_BIT			BIT(20)
#define TCP11_BIT		BIT(10)
#define TCP10_BIT		BIT(10)
#define HCPTR_RESET_VAL		HCPTR_RES1

/* ID_PFR1 definitions */
#define ID_PFR1_VIRTEXT_SHIFT	12
#define ID_PFR1_VIRTEXT_MASK	0xf

/* VTTBR definitions */
#define VTTBR_RESET_VAL		0x0

/* HDCR definitions */
#define HDCR_RESET_VAL		0x0

/* HSTR definitions */
#define HSTR_RESET_VAL		0x0

/* CNTHP_CTL definitions */
#define CNTHP_CTL_RESET_VAL	0x0

/******************************************************************************
 * Function prototypes
 ******************************************************************************/
void set_cpsr_mode_mon(void);
void set_cpsr_mode_svc(void);
void sta_cpu_resume(void);
void plat_cpu_resume_reset_late(void);

/******************************************************************************
 * Functions inline
 ******************************************************************************/
#ifndef ASM

static inline uint32_t read_id_pfr1(void)
{
	uint32_t reg;

	asm volatile ("mrc	p15, 0, %[reg], c0, c1, 1"
			: [reg] "=r" (reg));
	return reg;
}

static inline uint32_t read_scr(void)
{
	uint32_t scr;

	asm volatile ("mrc	p15, 0, %[scr], c1, c1, 0"
			: [scr] "=r" (scr));

	return scr;
}

static inline uint32_t read_midr(void)
{
	uint32_t midr;

	asm volatile ("mrc	p15, 0, %[midr], c0, c0, 0"
			: [midr] "=r" (midr));

	return midr;
}

static inline uint32_t read_pmcr(void)
{
	uint32_t pmcr;

	asm volatile ("mrc	p15, 0, %[pmcr], c9, c12, 0"
			: [pmcr] "=r" (pmcr));

	return pmcr;
}

static inline void write_hcr(uint32_t va)
{
	asm volatile ("mcr	p15, 4, %0, c1, c1, 0" : : "r" (va));
}

static inline void write_hcptr(uint32_t va)
{
	asm volatile ("mcr	p15, 4, %0, c1, c1, 2" : : "r" (va));
}

static inline void write_hdcr(uint32_t va)
{
	asm volatile ("mcr	p15, 4, %0, c1, c1, 1" : : "r" (va));
}

static inline void write_hstr(uint32_t va)
{
	asm volatile ("mcr	p15, 4, %0, c1, c1, 3" : : "r" (va));
}

static inline void write_vpidr(uint32_t va)
{
	asm volatile ("mcr	p15, 4, %0, c0, c0, 0" : : "r" (va));
}

static inline void write_vmpidr(uint32_t va)
{
	asm volatile ("mcr	p15, 4, %0, c0, c0, 5" : : "r" (va));
}

static inline void write_cnthp_ctl(uint32_t va)
{
	asm volatile ("mcr	p15, 4, %0, c14, c2, 1" : : "r" (va));
}

static inline void write_cnthctl(uint32_t va)
{
	asm volatile ("mcr	p15, 4, %0, c14, c1, 0" : : "r" (va));
}

static inline void write64_cntvoff(uint64_t va)
{
	asm volatile ("mcrr	p15, 4, %Q0, %R0, c14" : : "r" (va));
}

static inline void write64_vttbr(uint64_t va)
{
	asm volatile ("mcrr	p15, 6, %Q0, %R0, c2" : : "r" (va));
}

#endif /*ASM*/

#endif /*STA_HELPER_H*/
