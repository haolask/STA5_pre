#include "sta_map.h"
#include "sta_type.h"
/**
 * @brief chip mode enum
 */
enum chip_mode {
	INPUT_CAPTURE,
	OUTPUT_COMPARE,
	FORCED_COMPARE,
	ONE_PULSE,
	PWM,
	PWM_INPUT
};

/**
 * @struct pwm_state
 * @brief represents a pwm chip state
 * @period duration time of one pwm cycle 
 * @duty_cycle portion of 'on' time
 * @polarity polarity can be NORMAL (0) or INVERSE (1)
 * @enabled chip is active or not
 */
struct pwm_state {
	uint32_t period;
	uint32_t duty_cycle;
	bool polarity;
	bool enabled;
};

/**
 * @struct eft_chip
 * @brief represents the extended function timer chip
 * @enabled actual enable state
 * @period actual period
 * @duty_cycle actual duty cycle
 * @eft	pointer to the eft register
 */
struct eft_chip {
	bool enabled;
	uint32_t period;
	uint32_t duty_cycle;
	enum chip_mode mode;
	t_eft *eft;
};

/**
 * @brief	eft pwm 
 * @param	pwm chip to apply new state to
 * @param	new state which will be applyed
 * @return 	0 if no error, -1 if new state is incoherent
 */
int eft_pwm_apply(struct eft_chip *chip, struct pwm_state *state);

/**
 * @brief	free eft chip
 * @param	eft chip
 */
void eft_delete(struct eft_chip *chip);

/**
 * @brief	init a new eft chip in a specific mode
 * @param	eft register structure
 * @param	chip mode
 */
struct eft_chip *eft_init(t_eft *eft, enum chip_mode mode);
