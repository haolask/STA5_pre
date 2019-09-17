/**
 * @file sta_gpio.c
 * @brief This file provides all the UART firmware functions.
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include <string.h>
#include <errno.h>

#include "utils.h"

#include "sta_gpio.h"
#include "sta_common.h"
#include "trace.h"
#include "sta_pinmux.h"

#define GPIO_NUMBER_PER_BANK 32

#define M3_GPIO_PIN_OFFSET_STA1385_CUT1			8
#define AO_GPIO_PIN_OFFSET_STA1385_CUT2			16
#define AO_GPIO_PIN_OFFSET_AFSLAB_STA1385_CUT1	26

enum gpio_banks {
	M3_GPIO_BANK,
	S_GPIO_BANK,
	NUM_GPIO_BANKS,
};

struct gpio_irqs_desc {
	int (*handlers[GPIO_NUMBER_PER_BANK]) (void);
	uint32_t status;
};

static struct gpio_irqs_desc gpio_irqs[NUM_GPIO_BANKS];

/**
 * @brief	GPIO request and register interrupt service routine
 *		only M3 and shared GPIO are possible
 */
int gpio_request_irq(unsigned int abs_pin_no, int (*handler) (void))
{
	uint32_t rel_pin_no;
	uint32_t gpio_pin_mask;
	static bool initialized = 0;
	uint32_t addr;
	uint32_t bank;

	if (!initialized) {
		memset(&gpio_irqs, 0, sizeof(struct gpio_irqs_desc));
		initialized = 1;
	}

	if (!handler)
		return -EINVAL;

	if (abs_pin_no <= A7_GPIO_PIN_MAX)
		return -EINVAL;

	if (gpio_get_coordinates(abs_pin_no, &rel_pin_no,
				 &gpio_pin_mask, &addr))
		return -EINVAL;

	if (addr == S_GPIO0_BASE)
		bank = S_GPIO_BANK;
	else
		bank = M3_GPIO_BANK;

	if (gpio_pin_mask & gpio_irqs[bank].status)
		return -EBUSY;

	gpio_irqs[bank].status |= gpio_pin_mask;
	gpio_irqs[bank].handlers[rel_pin_no] = handler;

	TRACE_INFO("%s: gpio %d\n", __func__, rel_pin_no);
	return 0;
}

/**
 * @brief	GPIO M3 main interrupt service routine
 */
void gpio_irq_handler(void)
{
	int i;
	uint32_t is = read_reg(M3_GPIO0_BASE + GPIO_IS);

	for (i = 0; i < GPIO_NUMBER_PER_BANK; i++) {
		/* Only clear interrupt status bit that match a registered user
		 * in uCSS side.
		 * Let keep other bit un-cleared assuming driver running in the
		 * application (Linux) get the ownership so will clear it later.
		 */
		if ((BIT(i) & is) && (BIT(i) & gpio_irqs[M3_GPIO_BANK].status)) {
			set_bit_reg(BIT(i), M3_GPIO0_BASE + GPIO_IC);
			gpio_irqs[M3_GPIO_BANK].handlers[i] ();
		}
	}
}

/**
 * @brief	S_GPIO main interrupt service routine
 */
void GPIO_S_IRQHandler(void)
{
	int i;
	uint32_t is = read_reg(S_GPIO0_BASE + GPIO_IS);

	for (i = 0; i < GPIO_NUMBER_PER_BANK; i++) {
		/* Only clear interrupt status bit that match a registered user
		 * in uCSS side.
		 * Let keep other bit un-cleared assuming driver running in the
		 * application (Linux) get the ownership so will clear it later.
		 */
		if ((BIT(i) & is) && (BIT(i) & gpio_irqs[S_GPIO_BANK].status)) {
			set_bit_reg(BIT(i), S_GPIO0_BASE + GPIO_IC);
			gpio_irqs[S_GPIO_BANK].handlers[i] ();
		}
	}
}

/**
 * @brief	Returns where the GPIO is located
 * @param	absolute pin number
 * Output	pointer to relative pin number
 * Output	pointer to relative pin mask
 * Output	pointer to GPIo bank
 * @return	0 if no error, not 0 otherwise
 */
int gpio_get_coordinates(unsigned int abs_pin_no, uint32_t *rel_pin_no,
			 uint32_t *gpio_pin_mask, uint32_t *addr)
{
	if (abs_pin_no <= A7_GPIO_PIN_MAX) {
		*rel_pin_no = (uint32_t)(abs_pin_no % GPIO_NUMBER_PER_BANK);

		/**
		 * Calculate the controller offset,
		 * in case the GPIO belongs to A7 domain
		 */
		*addr = (uint32_t)(((uint8_t *)A7_GPIO0_BASE) +
				    (0x1000 * (abs_pin_no /
					       GPIO_NUMBER_PER_BANK)));

	} else if (abs_pin_no >= M3_GPIO_PIN_OFFSET &&
		   abs_pin_no <= M3_GPIO_PIN_MAX) {
		*rel_pin_no = (uint32_t)(abs_pin_no - M3_GPIO_PIN_OFFSET);

		if ((get_soc_id() == SOCID_STA1385) &&
		    (get_cut_rev() == CUT_10)) {
			/* cut1.0 specific */
			*rel_pin_no += M3_GPIO_PIN_OFFSET_STA1385_CUT1;
		}

		/* At contrary, we have only one bank for M3 GPIOs */
		*addr = (uint32_t)M3_GPIO0_BASE;

	} else if (abs_pin_no >= S_GPIO_PIN_OFFSET &&
		   abs_pin_no <= S_GPIO_PIN_MAX) {
		*rel_pin_no = (uint32_t)(abs_pin_no - S_GPIO_PIN_OFFSET);

		/* We have only one bank for Shared GPIOs */
		*addr = (uint32_t)S_GPIO0_BASE;

	} else if (abs_pin_no >= AO_GPIO_PIN_OFFSET &&
		   abs_pin_no <= AO_GPIO_PIN_MAX) {

		*rel_pin_no = (uint32_t)(abs_pin_no - AO_GPIO_PIN_OFFSET);

		if ((get_soc_id() == SOCID_STA1385) &&
		    (get_cut_rev() >= CUT_20)) {
			/* cut2.0 and upper */
			*rel_pin_no += AO_GPIO_PIN_OFFSET_STA1385_CUT2;
		}

		/* AO_GPIOs are defined within M3 GPIOs bank */
		*addr = (uint32_t)M3_GPIO0_BASE;
	} else {
		return -EINVAL;
	}

	*gpio_pin_mask = (uint32_t)(1 << *rel_pin_no);

	return 0;
}

/**
 * @brief	Reset the GPIO passed as parameter
 * @param	absolute pin number
 * @return	0 if no error, not 0 otherwise
 */
int gpio_reset_gpio_pin(unsigned int abs_pin_no)
{
	int err;
	uint32_t rel_pin_no;
	uint32_t gpio_pin_mask;
	uint32_t addr;
	uint32_t reg;

	err =
	    gpio_get_coordinates(abs_pin_no, &rel_pin_no, &gpio_pin_mask, &addr);
	if (err)
		return err;

	/* Reset basic registers */
	reg = read_reg(addr + GPIO_IC);
	write_reg(reg | gpio_pin_mask, addr + GPIO_IC);

	reg = read_reg(addr + GPIO_RIMSC);
	write_reg(reg & ~gpio_pin_mask, addr + GPIO_RIMSC);

	reg = read_reg(addr + GPIO_FIMSC);
	write_reg(reg & ~gpio_pin_mask, addr + GPIO_FIMSC);

	write_reg(gpio_pin_mask, addr + GPIO_DIRS);
	write_reg(gpio_pin_mask, addr + GPIO_DATC);
	write_reg(gpio_pin_mask, addr + GPIO_DIRC);

	reg = read_reg(addr + GPIO_PDIS);
	write_reg(reg & ~gpio_pin_mask, addr + GPIO_PDIS);

	reg = read_reg(addr + GPIO_SLPM);
	write_reg(reg & ~gpio_pin_mask, addr + GPIO_SLPM);

	reg = read_reg(addr + GPIO_IC);
	write_reg(reg & ~gpio_pin_mask, addr + GPIO_IC);

	if ((get_soc_id() == SOCID_STA1385) && (get_cut_rev() == CUT_10)) {
		/* need a specific management for M3_GPIO and AO_GPIO on STA1385 cut1.0 */
		uint32_t ao_gpio_rel_pin_no = rel_pin_no;
		uint32_t ao_gpio_pin_mask = gpio_pin_mask;

		if ((abs_pin_no >= AO_GPIO_PIN_OFFSET) && (abs_pin_no <= AO_GPIO_PIN_MAX)) {
			/* AO_GPIO are shifted by 26 on cut1.0 for AFSLA/B registers */
			ao_gpio_rel_pin_no = rel_pin_no + AO_GPIO_PIN_OFFSET_AFSLAB_STA1385_CUT1;
			ao_gpio_pin_mask = (1 << ao_gpio_rel_pin_no);
		}

		gpio_pin_mask = ao_gpio_pin_mask;
	}

	reg = read_reg(addr + GPIO_AFSLA);
	write_reg(reg & ~gpio_pin_mask, addr + GPIO_AFSLA);

	reg = read_reg(addr + GPIO_AFSLB);
	write_reg(reg & ~gpio_pin_mask, addr + GPIO_AFSLB);

	return 0;
}

/**
 * @brief acknowledge irq, clear irq of a pin
 * @param absolute pin to clean
 */
void gpio_irq_ack(unsigned int abs_pin_no)
{
	int err;
	uint32_t rel_pin_no;
	uint32_t gpio_pin_mask;
	uint32_t addr;
	uint32_t reg;

	err =
	    gpio_get_coordinates(abs_pin_no, &rel_pin_no, &gpio_pin_mask,
				 &addr);

	if (!err) {
		reg = read_reg(addr + GPIO_IC);
		write_reg(reg & ~gpio_pin_mask, addr + GPIO_IC);
	}
}

/**
 * @brief	Read the configuration of the pin passed as parameter
 * @param	absolute pin number
 * @param	GPIO configuration structure
 * @return	0 if no error, not 0 otherwise
 */
int gpio_get_pin_dir(unsigned int abs_pin_no)
{
	int err;
	uint32_t rel_pin_no;
	uint32_t gpio_pin_mask;
	uint32_t addr;
	int directions;

	err = gpio_get_coordinates(abs_pin_no, &rel_pin_no, &gpio_pin_mask,
				   &addr);
	if (err)
		return -EINVAL;

	directions = read_reg(addr + GPIO_DIR);

	return ((directions & (1 << rel_pin_no)) >> rel_pin_no);
}

/**
 * @brief	Read the mode (gpio, afslA/B/C) of a pin
 * @param	absolute pin number to read
 * @return	gpio: 0, afslA: 1, afslB: 2, afslC: 3
 */
enum rgpio_mode gpio_get_mode(unsigned int abs_pin_no)
{
	int err;
	uint32_t rel_pin_no;
	uint32_t gpio_pin_mask;
	uint32_t addr;
	int afsla;
	int afslb;

	err = gpio_get_coordinates(abs_pin_no, &rel_pin_no, &gpio_pin_mask,
				   &addr);
	if (err)
		return -EINVAL;

	afsla = read_reg(addr + GPIO_AFSLA);
	afslb = read_reg(addr + GPIO_AFSLB);

	return afsla & gpio_pin_mask ? AFSLA :
		afslb & gpio_pin_mask ? AFSLB :
		GPIO;
}

/**
 * @brief	Read the trigger of a pin
 * @param	absolute pin number to read
 * @return	Rising_edge: 2, falling_edge: 3, both: 4, disable: 1
 */
int gpio_get_trig(unsigned int abs_pin_no)
{
	int err, ret;
	uint32_t rel_pin_no, gpio_pin_mask, addr, rimsc, fimsc;

	err = gpio_get_coordinates(abs_pin_no, &rel_pin_no, &gpio_pin_mask,
				   &addr);
	if (err)
		return -EINVAL;

	rimsc = read_reg(addr + GPIO_RIMSC);
	fimsc = read_reg(addr + GPIO_FIMSC);
	ret = rimsc & gpio_pin_mask ?
		(fimsc & gpio_pin_mask ?
		  GPIO_TRIG_BOTH_EDGES : GPIO_TRIG_RISING_EDGE) :
		(fimsc & gpio_pin_mask ?
		  GPIO_TRIG_FALLING_EDGE : GPIO_TRIG_DISABLE);

	return ret;
}

/**
 * @brief	Read level of a pin
 * @param	absolute pin number to read
 * @return	Pullup:2, Pulldown: 3, disable: 1
 */
int gpio_get_level(unsigned int abs_pin_no)
{
	int err;
	uint32_t rel_pin_no, gpio_pin_mask, addr;
	uint32_t regdir, regdat, regpdis;
	bool afsla, afslb;

	err = gpio_get_coordinates(abs_pin_no, &rel_pin_no, &gpio_pin_mask,
				   &addr);
	if (err) {
		TRACE_ERR("get level failed to get coordinate\n");
		return -EINVAL;
	}

	regdir = read_reg(addr + GPIO_DIR);
	regpdis = read_reg(addr + GPIO_PDIS);

	if (!(regdir & gpio_pin_mask)) {
		if (!(regpdis & gpio_pin_mask)) {
			regdat = read_reg(addr + GPIO_DAT);

			return regdat & gpio_pin_mask ?
				GPIO_LEVEL_PULLUP : GPIO_LEVEL_PULLDOWN;
		}
	} else {
		afsla = read_reg(addr + GPIO_AFSLA) & gpio_pin_mask;
		afslb = read_reg(addr + GPIO_AFSLB) & gpio_pin_mask;
		if (!afsla && !afslb) {
			regdat = read_reg(addr + GPIO_DAT);
			return regdat & gpio_pin_mask ?
				GPIO_LEVEL_LOW : GPIO_LEVEL_HIGH;
		}
	}

	return GPIO_LEVEL_PULLDISABLE;
}

/**
 * @brief	Apply a configuration to the pin passed as parameter
 * @param	absolute pin number
 * @param	GPIO configuration structure
 * @return	0 if no error, not 0 otherwise
 */
int gpio_set_pin_config(unsigned int abs_pin_no, struct gpio_config *config)
{
	int err;
	uint32_t rel_pin_no;
	uint32_t gpio_pin_mask;
	uint32_t addr;
	uint32_t reg;

	err =
	    gpio_get_coordinates(abs_pin_no, &rel_pin_no, &gpio_pin_mask, &addr);
	if (err)
		return err;

	if (config->mode == GPIO_MODE_SOFTWARE) {
		switch (config->direction) {
		case GPIO_DIR_INPUT:
			write_reg(gpio_pin_mask, addr + GPIO_DIRC);
			break;

		case GPIO_DIR_OUTPUT:
			write_reg(gpio_pin_mask, addr + GPIO_DIRS);
			break;

		case GPIO_DIR_LEAVE_UNCHANGED:
			break;

		default:
			return -EINVAL;
		}

		switch (config->trig) {
		case GPIO_TRIG_DISABLE:
			reg = read_reg(addr + GPIO_RIMSC);
			write_reg(reg & ~gpio_pin_mask, addr + GPIO_RIMSC);
			reg = read_reg(addr + GPIO_FIMSC);
			write_reg(reg & ~gpio_pin_mask, addr + GPIO_FIMSC);
			break;

		case GPIO_TRIG_RISING_EDGE:
			reg = read_reg(addr + GPIO_RIMSC);
			write_reg(reg | gpio_pin_mask, addr + GPIO_RIMSC);
			reg = read_reg(addr + GPIO_FIMSC);
			write_reg(reg & ~gpio_pin_mask, addr + GPIO_FIMSC);
			break;

		case GPIO_TRIG_FALLING_EDGE:
			reg = read_reg(addr + GPIO_RIMSC);
			write_reg(reg & ~gpio_pin_mask, addr + GPIO_RIMSC);
			reg = read_reg(addr + GPIO_FIMSC);
			write_reg(reg | gpio_pin_mask, addr + GPIO_FIMSC);
			break;

		case GPIO_TRIG_BOTH_EDGES:
			reg = read_reg(addr + GPIO_RIMSC);
			write_reg(reg | gpio_pin_mask, addr + GPIO_RIMSC);
			reg = read_reg(addr + GPIO_FIMSC);
			write_reg(reg | gpio_pin_mask, addr + GPIO_FIMSC);
			break;

		case GPIO_TRIG_LEAVE_UNCHANGED:
			break;

		case GPIO_TRIG_HIGH_LEVEL:
		case GPIO_TRIG_LOW_LEVEL:
			return -ENOTSUP;

		case GPIO_MASK_BOTH:
			reg = read_reg(addr + GPIO_RIMSC);
			write_reg(reg & ~gpio_pin_mask, addr + GPIO_RIMSC);
			reg = read_reg(addr + GPIO_FIMSC);
			write_reg(reg & ~gpio_pin_mask, addr + GPIO_FIMSC);
			break;

		case GPIO_MASK_FALLING:
			reg = read_reg(addr + GPIO_FIMSC);
			write_reg(reg & ~gpio_pin_mask, addr + GPIO_FIMSC);
			break;

		case GPIO_MASK_RISING:
			reg = read_reg(addr + GPIO_RIMSC);
			write_reg(reg & ~gpio_pin_mask, addr + GPIO_RIMSC);
			break;

		default:
			return -EINVAL;
		}

		switch (config->level) {
		case GPIO_LEVEL_PULLDISABLE:
			reg = read_reg(addr + GPIO_PDIS);
			write_reg(reg | gpio_pin_mask, addr + GPIO_PDIS);
			break;
		case GPIO_LEVEL_PULLDOWN:
			reg = read_reg(addr + GPIO_DIR);
			if (!(reg & gpio_pin_mask)) {
				reg = read_reg(addr + GPIO_PDIS);
				write_reg(reg & ~gpio_pin_mask,
					  addr + GPIO_PDIS);
				write_reg(gpio_pin_mask, addr + GPIO_DATC);
			}
			break;
		case GPIO_LEVEL_PULLUP:
			reg = read_reg(addr + GPIO_DIR);
			if (!(reg & gpio_pin_mask)) {
				reg = read_reg(addr + GPIO_PDIS);
				write_reg(reg & ~gpio_pin_mask,
					  addr + GPIO_PDIS);
				write_reg(gpio_pin_mask, addr + GPIO_DATS);
			}
			break;
		case GPIO_LEVEL_HIGH:
			reg = read_reg(addr + GPIO_DIR);
			if (reg & gpio_pin_mask)
				write_reg(gpio_pin_mask, addr + GPIO_DATS);
			break;
		case GPIO_LEVEL_LOW:
			reg = read_reg(addr + GPIO_DIR);
			if (reg & gpio_pin_mask)
				write_reg(gpio_pin_mask, addr + GPIO_DATC);
			break;
		case GPIO_TRIG_LEAVE_UNCHANGED:
			break;
		default:
			return -EINVAL;
		}

	}

	if ((get_soc_id() == SOCID_STA1385) && (get_cut_rev() == CUT_10)) {
		/* need a specific management for M3_GPIO and AO_GPIO on STA1385 cut1.0 */
		uint32_t ao_gpio_rel_pin_no = rel_pin_no;
		uint32_t ao_gpio_pin_mask = gpio_pin_mask;

		if ((abs_pin_no >= AO_GPIO_PIN_OFFSET) && (abs_pin_no <= AO_GPIO_PIN_MAX)) {
			/* AO_GPIO are shifted by 26 on cut1.0 for AFSLA/B registers */
			ao_gpio_rel_pin_no = rel_pin_no + AO_GPIO_PIN_OFFSET_AFSLAB_STA1385_CUT1;
			ao_gpio_pin_mask = (1 << ao_gpio_rel_pin_no);
		}

		gpio_pin_mask = ao_gpio_pin_mask;
	}

	switch (config->mode) {
	case GPIO_MODE_SOFTWARE:
		reg = read_reg(addr + GPIO_AFSLA);
		write_reg(reg & ~gpio_pin_mask, addr + GPIO_AFSLA);
		reg = read_reg(addr + GPIO_AFSLB);
		write_reg(reg & ~gpio_pin_mask, addr + GPIO_AFSLB);
		break;

	case GPIO_MODE_ALT_FUNCTION_A:
		reg = read_reg(addr + GPIO_AFSLA);
		write_reg(reg | gpio_pin_mask, addr + GPIO_AFSLA);
		reg = read_reg(addr + GPIO_AFSLB);
		write_reg(reg & ~gpio_pin_mask, addr + GPIO_AFSLB);
		break;

	case GPIO_MODE_ALT_FUNCTION_B:
		reg = read_reg(addr + GPIO_AFSLA);
		write_reg(reg & ~gpio_pin_mask, addr + GPIO_AFSLA);
		reg = read_reg(addr + GPIO_AFSLB);
		write_reg(reg | gpio_pin_mask, addr + GPIO_AFSLB);
		break;

	case GPIO_MODE_ALT_FUNCTION_C:
		reg = read_reg(addr + GPIO_AFSLA);
		write_reg(reg | gpio_pin_mask, addr + GPIO_AFSLA);
		reg = read_reg(addr + GPIO_AFSLB);
		write_reg(reg | gpio_pin_mask, addr + GPIO_AFSLB);
		break;

	case GPIO_MODE_LEAVE_UNCHANGED:
		break;

	default:
		return -EINVAL;
	}
	return 0;
}

/**
 * @brief	Read the status of a GPIO
 * @param	absolute pin number
 * Output         : GPIO value
 * @return	0 if no error, not 0 otherwise
 */
int gpio_read_gpio_pin(unsigned int abs_pin_no, int *value)
{
	int err;
	uint32_t rel_pin_no;
	uint32_t gpio_pin_mask;
	uint32_t addr;
	uint32_t reg;

	if (!value)
		return -EINVAL;

	err = gpio_get_coordinates(abs_pin_no,
				  &rel_pin_no, &gpio_pin_mask, &addr);

	if (err)
		return err;

	reg = read_reg(addr + GPIO_DAT);
	if ((reg & gpio_pin_mask) == gpio_pin_mask) {
		*value = GPIO_DATA_HIGH;
	} else {
		*value = GPIO_DATA_LOW;
	}

	return 0;
}

/**
 * @brief	Set the status of a GPIO
 * @param	absolute pin number
 * @return	0 if no error, not 0 otherwise
 */
int gpio_set_gpio_pin(unsigned int abs_pin_no)
{
	int err;
	uint32_t rel_pin_no;
	uint32_t gpio_pin_mask;
	uint32_t addr;

	err = gpio_get_coordinates(abs_pin_no,
				  &rel_pin_no, &gpio_pin_mask, &addr);

	if (err)
		return err;

	write_reg(gpio_pin_mask, addr + GPIO_DATS);

	return 0;
}

/**
 * @brief	Clear the status of a GPIO
 * @param	absolute pin number
 * @return	0 if no error, not 0 otherwise
 */
int gpio_clear_gpio_pin(unsigned int abs_pin_no)
{
	int err;
	uint32_t rel_pin_no;
	uint32_t gpio_pin_mask;
	uint32_t addr;

	err = gpio_get_coordinates(abs_pin_no,
				  &rel_pin_no, &gpio_pin_mask, &addr);
	if (err)
		return err;

	write_reg(gpio_pin_mask, addr + GPIO_DATC);

	return 0;
}

/**
 * @brief	Request gpio pin muxing configuration
 * @param	muxing configuration
 * @param	number of mux
 * @return	0 if no error, not 0 otherwise
 */
int gpio_request_mux(const struct gpio_mux *p_mux, uint32_t nelems)
{
	int err;
	unsigned int i, gpio;
	struct gpio_config pin;

	if (!p_mux)
		return -EINVAL;

	for (i = 0; i < nelems; i++, p_mux++) {
		for (gpio = p_mux->start; gpio <= p_mux->end; gpio++) {
			pin.mode = p_mux->mux_type;
			if (p_mux->mux_type == GPIO_MODE_SOFTWARE)
				pin.direction = GPIO_DIR_INPUT;
			else
				pin.direction = GPIO_DIR_LEAVE_UNCHANGED;
			pin.level = GPIO_LEVEL_LEAVE_UNCHANGED;
			pin.trig = GPIO_TRIG_LEAVE_UNCHANGED;
			err = gpio_set_pin_config(gpio, &pin);
			if (err)
				return err;
		}
	}

	return 0;
}
