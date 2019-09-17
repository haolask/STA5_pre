/**
 * @file sta_systick.c
 * @brief Provide Systick firmware functions
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include <errno.h>

#include "utils.h"

#include "sta_systick.h"

/* CTRL TICKINT Mask */
#define CTRL_TICKINT_SET      0x00000002
#define CTRL_TICKINT_RESET    (~0 & CTRL_TICKINT_SET)

/* SysTick Flag Mask */
#define FLAG_MASK             0x1F

#define IS_SYSTICK_CLK_SOURCE(s)	((s == SYSTICK_CLK_SOURCE_HCLK) || \
									(s == SYSTICK_CLK_SOURCE_HCLK_DIV8))

#define IS_SYSTICK_COUNTER(c)		((c == SYSTICK_COUNTER_DISABLE) || \
									(c == SYSTICK_COUNTER_ENABLE)  || \
									(c == SYSTICK_COUNTER_CLEAR))

#define IS_SYSTICK_FLAG(f)			((f == SYSTICK_FLAG_COUNT) || \
									(f == SYSTICK_FLAG_SKEW)  || \
									(f == SYSTICK_FLAG_NOREF))

#define SYSTICK_VALUE_MASK		0xFFFFFF

/**
 * @brief Configures the SysTick clock source.
 * @param - systick_clk_source: specifies the SysTick clock source.
 *          This parameter can be one of the following values:
 *              - SYSTICK_CLK_SOURCE_HCLK_DIV8: AHB clock divided by 8
 *              selected as SysTick clock source.
 *              - SYSTICK_CLK_SOURCE_HCLK: AHB clock selected as
 *              SysTick clock source.
 * @return 0 if no error, not 0 otherwise
 */
int systick_clk_source_config(uint32_t systick_clk_source)
{
	if (!IS_SYSTICK_CLK_SOURCE(systick_clk_source))
		return -EINVAL;

	if (systick_clk_source == SYSTICK_CLK_SOURCE_HCLK)
		systick_regs->ctrl |= SYSTICK_CLK_SOURCE_HCLK;
	else
		systick_regs->ctrl &= SYSTICK_CLK_SOURCE_HCLK_DIV8;

	return 0;
}

/**
 * @brief Sets SysTick Reload value.
 * @param - reload: SysTick reload new value.
 *          This parameter must be a number between 1 and 0xFFFFFF.
 * @return 0 if no error, not 0 otherwise
 */
int systick_set_reload(uint32_t reload)
{
	systick_regs->load = reload & SYSTICK_VALUE_MASK;

	return 0;
}

/**
 * @brief Enables or disables the SysTick counter.
 * @param - systick_counter: new state of the SysTick counter.
 *          This parameter can be one of the following values:
 *              - SYSTICK_COUNTER_DISABLE: Disable counter
 *              - SYSTICK_COUNTER_ENABLE: Enable counter
 *              - SYSTICK_COUNTER_CLEAR: Clear counter value to 0
 * @return 0 if no error, not 0 otherwise
 */
int systick_counter_cmd(uint32_t systick_counter)
{
	if (!IS_SYSTICK_COUNTER(systick_counter))
		return -EINVAL;

	if (systick_counter == SYSTICK_COUNTER_CLEAR) {
		systick_regs->val = SYSTICK_COUNTER_CLEAR;
	} else {
		if (systick_counter == SYSTICK_COUNTER_ENABLE)
			systick_regs->ctrl |= SYSTICK_COUNTER_ENABLE;
		else
			systick_regs->ctrl &= ~SYSTICK_COUNTER_ENABLE;
	}

	return 0;
}

/**
 * @brief Enables or disables the SysTick Interrupt.
 * @param - enable: new state of the SysTick Interrupt.
 *          This parameter can be: ENABLE or DISABLE.
 */
static void systick_it_config(bool enable)
{
	if (enable)
		systick_regs->ctrl |= CTRL_TICKINT_SET;
	else
		systick_regs->ctrl &= CTRL_TICKINT_RESET;
}

/**
 * @brief Enables the SysTick Interrupt.
 */
void systick_enable_interrupt(void)
{
	systick_it_config(true);
}

/**
 * @brief Disables the SysTick Interrupt.
 */
void systick_disable_interrupt(void)
{
	systick_it_config(false);
}

/**
 * @brief Gets SysTick counter value.
 * @return SysTick current value
 */
uint32_t systick_get_counter(void)
{
	return systick_regs->val;
}

/**
 * systick_get_flag_status
 * Checks whether the specified SysTick flag is set or not.
 * @param systick_flag: specifies the flag to check.
 *                    This parameter can be one of the following values:
 *                       - SysTick_FLAG_COUNT
 *                       - SysTick_FLAG_SKEW
 *                       - SysTick_FLAG_NOREF
 * @return bit status, true if the flag is set, false if reset
 */
bool systick_get_flag_status(uint8_t systick_flag)
{
	uint32_t tmp = 0;
	uint32_t statusreg = 0;

	if (!IS_SYSTICK_FLAG(systick_flag))
		return false;

	/* Determine wether the flag to check is in CTRL or CALIB register */
	if ((systick_flag >> 5) == 1)
		statusreg = systick_regs->ctrl;
	else
		statusreg = systick_regs->calib;

	/* Get the flag position */
	tmp = systick_flag & FLAG_MASK;

	return (statusreg & (BIT(tmp)) >> tmp);
}
