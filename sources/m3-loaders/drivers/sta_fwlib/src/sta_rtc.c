/**
 * @file sta_rtc.c
 * @brief Provide all the sta RTC driver functions.
 *
 * Copyright (C) ST-Microelectronics SA 2017
 * @author: Jean-Nicolas GRAUX <jean-nicolas.graux@st.com>
 */

#include <errno.h>

#include "sta_nvic.h"
#include "sta_pmu.h"
#include "sta_type.h"

static inline bool rtc_is_enabled(void)
{
	return rtc_regs->rtc_trc.bit.irtcen ? true : false;
}

static inline void rtc_enable(void)
{
	if (!rtc_is_enabled())
		rtc_regs->rtc_trc.bit.irtcen = 1;
}

static inline void rtc_disable(void)
{
	if (rtc_is_enabled())
		rtc_regs->rtc_trc.bit.irtcen = 0;
}

static inline void rtc_enable_irq(void)
{
	rtc_regs->rtc_imsc.bit.rtcimsc = 1;
}

static inline void rtc_disable_irq(void)
{
	rtc_regs->rtc_imsc.bit.rtcimsc = 0;
}

int rtc_get_time(uint32_t *time)
{
	if (!rtc_is_enabled())
		return -EIO;
	*time = rtc_regs->rtc_dr;
	return 0;
}

void rtc_set_time(uint32_t time)
{
	rtc_regs->rtc_dr = time;
	rtc_regs->rtc_ccr.bit.load_counter = 1;
}

int rtc_set_timeout(uint32_t timeout)
{
	uint32_t val;
	int ret;

	ret = rtc_get_time(&val);
	if (ret)
		return ret;
	rtc_regs->rtc_mr = val + timeout;
	return 0;
}

void RTC_IRQHandler(void)
{
	rtc_regs->rtc_icr.bit.rtcicr = 1;
	/* TODO: notify somebody */
}

int rtc_init(void)
{
	int ret;
	struct nvic_chnl irq_chnl;

	irq_chnl.id = RTC_IRQChannel;
	irq_chnl.preempt_prio = IRQ_LOW_PRIO;
	irq_chnl.enabled = true;
	ret = nvic_chnl_init(&irq_chnl);
	if (ret)
		return ret;

	rtc_enable_irq();
	rtc_enable();
	return 0;
}

int rtc_deinit(void)
{
	struct nvic_chnl irq_chnl;

	rtc_disable_irq();
	rtc_disable();
	irq_chnl.id = RTC_IRQChannel;
	irq_chnl.preempt_prio = IRQ_LOW_PRIO;
	irq_chnl.enabled = true;
	nvic_chnl_disable(&irq_chnl);
	return 0;
}
