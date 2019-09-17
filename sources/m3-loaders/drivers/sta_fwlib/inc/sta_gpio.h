/**
 * @file sta_gpio.h
 * @brief This file provides all the GPIO firmware definitions.
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#ifndef _STA_GPIO_H_
#define _STA_GPIO_H_

#include "sta_map.h"

/* Registers offsets */
#define GPIO_DAT		0x00
#define GPIO_DATS		0x04
#define GPIO_DATC		0x08
#define GPIO_PDIS		0x0C
#define GPIO_DIR		0x10
#define GPIO_DIRS		0x14
#define GPIO_DIRC		0x18
#define GPIO_SLPM		0x1C
#define GPIO_AFSLA		0x20
#define GPIO_AFSLB		0x24
#define GPIO_RIMSC		0x40
#define GPIO_FIMSC		0x44
#define GPIO_IS			0x48
#define GPIO_IC			0x4C

/* GPIO modes and alternate functions */
#define GPIO_MODE_LEAVE_UNCHANGED	0
#define GPIO_MODE_SOFTWARE			1
#define GPIO_MODE_ALT_FUNCTION_A	2
#define GPIO_MODE_ALT_FUNCTION_B	3
#define GPIO_MODE_ALT_FUNCTION_C	4

/* GPIO directions */
#define GPIO_DIR_LEAVE_UNCHANGED	0
#define GPIO_DIR_INPUT				1
#define GPIO_DIR_OUTPUT				2

/* GPIO Level trigger */
#define GPIO_LEVEL_LEAVE_UNCHANGED	0
#define GPIO_LEVEL_PULLDISABLE		1
#define GPIO_LEVEL_PULLUP			2
#define GPIO_LEVEL_PULLDOWN			3
#define GPIO_LEVEL_LOW				4
#define GPIO_LEVEL_HIGH				5
#define GPIO_HIGH_IMPEDENCE			6

/* GPIO trigger mode */
#define GPIO_TRIG_LEAVE_UNCHANGED	0
#define GPIO_TRIG_DISABLE			1
#define GPIO_TRIG_RISING_EDGE		2
#define GPIO_TRIG_FALLING_EDGE		3
#define GPIO_TRIG_BOTH_EDGES		4
#define GPIO_TRIG_HIGH_LEVEL		5
#define GPIO_TRIG_LOW_LEVEL			6
#define GPIO_MASK_RISING		7
#define GPIO_MASK_FALLING		8
#define GPIO_MASK_BOTH			9

/* GPIO data state */
#define GPIO_DATA_LOW				0
#define GPIO_DATA_HIGH				1

enum rgpio_mode {
	GPIO,
	AFSLA,
	AFSLB,
	AFSLC,
};

/**
 * @struct gpio_mux
 * @brief describes a muxing configurationg for a single gpio or a group of
 * gpios.
 * @start first gpio index
 * @end last gpio index (= start for 1 gpio)
 * @mux_type the mux configuration (@see GPIO_MODE_*)
 */
struct gpio_mux {
	uint8_t start;
	uint8_t end;
	uint8_t mux_type;
};

/**
 * @struct gpio_config
 * @brief represents the configuration for a given gpio pin
 * @mode: GPIO mode (software) or alternate function (A, B or C)
 * @direction: input or output
 * @trig: trigger mode (rising or falling edge, high or low, etc.)
 * @level: level mode (pull up disable, pull up/down, high, low, etc.)
 */
struct gpio_config {
	int mode;
	int direction;
	int trig;
	int level;
};

/**
 * @brief	GPIO M3 main interrupt service routine
 */
int gpio_request_irq(unsigned int abs_pin_no, int (*handler) (void));

/**
 * @brief	GPIO M3 main interrupt service routine
 */
void gpio_irq_handler(void);
void GPIO_S_IRQHandler(void);

/**
 * @brief	Returns where the GPIO is located
 * @param	absolute pin number
 * @param   pointer to relative pin number
 * @param   pointer to relative pin mask
 * @param   pointer to GPIo bank
 * @return	0 if no error, not 0 otherwise
 */
int gpio_get_coordinates(unsigned int abs_pin_no, uint32_t *rel_pin_no,
			uint32_t *gpio_pin_mask, uint32_t *addr);

/**
 * @brief	Reset the GPIO passed as parameter
 * @param	absolute pin number
 * @return	0 if no error, not 0 otherwise
 */
int gpio_reset_gpio_pin(unsigned int abs_pin_no);

/**
 * @brief	Read the configuration of the pin passed as parameter
 * @param	absolute pin number
 * @param	GPIO configuration structure
 * @return	0 if no error, not 0 otherwise
 */
int gpio_get_pin_dir(unsigned int abs_pin_no);

/**
 * @brief	Read the mode (gpio, afslA/B/C) of a pin
 * @param	absolute pin number to read
 * @return	gpio: 0, afslA: 1, afslB: 2, afslC: 3
 */
enum rgpio_mode gpio_get_mode(unsigned int abs_pin_no);

/**
 * @brief	Read the trigger of a pin
 * @param	absolute pin number to read
 * @return	Rising_edge: 2, falling_edge: 3, both: 4, disable: 1
 */
int gpio_get_trig(unsigned int abs_pin_no);

/**
 * @brief	Read level of a pin
 * @param	absolute pin number to read
 * @return	Pullup:2, Pulldown: 3, disable: 1
 */
int gpio_get_level(unsigned int abs_pin_no);

/**
 * @brief acknowledge irq, clear irq of a pin
 * @param absolute pin to clean
 */
void gpio_irq_ack(unsigned int abs_pin_no);

/**
 * @brief	Apply a configuration to the pin passed as parameter
 * @param	absolute pin number
 * @param	GPIO configuration structure
 * @return	0 if no error, not 0 otherwise
 */
int gpio_set_pin_config(unsigned int abs_pin_no, struct gpio_config *config);

/**
 * @brief	Read the status of a GPIO
 * @param	absolute pin number
 * @param	GPIO value
 * @return	0 if no error, not 0 otherwise
 */
int gpio_read_gpio_pin(unsigned int abs_pin_no, int *value);

/**
 * @brief	Set the status of a GPIO
 * @param	absolute pin number
 * @return	0 if no error, not 0 otherwise
 */
int gpio_set_gpio_pin(unsigned int abs_pin_no);

/**
 * @brief	Clear the status of a GPIO
 * @param	absolute pin number
 * @return	0 if no error, not 0 otherwise
 */
int gpio_clear_gpio_pin(unsigned int abs_pin_no);

/**
 * @brief	Request gpio pin muxing configuration
 * @param	muxing configuration
 * @param	nelems: number of elements
 * @return	0 if no error, not 0 otherwise
 */
int gpio_request_mux(const struct gpio_mux *p_mux, uint32_t nelems);

#endif
