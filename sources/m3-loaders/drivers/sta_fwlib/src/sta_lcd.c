/**
 * @file sta_lcd.c
 * @brief This file provides all the LCD functions
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include <errno.h>

#include "string.h"
#include "trace.h"
#include "utils.h"

#include "sta_common.h"
#include "sta_adc.h"
#include "sta_gpio.h"
#include "sta_lcd.h"
#include "sta_qos.h"
#include "sta_ltdc.h"
#include "sta_pinmux.h"
#include "sta_eft.h"

#define DEFAULT_OCBR_VALUE 0x1000
#define DEFAULT_OCAR_VALUE 0x0800

/* Maximum Voltage (mV) of DCO ADC */
#define MAX_ADC_VOLTAGE_VALUE 3300
/* Maximum read value linked to maximum Voltage */
#define MAX_ADC_HEXA_VALUE 0x3ff

#define CLKDIV_CLCD_PLL2_FVCOBY2_BY				0
#define CLKDIV_CLCD_PIXCLK					1
#define CLKDIV_CLCD_PIXCLK_PLL2_FVCOBY2_DIV_1			1
#define CLKDIV_CLCD_PIXCLK_PLL2_FVCOBY2_DIV_3			3

/**
 * @brief	Helper to detect and select display configuration
 * @return	0 if no error, not 0 otherwise
 */
int lcd_select_display_cfg(void)
{
	uint32_t data;
	uint32_t data_mV;
	int err;

	/* display detection available on A5_EVB from RevC version */
	if ((get_board_id() == BOARD_A5_EVB) && (get_board_rev_id() >= 2)) {
		/* Read the DCO ADC value if present */
		err = sta_adc_read_data(dco_adc_regs, 0, &data);
		if (err) {
			TRACE_ERR("Failed to read from ADC !\n");
			set_display_cfg(NO_DETECTION);
			return -1;
		}

		data_mV = data * MAX_ADC_VOLTAGE_VALUE / MAX_ADC_HEXA_VALUE;
		/* For cluster display, voltage between 0 and 0.199V */
		if (data_mV < 200)
			set_display_cfg(SINGLE_CLUSTER);
		/* For cluster HD display, voltage between 0.2V and 0.299V */
		else if ((data_mV >= 200) && (data_mV < 300))
			set_display_cfg(SINGLE_CLUSTER_HD);
		/* For Hybrid Cluster WVGA display, voltage between 0.3V and 0.599V */
		else if ((data_mV >= 300) && (data_mV < 600))
			set_display_cfg(SINGLE_HYBRID_CLUSTER_WVGA);
		/* For Hybrid Cluster 720p display, voltage between 0.6V and 0.799V */
		else if ((data_mV >= 600) && (data_mV < 800))
			set_display_cfg(SINGLE_HYBRID_CLUSTER_720P);
		/* For 720P 7" display, voltage between 0.8V and 1.199V */
		else if ((data_mV >= 800) && (data_mV < 1200))
			set_display_cfg(SINGLE_720P);
		/* For 720P 10.1" display, voltage between 1.2V and 1.399V */
		else if ((data_mV >= 1200) && (data_mV < 1400))
			set_display_cfg(SINGLE_720P_10INCHES);
		/* For dual WVGA display, voltage between 2.1V and 2.399V */
		else if ((data_mV >= 2100) && (data_mV < 2400))
			set_display_cfg(DUAL_WVGA);
		/* For single WVGA display, voltage > 3.1V */
		else if (data_mV >= 3100)
			set_display_cfg(SINGLE_WVGA);
		else {
			TRACE_ERR("Failed to select Display from ADC value\n");
			set_display_cfg(NO_DETECTION);
			return -1;
		}
	} else {
		set_display_cfg(NO_DETECTION);
	}
	trace_printf("Display config: %d\n", get_display_cfg());

	return 0;
}

/**
 * @brief	Helper to initialize misc and src display registers
 * @return	0 if no error, not 0 otherwise
 */
int lcd_init_misc_src_reg(void)
{
	uint32_t div_value, pll2_fvcoby2_div_value;

	/* Set display misc A7 registers */

	/* unmask clcd interrupts */
	misc_a7_regs->clcd_int_en.bit.irq_scanline_o = 1;
	misc_a7_regs->clcd_int_en.bit.irq_fifo_underrun_o = 1;
	misc_a7_regs->clcd_int_en.bit.irq_bus_error_o = 1;
	misc_a7_regs->clcd_int_en.bit.irq_reg_reload_o = 1;

	if (get_display_cfg() == DUAL_WVGA) {
		misc_a7_regs->misc_reg4.bit.reg_pol_even = 1;
		misc_a7_regs->misc_reg4.bit.reg_pol_odd = 1;
		misc_a7_regs->misc_reg4.bit.clk_sel = 0; /* half clock */
	} else {
		misc_a7_regs->misc_reg4.bit.clk_sel = 1;
	}

	/* Set display M3 src registers */
	/*
	 * clcd case: programmable divider selected
	 * FVCOBY2 div1 (=614.4) or div3 (=208.4) / (div_value + 1)
	 */
	src_m3_regs->clkdivcr.bit.clcd = CLKDIV_CLCD_PLL2_FVCOBY2_BY;

	if (get_cut_rev() < CUT_30)
		pll2_fvcoby2_div_value = CLKDIV_CLCD_PIXCLK_PLL2_FVCOBY2_DIV_3;
	else
		pll2_fvcoby2_div_value = CLKDIV_CLCD_PIXCLK_PLL2_FVCOBY2_DIV_1;

	switch (get_display_cfg()) {
	case SINGLE_WVGA:
	case SINGLE_HYBRID_CLUSTER_WVGA:
		div_value = (18 / pll2_fvcoby2_div_value) - 1; /* 34.1Mhz */
		break;
	case DUAL_WVGA:
	case SINGLE_HYBRID_CLUSTER_720P:
	case SINGLE_720P_10INCHES:
	case SINGLE_720P:
		/* Avoid divider by 3 with cut1 */
		if (get_cut_rev() == CUT_10)
			div_value = (12 / pll2_fvcoby2_div_value) - 1; /* 51.2MHz */
		else
			div_value = (9 / pll2_fvcoby2_div_value) - 1; /* 68.2Mhz */
		break;
	case SINGLE_CLUSTER:
		div_value = (12 / pll2_fvcoby2_div_value) - 1; /* 51.2Mhz */
		break;
	case SINGLE_CLUSTER_HD:
		div_value = (7 / pll2_fvcoby2_div_value) - 1; /* 88Mhz (102Mhz < cut3.0) */
		break;
	case NO_DETECTION:
	default:
		/* can be changed according to specific board panel */
		div_value = (18 / pll2_fvcoby2_div_value) - 1; /* 34.1Mhz */
		break;
	}

	src_m3_regs->clkdivcr.bit.clcd_div_value = div_value;

	return 0;
}

/* rocktech rk070er9427-ct timings */
const struct videomode single_wvga_timings = {34134, 800, 237, 26, 20, 480, 22,
	13, 10, 0};

/* ampire am-800480aztmqw-54h timings */
const struct videomode single_hybrid_wvga_timings = {34134, 800, 40, 196, 48,
	480, 32, 10, 3, 0};

/* ampire am-1280800p2tzqw-t01h timings */
const struct videomode single_720p_timings = {68300, 1280, 64, 4, 1, 800, 40,
	1, 1, 0};

/* chunghwa claa103wa02xn display timings */
const struct videomode single_cluster_timings = {51210, 1280, 200, 50, 50, 480,
	25, 25, 10, 0};

/* chunghwa claa123fca1xn display timings */
const struct videomode single_cluster_hd_timings = {87780, 1920, 32, 16, 16,
	720, 12, 3, 2, 0};

/* Default timings : can be changed according to specific board panel */
const struct videomode single_default_timings = {34134, 800, 237, 26, 20, 480,
	22, 13, 10, 0};

/**
 * @brief	Initialize LCD controller
 * @return	0 if no error, not 0 otherwise
 */
int lcd_init(void)
{
	struct videomode lcd_video_mode;
	const struct videomode *new_mode;
	int ret = -1;
	bool dual_display = false;

	struct gpio_config pin;

	if(get_display_cfg() == SINGLE_HYBRID_CLUSTER_720P) {
		/* Hybrid cluster backlight */
		pin.direction   = GPIO_DIR_OUTPUT;
		pin.mode        = GPIO_MODE_SOFTWARE;
		pin.level       = GPIO_LEVEL_LEAVE_UNCHANGED;
		pin.trig        = GPIO_TRIG_LEAVE_UNCHANGED;
		/* Export and reset the M3_GPIO 15 */
		gpio_set_pin_config(M3_GPIO(15), &pin);
		gpio_set_gpio_pin(M3_GPIO(15));

	} else {
		/* TODO configure LCD PWM, reset GPIOs??? */
		pinmux_request("pwm_mux");

		/* Set PWM default value */
		struct eft_chip *chip = eft_init(eft3_regs, PWM);
		struct pwm_state state = {
			.period = DEFAULT_OCBR_VALUE,
			.duty_cycle = 0, //set the duty cycle to 50%
			.polarity = 0,
			.enabled = 1,
		};
		ret = eft_pwm_apply(chip, &state);
		if (ret)
			return ret;
		eft_delete(chip);

	}

	/* configure CLCD here... */
	ret = ltdc_init();
	if (ret)
		return ret;

	/* define videomode info (characteristics of the LCD/HDMI output) */
	switch (get_display_cfg()) {
	case DUAL_WVGA:
		dual_display = true;
		/* Do not break, to initialize video mode */
	case SINGLE_WVGA:
		new_mode = &single_wvga_timings;
		break;
	case SINGLE_HYBRID_CLUSTER_720P:
	case SINGLE_720P_10INCHES:
	case SINGLE_720P:
		new_mode = &single_720p_timings;
		break;
	case SINGLE_CLUSTER:
		new_mode = &single_cluster_timings;
		break;
	case SINGLE_CLUSTER_HD:
		new_mode = &single_cluster_hd_timings;
		break;
	case SINGLE_HYBRID_CLUSTER_WVGA:
		new_mode = &single_hybrid_wvga_timings;
		break;
	case NO_DETECTION:
	default:
		new_mode = &single_default_timings;
		break;
	}
	memcpy(&lcd_video_mode, new_mode, sizeof(struct videomode));

	ret = ltdc_mode_set(&lcd_video_mode, dual_display);

	return ret;
}

void lcd_get_display_resolution(uint32_t *width, uint32_t *height)
{
	const struct videomode *video_mode;

	switch (get_display_cfg()) {
	case DUAL_WVGA:
	case SINGLE_WVGA:
		video_mode = &single_wvga_timings;
		break;
	case SINGLE_HYBRID_CLUSTER_720P:
	case SINGLE_720P_10INCHES:
	case SINGLE_720P:
		video_mode = &single_720p_timings;
		break;
	case SINGLE_CLUSTER:
		video_mode = &single_cluster_timings;
		break;
	case SINGLE_CLUSTER_HD:
		video_mode = &single_cluster_hd_timings;
		break;
	case SINGLE_HYBRID_CLUSTER_WVGA:
		video_mode = &single_hybrid_wvga_timings;
		break;
	case NO_DETECTION:
	default:
		video_mode = &single_default_timings;
		break;
	}
	*width = video_mode->hactive;
	*height = video_mode->vactive;
}
