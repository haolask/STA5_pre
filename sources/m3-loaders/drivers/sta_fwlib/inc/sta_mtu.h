/**
 * @file sta_mtu.h
 * @brief Multi Timer Unit (MTU) header file
 *
 * It allows to dynamically allocate a timer and select the functional mode
 * for each MTU unit. A background timer allowing to generate a free-running
 * time base in second, millisecond and/or microsecond can also be optionally
 * started.
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#ifndef __STA_MTU_H__
#define __STA_MTU_H__

#include "sta_lib.h"

/* Timer modes */
#define FREE_RUNNING_MODE	0
#define ONE_SHOT_MODE		1
#define	PERIODIC_MODE		2

/* Timebase reading type */
#define TIME_IN_SEC			0
#define TIME_IN_MS			1
#define TIME_IN_US			2

/* Background timer for timebase handling */
#define BG_TIMER_OFF		0
#define BG_TIMER_ON			1

/**
 * struct mtu_device - MTU device data
 * @id: MTU instance id
 * @base: MTU instance base address
 * @mtu_cb: atomic callback fetched every MTU interrupts
 */
struct mtu_device {
	int id;
	t_mtu *base;
	void (*mtu_cb) (void *timer);
};

/**
 * struct mtu_time - MTU Time base
 * @dev: MTU instance
 * @t_sec: Time base counter in seconds
 */
struct mtu_time {
	struct mtu_device *dev;
	uint32_t t_sec;
};

/**
  * @brief  This routine initializes the MTU timer instances of a selected
  * device. If requested, a background periodic timer is configured and
  * started to handle a time base (in sec, msec, usec).
  * @param	mtu_base: base address of the MTU device
  * @param	timebase: flag to request a background timer starting for one
  * sec time base handlingoutput divison factor (0..15)
  * @retval 0 if no error, not 0 otherwise
  */
int mtu_init(t_mtu *mtu_base, int timebase);

/**
  * @brief  Starts a timer
  * @param	udelay the timer counter
  * @param	mtu_mode the operating mode of the timer (Oneshot or periodic)
  * @param	cb hook to be called once the timer expires
  * @retval 0 if no error, not 0 otherwise
  */
struct mtu_device *mtu_start_timer(uint32_t udelay, int mtu_mode,
				   void (*cb) (void *p));

/**
  * @brief Stops a timer
  * @param	mtu_mode the operating mode of the timer (Oneshot or periodic)
  * @retval 0 if no error, not 0 otherwise
  */
int mtu_stop_timer(struct mtu_device *mtu);

/**
  * @brief	Returns timer value of a given mtu
  * @param	mtu device
  * @retval timer value
  */
uint32_t mtu_read_timer_value(struct mtu_device *mtu);

/**
  * @brief  Waits for a given delay (in µsecconds)
  * @param	udelay: amount of µsec to wait for
  * @retval 0 if no error, not 0 otherwise
  */
int mtu_wait_delay(uint32_t udelay);

/**
  * @brief  Provides the timebase according to the unit of time supplied as
  * parameter
  * @param	unit of time to be used (seconds, milliseconds and µseconds)
  * @retval timebase according to the given unit of time
  */
uint32_t mtu_get_timebase(int unit);

/**
  * @brief  Provides the timebase in microseconds from given base on u32
  * @retval timebase in microseconds
  */
static inline uint32_t mtu_get_time(uint32_t base)
{
	return mtu_get_timebase(TIME_IN_SEC) * 1000000
			+ mtu_get_timebase(TIME_IN_US) - base;
}

/**
  * @brief  MTU interrupt handler
  * @param	None
  * @retval None
  */
void mtu_irq_handler(void);

#define udelay(x)  mtu_wait_delay(x)
#define mdelay(x)  mtu_wait_delay((x) * 1000)
#define udelay_dummy(x) for(int i = 0; i < (100000 * x); i++)

#endif /* __STA_MTU_H__ */
