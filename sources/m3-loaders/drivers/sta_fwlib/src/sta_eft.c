#include <stdlib.h>

#include "sta_eft.h"
#include "FreeRTOS.h"
#include "trace.h"

static void update_duty(struct eft_chip *chip, uint32_t duty_cycle)
{
	if (chip->enabled) {
		chip->duty_cycle = duty_cycle;
		chip->eft->eft_ocar = chip->period - duty_cycle;
	}
}

static void update_period(struct eft_chip *chip, uint32_t period)
{
	if (chip->enabled) {
		chip->period = period;
		chip->eft->eft_ocbr = period;
	}
}

static void enable_pwm_chip(struct eft_chip *chip)
{
	chip->eft->eft_cr1.bit.en = 1;
	chip->enabled = 1;
}

static void disable_pwm_chip(struct eft_chip *chip)
{
	chip->eft->eft_cr1.bit.en = 0;
	update_duty(chip, 0);
	update_period(chip, 0);
	chip->enabled = 0;
}

int eft_pwm_apply(struct eft_chip *chip, struct pwm_state *state)
{
	if (!chip || chip->mode != PWM || !state ||
	    (!state->period  && !chip->period) ||
	    (state->duty_cycle > state->period ||
	     state->duty_cycle > chip->period))
		return -1;

	if (state->enabled) {
		enable_pwm_chip(chip);
	} else {
		disable_pwm_chip(chip);
		return 0;
	}

	if (chip->period != state->period)
		update_period(chip, state->period);

	if (chip->duty_cycle != state->duty_cycle)
		update_duty(chip, state->duty_cycle);

	return 0;
}

static void eft_set_pwm_mode(struct eft_chip *chip)
{
	chip->eft->eft_cr1.reg = 0x8150;
	chip->enabled = true;
	chip->mode = PWM;
}

static void eft_set_one_pulse_mode(struct eft_chip *chip, uint32_t pulse_length)
{
	chip->eft->eft_cr1.reg = 0x0364;
	chip->eft->eft_cr2.reg = 0;
	chip->eft->eft_cr2.bit.cc = 99;
}

void eft_delete(struct eft_chip *chip)
{
	vPortFree(chip);
}

struct eft_chip *eft_init(t_eft *eft, enum chip_mode mode)
{
	struct eft_chip *chip = pvPortMalloc(sizeof(struct eft_chip));

	chip->eft     = eft;
	if (mode == PWM)
		eft_set_pwm_mode(chip);
	if (mode == ONE_PULSE)
		eft_set_one_pulse_mode(chip, 0);

	return chip;
}
