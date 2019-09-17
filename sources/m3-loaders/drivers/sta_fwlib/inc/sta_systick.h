/**
 * @file sta_systick.h
 * @brief Provide Systick firmware API
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#ifndef __STA_SYSTICK_H__
#define __STA_SYSTICK_H__

#include "sta_map.h"

#define SYSTICK_CLK_SOURCE_HCLK_DIV8    0xFFFFFFFB
#define SYSTICK_CLK_SOURCE_HCLK         0x00000004

/* SysTick counter state */
#define SYSTICK_COUNTER_DISABLE        0xFFFFFFFE
#define SYSTICK_COUNTER_ENABLE         0x00000001
#define SYSTICK_COUNTER_CLEAR          0x00000000

/* SysTick Flag */
#define SYSTICK_FLAG_COUNT             0x30
#define SYSTICK_FLAG_SKEW              0x5E
#define SYSTICK_FLAG_NOREF             0x5F

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
int systick_clk_source_config(uint32_t systick_clk_source);

/**
 * @brief Sets SysTick Reload value.
 * @param - reload: SysTick reload new value.
 *          This parameter must be a number between 1 and 0xFFFFFF.
 * @return 0 if no error, not 0 otherwise
 */
int systick_set_reload(uint32_t reload);

/**
 * @brief Enables or disables the SysTick counter.
 * @param - systick_counter: new state of the SysTick counter.
 *          This parameter can be one of the following values:
 *              - SYSTICK_COUNTER_DISABLE: Disable counter
 *              - SYSTICK_COUNTER_ENABLE: Enable counter
 *              - SYSTICK_COUNTER_CLEAR: Clear counter value to 0
 * @return 0 if no error, not 0 otherwise
 */
int systick_counter_cmd(uint32_t systick_counter);

/**
 * @brief Enables the SysTick Interrupt.
 */
void systick_enable_interrupt(void);

/**
 * @brief Disables the SysTick Interrupt.
 */
void systick_disable_interrupt(void);

/**
 * @brief Gets SysTick counter value.
 * @return SysTick current value
 */
uint32_t systick_get_counter(void);

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
bool systick_get_flag_status(uint8_t systick_flag);

#endif /* __STA_SYSTICK_H__ */
