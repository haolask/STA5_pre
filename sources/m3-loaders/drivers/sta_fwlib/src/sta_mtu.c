/**
 * @file sta_mtu.c
 * @brief Multi Timer Unit (MTU) functions
 *
 * It allows to dynamically allocate a timer and select the functional mode
 * for each MTU unit. A background timer allowing to generate a free-running
 * time base in second, millisecond and/or microsecond can also be optionally
 * started.
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include <stdint.h>
#include <errno.h>
#include <string.h>

#include "utils.h"
#include "trace.h"

#include "sta_mtu.h"
#include "sta_map.h"
#include "sta_platform.h"
#include "sta_nvic.h"

#define MTU_M3_DIVIDER	8


#define PERIODIC_TIMING		    (1000000000UL)	/* 1000sec */

/* Registers dynamic offsets */
#define MTU_TLR(y)				(0x10 + (y * 0x10))
#define MTU_TVAL(y)				(0x14 + (y * 0x10))
#define MTU_TCR(y)				(0x18 + (y * 0x10))
#define MTU_TBGLR(y)			(0x1C + (y * 0x10))

#define TIMER_OS_SHOT_COUNT		BIT(0)
#define TIMER_SIZE_32	        BIT(1)
#define TIMER_MODE_PERIODIC		BIT(6)
#define TIMER_EN				BIT(7)
#define TIMER_EN_SHIFT			7

#define ICR_ALL_ITS     0xF
#define MTU_MAX_INST    4

/**
 * struct context - MTU overall context
 * @current_mtu_base: current MTU base address
 * @devs: list of mtu_devices
 * @time: current time reference
 */
struct context {
	t_mtu *current_mtu_base;
	struct mtu_device devs[MTU_MAX_INST];
	struct mtu_time time;
	unsigned int mxtal;
};

static struct context mtu_ctx;


/**
  * @brief  updates timebase
  * @param	None
  * @retval None
  */
static inline void mtu_time_base(void *p)
{
	mtu_ctx.time.t_sec += (PERIODIC_TIMING / 1000000);
}

/**
  * @brief  This routine initializes the MTU timer instances of a selected
  * device. If requested, a background periodic timer is configured and
  * started to handle a time base (in sec, msec, usec).
  * @param	mtu_base: base address of the MTU device
  * @param	timebase: flag to request a background timer starting for one
  * sec time base handlingoutput divison factor (0..15)
  * @retval 0 if no error, not 0 otherwise
  */
int mtu_init(t_mtu *mtu_base, int timebase)
{
	int err;
	int i;
	struct nvic_chnl irq_chnl;
	struct mtu_device *mtu;

	/* Enable MTU interrupt if not already done */
	irq_chnl.id = MTU_IRQChannel;
	irq_chnl.preempt_prio = IRQ_LOW_PRIO;
	irq_chnl.enabled = true;

	err = nvic_chnl_init(&irq_chnl);
	if (err)
		return err;

	/* Clear all IT */
	mtu_base->icr = ICR_ALL_ITS;

	/* Init MTU parameters for each timer instance */
	for (i = 0, mtu = mtu_ctx.devs; i < MTU_MAX_INST; i++, mtu++) {
		set_bit_reg(TIMER_SIZE_32, (uint32_t) mtu_base + MTU_TCR(i));
		/* Default configuration */
		/* - free-running mode  */
		/* - clk division to 1 */
		/* - Timer disable */
	}
	/* Set MTU HW IP used */
	mtu_ctx.current_mtu_base = mtu_base;

	/* Set Mxtal informationsTU HW IP used */
	mtu_ctx.mxtal = m3_get_mxtal() / 1000000;

	/* Backgound timer requested */
	if (timebase == BG_TIMER_ON) {
		/* Configure and start a 1000s (~17 min) periodic timer to enable a time base */
		mtu_ctx.time.dev =
		    mtu_start_timer(PERIODIC_TIMING, PERIODIC_MODE,
				    mtu_time_base);
	}

	return 0;
}

/**
  * @brief  Changes timer state (enable/disable) of a given mtu
  * @param	mtu: the current mtu to consider
  * @param	enable: set to true to enable, false to disable
  * @retval 0 if no error, not 0 otherwise
  */
static int __mtu_timer_change_state(struct mtu_device *mtu, bool enable)
{
	uint32_t ctrl;

	if (!mtu)
		return -EINVAL;

	ctrl = read_reg((uint32_t) mtu->base + MTU_TCR(mtu->id));

	if (enable && !(ctrl >> TIMER_EN_SHIFT))
		ctrl |= TIMER_EN;
	else if (!enable && (ctrl >> TIMER_EN_SHIFT))
		ctrl &= ~TIMER_EN;

	write_reg(ctrl, (uint32_t) mtu->base + MTU_TCR(mtu->id));

	return 0;
}

/**
  * @brief  Enables the timer of a given mtu
  * @param	mtu: the current mtu to consider
  * @retval 0 if no error, not 0 otherwise
  */
static inline int mtu_timer_enable(struct mtu_device *mtu)
{
	return __mtu_timer_change_state(mtu, true);
}

/**
  * @brief  Disables the timer of a given mtu
  * @param	mtu: the current mtu to consider
  * @retval 0 if no error, not 0 otherwise
  */
static inline int mtu_timer_disable(struct mtu_device *mtu)
{
	return __mtu_timer_change_state(mtu, false);
}

/**
  * @brief  Sets the timer mode of a given mtu
  * @param	mtu: the current mtu to consider
  * @param	mode: the mode to be applied (Free running, oneshot or periodic)
  * @retval 0 if no error, not 0 otherwise
  */
static int mtu_set_timer_mode(struct mtu_device *mtu, int mtu_mode)
{
	uint32_t ctrl;

	if (!mtu)
		return -EINVAL;

	ctrl = read_reg((uint32_t) mtu->base + MTU_TCR(mtu->id));

	/* Set timer mode */
	switch (mtu_mode) {
	case ONE_SHOT_MODE:
		ctrl |= TIMER_OS_SHOT_COUNT;
		break;
	case PERIODIC_MODE:
		ctrl |= TIMER_MODE_PERIODIC;
		break;
	case FREE_RUNNING_MODE:
		ctrl &= ~TIMER_OS_SHOT_COUNT; /* wrapping mode */
		ctrl &= ~TIMER_MODE_PERIODIC;	 /* free running mode */
		break;
	default:
		return -EINVAL;
	}

	write_reg(ctrl, (uint32_t) mtu->base + MTU_TCR(mtu->id));

	return 0;
}

/**
  * @brief  Sets the timer counter of a given mtu
  * @param	mtu: the current mtu to consider
  * @param	udelay: timer counter
  * @retval 0 if no error, not 0 otherwise
  */
static int mtu_set_timer_counter(struct mtu_device *mtu, uint32_t udelay)
{
	if (!mtu)
		return -EINVAL;

	/* set the passed loadvalue in the down counter */
	write_reg(udelay, (uint32_t) mtu->base + MTU_TLR(mtu->id));

	return 0;
}

/**
  * @brief  Starts a timer
  * @param	udelay the timer counter
  * @param	mtu_mode the operating mode of the timer (Oneshot or periodic)
  * @param	cb hook to be called once the timer expires
  * @retval 0 if no error, not 0 otherwise
  */
struct mtu_device *mtu_start_timer(uint32_t udelay, int mtu_mode,
				   void (*cb) (void *p))
{
	uint32_t i;
	uint32_t ctrl;
	struct mtu_device *mtu;
	uint32_t udelay_tick;

	if (udelay > ((0x100000000 / mtu_ctx.mxtal) * MTU_M3_DIVIDER)) {
		udelay_tick = 0xFFFFFFFF;
	} else {
		udelay_tick = udelay / MTU_M3_DIVIDER * mtu_ctx.mxtal;
	}

	/* Search 1st free  MTU parameters for each timer instance */
	for (i = 0, mtu = mtu_ctx.devs; i < MTU_MAX_INST; i++, mtu++) {
		if (mtu->base != NULL)
			continue;

		/* Free timer found: Configure and start it */
		mtu->base = mtu_ctx.current_mtu_base;
		mtu->id = i;

		if (cb) {
			mtu->mtu_cb = cb;
			/* Enable MTU interrupt */
			mtu_ctx.current_mtu_base->icr |= BIT(i);
			mtu_ctx.current_mtu_base->imsc |= BIT(i);
		}

		ctrl = read_reg((uint32_t) mtu->base + MTU_TCR(mtu->id));
		/*  Do nothing if background periodic timer is already ON */
		if ((udelay != PERIODIC_TIMING) ||
		    !(ctrl & TIMER_EN)) {
			mtu_set_timer_mode(mtu, mtu_mode);
			mtu_set_timer_counter(mtu, udelay_tick);
			mtu_timer_enable(mtu);
		}

		return mtu;
	}
	return NULL;
}

/**
  * @brief Stops a timer
  * @param	mtu_mode the operating mode of the timer (Oneshot or periodic)
  * @retval 0 if no error, not 0 otherwise
  */
int mtu_stop_timer(struct mtu_device *mtu)
{
	if (!mtu)
		return -EINVAL;

	/* Init timer mode to default value */
	mtu_set_timer_mode(mtu, FREE_RUNNING_MODE);
	mtu_timer_disable(mtu);

	/* Disable MTU interrupt */
	mtu_ctx.current_mtu_base->imsc &= ~BIT(mtu->id);

	/* Init MTU device instance */
	memset(mtu, 0, sizeof(*mtu));

	return 0;
}

/**
  * @brief	Returns timer value of a given mtu
  * @param	mtu device
  * @retval timer value
  */
uint32_t mtu_read_timer_value(struct mtu_device *mtu)
{
	return read_reg((uint32_t) mtu->base + MTU_TVAL(mtu->id));
}

/**
  * @brief  Waits for a given delay (in µsecconds)
  * @param	udelay: amount of µsec to wait for
  * @retval 0 if no error, not 0 otherwise
  */
int mtu_wait_delay(uint32_t udelay)
{
	struct mtu_device *mtu;

	/* Configure and start timer to expire after delay value */
	mtu = mtu_start_timer(udelay, ONE_SHOT_MODE, NULL);
	if (!mtu)
		return -ENODEV;

	/* Loop till the TIMER value is not zero */
	while (mtu_read_timer_value(mtu)) ;

	mtu_stop_timer(mtu);

	return 0;
}

/**
  * @brief  Provides the timebase according to the unit of time supplied as
  * parameter
  * @param	unit of time to be used (seconds, milliseconds and µseconds)
  * @retval timebase according to the given unit of time
  */

uint32_t mtu_get_timebase(int unit)
{
	uint32_t current_time;

	if (mtu_ctx.time.dev)
		current_time = (mtu_read_timer_value(mtu_ctx.time.dev) / mtu_ctx.mxtal)
						* MTU_M3_DIVIDER;
	else
		return 0;

	switch (unit) {
	case TIME_IN_SEC:
		return mtu_ctx.time.t_sec +
		    ((PERIODIC_TIMING - current_time) / 1000000);

	case TIME_IN_MS:
		return ((PERIODIC_TIMING - current_time) % 1000000) / 1000;

	case TIME_IN_US:
		return (PERIODIC_TIMING - current_time) % 1000000;

	default:
		break;
	}

	return PERIODIC_TIMING - current_time;
}

/**
  * @brief  MTU interrupt handler
  * @param	None
  * @retval None
  */
void mtu_irq_handler(void)
{
	uint32_t mis, n;
	struct mtu_device *mtu;

	/* Get pending MTU Masked Interrupts Status */
	mis = mtu_ctx.current_mtu_base->mis;

	/* Handle pending interrupt */
	for (n = 0; mis; n++) {
		if (mis & BIT(n)) {
			mis -= BIT(n);

			mtu = &mtu_ctx.devs[n];

			if (mtu->mtu_cb)
				/* Handle received message */
				mtu->mtu_cb((void *)mtu);
			else
				TRACE_ERR
				    ("MTU_IRQHandler: Unexpected MTU interrupt [id:%d]\n",
				     n);
			mtu_ctx.current_mtu_base->icr |= BIT(n);
		}
	}
}
