/**
 * Copyright (C) 2015 ST Microelectronics
 *
 * Jean-Nicolas Graux <jean-nicolas.graux@st.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 *
 * 2016-02-25 11:44:42: Automatic generation with pinout2pinctrl.py tool.
 */

#include <linux/kernel.h>
#include <linux/pinctrl/pinctrl.h>
#include "pinctrl-nomadik.h"

/* All the pins that can be used for GPIO and some other functions */
#define _GPIO(offset)		(offset)
#define S_GPIO(offset)		(offset + 128)
#define M3_GPIO(offset)		(offset + 160)

/* main gpios */

#define STA1295_PIN_B21		_GPIO(0)
#define STA1295_PIN_B22		_GPIO(1)
#define STA1295_PIN_D19		_GPIO(2)
#define STA1295_PIN_A22		_GPIO(3)
#define STA1295_PIN_B19		_GPIO(4)
#define STA1295_PIN_B20		_GPIO(5)
#define STA1295_PIN_D14		_GPIO(6)
#define STA1295_PIN_T23		_GPIO(7)
#define STA1295_PIN_B23		_GPIO(8)
#define STA1295_PIN_A18		_GPIO(9)
#define STA1295_PIN_A20		_GPIO(10)
#define STA1295_PIN_A19		_GPIO(11)
#define STA1295_PIN_A21		_GPIO(12)
#define STA1295_PIN_C18		_GPIO(13)
#define STA1295_PIN_AA23		_GPIO(14)
#define STA1295_PIN_T22		_GPIO(15)
#define STA1295_PIN_R22		_GPIO(16)
#define STA1295_PIN_Y23		_GPIO(17)
#define STA1295_PIN_R23		_GPIO(18)
#define STA1295_PIN_P22		_GPIO(19)
#define STA1295_PIN_R21		_GPIO(20)
#define STA1295_PIN_P23		_GPIO(21)
#define STA1295_PIN_AB20		_GPIO(22)
#define STA1295_PIN_Y20		_GPIO(23)
#define STA1295_PIN_Y21		_GPIO(24)
#define STA1295_PIN_W21		_GPIO(25)
#define STA1295_PIN_A4		_GPIO(26)
#define STA1295_PIN_A2		_GPIO(27)
#define STA1295_PIN_A3		_GPIO(28)
#define STA1295_PIN_C3		_GPIO(29)
#define STA1295_PIN_AB21		_GPIO(30)
#define STA1295_PIN_F4		_GPIO(31)
#define STA1295_PIN_D20		_GPIO(32)
#define STA1295_PIN_Y22		_GPIO(33)
#define STA1295_PIN_C6		_GPIO(34)
#define STA1295_PIN_B4		_GPIO(35)
#define STA1295_PIN_C7		_GPIO(36)
#define STA1295_PIN_A5		_GPIO(37)
#define STA1295_PIN_B1		_GPIO(38)
#define STA1295_PIN_B2		_GPIO(39)
#define STA1295_PIN_C20		_GPIO(40)
#define STA1295_PIN_C21		_GPIO(41)
#define STA1295_PIN_C22		_GPIO(42)
#define STA1295_PIN_C23		_GPIO(43)
#define STA1295_PIN_F12		_GPIO(44)
#define STA1295_PIN_F11		_GPIO(45)
#define STA1295_PIN_E20		_GPIO(46)
#define STA1295_PIN_E19		_GPIO(47)
#define STA1295_PIN_E17		_GPIO(48)
#define STA1295_PIN_A13		_GPIO(49)
#define STA1295_PIN_A14		_GPIO(50)
#define STA1295_PIN_C5		_GPIO(51)
#define STA1295_PIN_D15		_GPIO(52)
#define STA1295_PIN_A15		_GPIO(53)
#define STA1295_PIN_D16		_GPIO(54)
#define STA1295_PIN_C15		_GPIO(55)
#define STA1295_PIN_C16		_GPIO(56)
#define STA1295_PIN_A16		_GPIO(57)
#define STA1295_PIN_B18		_GPIO(58)
#define STA1295_PIN_A17		_GPIO(59)
#define STA1295_PIN_B17		_GPIO(60)
#define STA1295_PIN_C17		_GPIO(61)
#define STA1295_PIN_B16		_GPIO(62)
#define STA1295_PIN_Y19		_GPIO(63)
#define STA1295_PIN_AA19		_GPIO(64)
#define STA1295_PIN_AB19		_GPIO(65)
#define STA1295_PIN_AC19		_GPIO(66)
#define STA1295_PIN_AC20		_GPIO(67)
#define STA1295_PIN_Y18		_GPIO(68)
#define STA1295_PIN_AA18		_GPIO(69)
#define STA1295_PIN_AB18		_GPIO(70)
#define STA1295_PIN_AC18		_GPIO(71)
#define STA1295_PIN_Y17		_GPIO(72)
#define STA1295_PIN_AA17		_GPIO(73)
#define STA1295_PIN_AB17		_GPIO(74)
#define STA1295_PIN_AC17		_GPIO(75)
#define STA1295_PIN_Y16		_GPIO(76)
#define STA1295_PIN_AA16		_GPIO(77)
#define STA1295_PIN_AB16		_GPIO(78)
#define STA1295_PIN_AC16		_GPIO(79)
#define STA1295_PIN_Y15		_GPIO(80)
#define STA1295_PIN_AA15		_GPIO(81)
#define STA1295_PIN_AB15		_GPIO(82)
#define STA1295_PIN_AC15		_GPIO(83)
#define STA1295_PIN_Y14		_GPIO(84)
#define STA1295_PIN_AA14		_GPIO(85)
#define STA1295_PIN_AC14		_GPIO(86)
#define STA1295_PIN_AB14		_GPIO(87)
#define STA1295_PIN_Y13		_GPIO(88)
#define STA1295_PIN_AA13		_GPIO(89)
#define STA1295_PIN_AB13		_GPIO(90)
#define STA1295_PIN_AC13		_GPIO(91)
#define STA1295_PIN_A10		_GPIO(92)
#define STA1295_PIN_C10		_GPIO(93)
#define STA1295_PIN_B11		_GPIO(94)
#define STA1295_PIN_D10		_GPIO(95)
#define STA1295_PIN_E10		_GPIO(96)
#define STA1295_PIN_A11		_GPIO(97)
#define STA1295_PIN_B12		_GPIO(98)
#define STA1295_PIN_C11		_GPIO(99)
#define STA1295_PIN_A12		_GPIO(100)
#define STA1295_PIN_F19		_GPIO(101)
#define STA1295_PIN_D11		_GPIO(102)
#define STA1295_PIN_E11		_GPIO(103)
#define STA1295_PIN_D12		_GPIO(104)
#define STA1295_PIN_B13		_GPIO(105)
#define STA1295_PIN_D13		_GPIO(106)
#define STA1295_PIN_E12		_GPIO(107)
#define STA1295_PIN_E13		_GPIO(108)
#define STA1295_PIN_C12		_GPIO(109)
#define STA1295_PIN_B15		_GPIO(110)
#define STA1295_PIN_B14		_GPIO(111)
#define STA1295_PIN_C14		_GPIO(112)
#define STA1295_PIN_C13		_GPIO(113)
#define STA1295_PIN_E5		_GPIO(114)
#define STA1295_PIN_F1		_GPIO(115)
#define STA1295_PIN_C4		_GPIO(116)
#define STA1295_PIN_E3		_GPIO(117)
#define STA1295_PIN_E1		_GPIO(118)
#define STA1295_PIN_E4		_GPIO(119)
#define STA1295_PIN_B3		_GPIO(120)
#define STA1295_PIN_E2		_GPIO(121)
#define STA1295_PIN_D3		_GPIO(122)
#define STA1295_PIN_D1		_GPIO(123)
#define STA1295_PIN_D2		_GPIO(124)
#define STA1295_PIN_C1		_GPIO(125)
#define STA1295_PIN_D4		_GPIO(126)
#define STA1295_PIN_C2		_GPIO(127)

/* shared gpios */

#define STA1295_PIN_D6		S_GPIO(0)
#define STA1295_PIN_D5		S_GPIO(1)
#define STA1295_PIN_F20		S_GPIO(2)
#define STA1295_PIN_E6		S_GPIO(3)
#define STA1295_PIN_D7		S_GPIO(4)
#define STA1295_PIN_G20		S_GPIO(5)
#define STA1295_PIN_B5		S_GPIO(6)
#define STA1295_PIN_A6		S_GPIO(7)
#define STA1295_PIN_A7		S_GPIO(8)
#define STA1295_PIN_A8		S_GPIO(9)
#define STA1295_PIN_B6		S_GPIO(10)
#define STA1295_PIN_B7		S_GPIO(11)
#define STA1295_PIN_W22		S_GPIO(12)
#define STA1295_PIN_W19		S_GPIO(13)
#define STA1295_PIN_AA20		S_GPIO(14)
#define STA1295_PIN_W20		S_GPIO(15)

/* m3 gpios */

#define STA1295_PIN_U20		M3_GPIO(0)
#define STA1295_PIN_U21		M3_GPIO(1)
#define STA1295_PIN_V20		M3_GPIO(2)
#define STA1295_PIN_T20		M3_GPIO(3)
#define STA1295_PIN_V21		M3_GPIO(4)
#define STA1295_PIN_W23		M3_GPIO(5)
#define STA1295_PIN_V23		M3_GPIO(6)
#define STA1295_PIN_R20		M3_GPIO(7)
#define STA1295_PIN_B10		M3_GPIO(8)
#define STA1295_PIN_C8		M3_GPIO(9)
#define STA1295_PIN_C9		M3_GPIO(10)
#define STA1295_PIN_D8		M3_GPIO(11)
#define STA1295_PIN_D9		M3_GPIO(12)
#define STA1295_PIN_B8		M3_GPIO(13)
#define STA1295_PIN_B9		M3_GPIO(14)
#define STA1295_PIN_A9		M3_GPIO(15)

/*
 * The names of the pins are denoted by GPIO number and ball name, even
 * though they can be used for other things than GPIO, this is the first
 * column in the table of the data sheet and often used on schematics and
 * such.
 */
static const struct pinctrl_pin_desc nmk_sta1295_pins[] = {
	PINCTRL_PIN(STA1295_PIN_B21,		"GPIO0_B21"),
	PINCTRL_PIN(STA1295_PIN_B22,		"GPIO1_B22"),
	PINCTRL_PIN(STA1295_PIN_D19,		"GPIO2_D19"),
	PINCTRL_PIN(STA1295_PIN_A22,		"GPIO3_A22"),
	PINCTRL_PIN(STA1295_PIN_B19,		"GPIO4_B19"),
	PINCTRL_PIN(STA1295_PIN_B20,		"GPIO5_B20"),
	PINCTRL_PIN(STA1295_PIN_D14,		"GPIO6_D14"),
	PINCTRL_PIN(STA1295_PIN_T23,		"GPIO7_T23"),
	PINCTRL_PIN(STA1295_PIN_B23,		"GPIO8_B23"),
	PINCTRL_PIN(STA1295_PIN_A18,		"GPIO9_A18"),
	PINCTRL_PIN(STA1295_PIN_A20,		"GPIO10_A20"),
	PINCTRL_PIN(STA1295_PIN_A19,		"GPIO11_A19"),
	PINCTRL_PIN(STA1295_PIN_A21,		"GPIO12_A21"),
	PINCTRL_PIN(STA1295_PIN_C18,		"GPIO13_C18"),
	PINCTRL_PIN(STA1295_PIN_AA23,		"GPIO14_AA23"),
	PINCTRL_PIN(STA1295_PIN_T22,		"GPIO15_T22"),
	PINCTRL_PIN(STA1295_PIN_R22,		"GPIO16_R22"),
	PINCTRL_PIN(STA1295_PIN_Y23,		"GPIO17_Y23"),
	PINCTRL_PIN(STA1295_PIN_R23,		"GPIO18_R23"),
	PINCTRL_PIN(STA1295_PIN_P22,		"GPIO19_P22"),
	PINCTRL_PIN(STA1295_PIN_R21,		"GPIO20_R21"),
	PINCTRL_PIN(STA1295_PIN_P23,		"GPIO21_P23"),
	PINCTRL_PIN(STA1295_PIN_AB20,		"GPIO22_AB20"),
	PINCTRL_PIN(STA1295_PIN_Y20,		"GPIO23_Y20"),
	PINCTRL_PIN(STA1295_PIN_Y21,		"GPIO24_Y21"),
	PINCTRL_PIN(STA1295_PIN_W21,		"GPIO25_W21"),
	PINCTRL_PIN(STA1295_PIN_A4,		"GPIO26_A4"),
	PINCTRL_PIN(STA1295_PIN_A2,		"GPIO27_A2"),
	PINCTRL_PIN(STA1295_PIN_A3,		"GPIO28_A3"),
	PINCTRL_PIN(STA1295_PIN_C3,		"GPIO29_C3"),
	PINCTRL_PIN(STA1295_PIN_AB21,		"GPIO30_AB21"),
	PINCTRL_PIN(STA1295_PIN_F4,		"GPIO31_F4"),
	PINCTRL_PIN(STA1295_PIN_D20,		"GPIO32_D20"),
	PINCTRL_PIN(STA1295_PIN_Y22,		"GPIO33_Y22"),
	PINCTRL_PIN(STA1295_PIN_C6,		"GPIO34_C6"),
	PINCTRL_PIN(STA1295_PIN_B4,		"GPIO35_B4"),
	PINCTRL_PIN(STA1295_PIN_C7,		"GPIO36_C7"),
	PINCTRL_PIN(STA1295_PIN_A5,		"GPIO37_A5"),
	PINCTRL_PIN(STA1295_PIN_B1,		"GPIO38_B1"),
	PINCTRL_PIN(STA1295_PIN_B2,		"GPIO39_B2"),
	PINCTRL_PIN(STA1295_PIN_C20,		"GPIO40_C20"),
	PINCTRL_PIN(STA1295_PIN_C21,		"GPIO41_C21"),
	PINCTRL_PIN(STA1295_PIN_C22,		"GPIO42_C22"),
	PINCTRL_PIN(STA1295_PIN_C23,		"GPIO43_C23"),
	PINCTRL_PIN(STA1295_PIN_F12,		"GPIO44_F12"),
	PINCTRL_PIN(STA1295_PIN_F11,		"GPIO45_F11"),
	PINCTRL_PIN(STA1295_PIN_E20,		"GPIO46_E20"),
	PINCTRL_PIN(STA1295_PIN_E19,		"GPIO47_E19"),
	PINCTRL_PIN(STA1295_PIN_E17,		"GPIO48_E17"),
	PINCTRL_PIN(STA1295_PIN_A13,		"GPIO49_A13"),
	PINCTRL_PIN(STA1295_PIN_A14,		"GPIO50_A14"),
	PINCTRL_PIN(STA1295_PIN_C5,		"GPIO51_C5"),
	PINCTRL_PIN(STA1295_PIN_D15,		"GPIO52_D15"),
	PINCTRL_PIN(STA1295_PIN_A15,		"GPIO53_A15"),
	PINCTRL_PIN(STA1295_PIN_D16,		"GPIO54_D16"),
	PINCTRL_PIN(STA1295_PIN_C15,		"GPIO55_C15"),
	PINCTRL_PIN(STA1295_PIN_C16,		"GPIO56_C16"),
	PINCTRL_PIN(STA1295_PIN_A16,		"GPIO57_A16"),
	PINCTRL_PIN(STA1295_PIN_B18,		"GPIO58_B18"),
	PINCTRL_PIN(STA1295_PIN_A17,		"GPIO59_A17"),
	PINCTRL_PIN(STA1295_PIN_B17,		"GPIO60_B17"),
	PINCTRL_PIN(STA1295_PIN_C17,		"GPIO61_C17"),
	PINCTRL_PIN(STA1295_PIN_B16,		"GPIO62_B16"),
	PINCTRL_PIN(STA1295_PIN_Y19,		"GPIO63_Y19"),
	PINCTRL_PIN(STA1295_PIN_AA19,		"GPIO64_AA19"),
	PINCTRL_PIN(STA1295_PIN_AB19,		"GPIO65_AB19"),
	PINCTRL_PIN(STA1295_PIN_AC19,		"GPIO66_AC19"),
	PINCTRL_PIN(STA1295_PIN_AC20,		"GPIO67_AC20"),
	PINCTRL_PIN(STA1295_PIN_Y18,		"GPIO68_Y18"),
	PINCTRL_PIN(STA1295_PIN_AA18,		"GPIO69_AA18"),
	PINCTRL_PIN(STA1295_PIN_AB18,		"GPIO70_AB18"),
	PINCTRL_PIN(STA1295_PIN_AC18,		"GPIO71_AC18"),
	PINCTRL_PIN(STA1295_PIN_Y17,		"GPIO72_Y17"),
	PINCTRL_PIN(STA1295_PIN_AA17,		"GPIO73_AA17"),
	PINCTRL_PIN(STA1295_PIN_AB17,		"GPIO74_AB17"),
	PINCTRL_PIN(STA1295_PIN_AC17,		"GPIO75_AC17"),
	PINCTRL_PIN(STA1295_PIN_Y16,		"GPIO76_Y16"),
	PINCTRL_PIN(STA1295_PIN_AA16,		"GPIO77_AA16"),
	PINCTRL_PIN(STA1295_PIN_AB16,		"GPIO78_AB16"),
	PINCTRL_PIN(STA1295_PIN_AC16,		"GPIO79_AC16"),
	PINCTRL_PIN(STA1295_PIN_Y15,		"GPIO80_Y15"),
	PINCTRL_PIN(STA1295_PIN_AA15,		"GPIO81_AA15"),
	PINCTRL_PIN(STA1295_PIN_AB15,		"GPIO82_AB15"),
	PINCTRL_PIN(STA1295_PIN_AC15,		"GPIO83_AC15"),
	PINCTRL_PIN(STA1295_PIN_Y14,		"GPIO84_Y14"),
	PINCTRL_PIN(STA1295_PIN_AA14,		"GPIO85_AA14"),
	PINCTRL_PIN(STA1295_PIN_AC14,		"GPIO86_AC14"),
	PINCTRL_PIN(STA1295_PIN_AB14,		"GPIO87_AB14"),
	PINCTRL_PIN(STA1295_PIN_Y13,		"GPIO88_Y13"),
	PINCTRL_PIN(STA1295_PIN_AA13,		"GPIO89_AA13"),
	PINCTRL_PIN(STA1295_PIN_AB13,		"GPIO90_AB13"),
	PINCTRL_PIN(STA1295_PIN_AC13,		"GPIO91_AC13"),
	PINCTRL_PIN(STA1295_PIN_A10,		"GPIO92_A10"),
	PINCTRL_PIN(STA1295_PIN_C10,		"GPIO93_C10"),
	PINCTRL_PIN(STA1295_PIN_B11,		"GPIO94_B11"),
	PINCTRL_PIN(STA1295_PIN_D10,		"GPIO95_D10"),
	PINCTRL_PIN(STA1295_PIN_E10,		"GPIO96_E10"),
	PINCTRL_PIN(STA1295_PIN_A11,		"GPIO97_A11"),
	PINCTRL_PIN(STA1295_PIN_B12,		"GPIO98_B12"),
	PINCTRL_PIN(STA1295_PIN_C11,		"GPIO99_C11"),
	PINCTRL_PIN(STA1295_PIN_A12,		"GPIO100_A12"),
	PINCTRL_PIN(STA1295_PIN_F19,		"GPIO101_F19"),
	PINCTRL_PIN(STA1295_PIN_D11,		"GPIO102_D11"),
	PINCTRL_PIN(STA1295_PIN_E11,		"GPIO103_E11"),
	PINCTRL_PIN(STA1295_PIN_D12,		"GPIO104_D12"),
	PINCTRL_PIN(STA1295_PIN_B13,		"GPIO105_B13"),
	PINCTRL_PIN(STA1295_PIN_D13,		"GPIO106_D13"),
	PINCTRL_PIN(STA1295_PIN_E12,		"GPIO107_E12"),
	PINCTRL_PIN(STA1295_PIN_E13,		"GPIO108_E13"),
	PINCTRL_PIN(STA1295_PIN_C12,		"GPIO109_C12"),
	PINCTRL_PIN(STA1295_PIN_B15,		"GPIO110_B15"),
	PINCTRL_PIN(STA1295_PIN_B14,		"GPIO111_B14"),
	PINCTRL_PIN(STA1295_PIN_C14,		"GPIO112_C14"),
	PINCTRL_PIN(STA1295_PIN_C13,		"GPIO113_C13"),
	PINCTRL_PIN(STA1295_PIN_E5,		"GPIO114_E5"),
	PINCTRL_PIN(STA1295_PIN_F1,		"GPIO115_F1"),
	PINCTRL_PIN(STA1295_PIN_C4,		"GPIO116_C4"),
	PINCTRL_PIN(STA1295_PIN_E3,		"GPIO117_E3"),
	PINCTRL_PIN(STA1295_PIN_E1,		"GPIO118_E1"),
	PINCTRL_PIN(STA1295_PIN_E4,		"GPIO119_E4"),
	PINCTRL_PIN(STA1295_PIN_B3,		"GPIO120_B3"),
	PINCTRL_PIN(STA1295_PIN_E2,		"GPIO121_E2"),
	PINCTRL_PIN(STA1295_PIN_D3,		"GPIO122_D3"),
	PINCTRL_PIN(STA1295_PIN_D1,		"GPIO123_D1"),
	PINCTRL_PIN(STA1295_PIN_D2,		"GPIO124_D2"),
	PINCTRL_PIN(STA1295_PIN_C1,		"GPIO125_C1"),
	PINCTRL_PIN(STA1295_PIN_D4,		"GPIO126_D4"),
	PINCTRL_PIN(STA1295_PIN_C2,		"GPIO127_C2"),
	PINCTRL_PIN(STA1295_PIN_D6,		"S_GPIO0_D6"),
	PINCTRL_PIN(STA1295_PIN_D5,		"S_GPIO1_D5"),
	PINCTRL_PIN(STA1295_PIN_F20,		"S_GPIO2_F20"),
	PINCTRL_PIN(STA1295_PIN_E6,		"S_GPIO3_E6"),
	PINCTRL_PIN(STA1295_PIN_D7,		"S_GPIO4_D7"),
	PINCTRL_PIN(STA1295_PIN_G20,		"S_GPIO5_G20"),
	PINCTRL_PIN(STA1295_PIN_B5,		"S_GPIO6_B5"),
	PINCTRL_PIN(STA1295_PIN_A6,		"S_GPIO7_A6"),
	PINCTRL_PIN(STA1295_PIN_A7,		"S_GPIO8_A7"),
	PINCTRL_PIN(STA1295_PIN_A8,		"S_GPIO9_A8"),
	PINCTRL_PIN(STA1295_PIN_B6,		"S_GPIO10_B6"),
	PINCTRL_PIN(STA1295_PIN_B7,		"S_GPIO11_B7"),
	PINCTRL_PIN(STA1295_PIN_W22,		"S_GPIO12_W22"),
	PINCTRL_PIN(STA1295_PIN_W19,		"S_GPIO13_W19"),
	PINCTRL_PIN(STA1295_PIN_AA20,		"S_GPIO14_AA20"),
	PINCTRL_PIN(STA1295_PIN_W20,		"S_GPIO15_W20"),
	PINCTRL_PIN(STA1295_PIN_U20,		"M3_GPIO0_U20"),
	PINCTRL_PIN(STA1295_PIN_U21,		"M3_GPIO1_U21"),
	PINCTRL_PIN(STA1295_PIN_V20,		"M3_GPIO2_V20"),
	PINCTRL_PIN(STA1295_PIN_T20,		"M3_GPIO3_T20"),
	PINCTRL_PIN(STA1295_PIN_V21,		"M3_GPIO4_V21"),
	PINCTRL_PIN(STA1295_PIN_W23,		"M3_GPIO5_W23"),
	PINCTRL_PIN(STA1295_PIN_V23,		"M3_GPIO6_V23"),
	PINCTRL_PIN(STA1295_PIN_R20,		"M3_GPIO7_R20"),
	PINCTRL_PIN(STA1295_PIN_B10,		"M3_GPIO8_B10"),
	PINCTRL_PIN(STA1295_PIN_C8,		"M3_GPIO9_C8"),
	PINCTRL_PIN(STA1295_PIN_C9,		"M3_GPIO10_C9"),
	PINCTRL_PIN(STA1295_PIN_D8,		"M3_GPIO11_D8"),
	PINCTRL_PIN(STA1295_PIN_D9,		"M3_GPIO12_D9"),
	PINCTRL_PIN(STA1295_PIN_B8,		"M3_GPIO13_B8"),
	PINCTRL_PIN(STA1295_PIN_B9,		"M3_GPIO14_B9"),
	PINCTRL_PIN(STA1295_PIN_A9,		"M3_GPIO15_A9"),
};

/*
 * Read the pin group names like this:
 * u0_a_1    = first group of pins for uart0 on alt function a
 * i2c2_b_2  = second group of pins for i2c2 on alt function b
 */

/* Altfunction A */

/* mc1_a_1 =
	SDMMC1_CMD, SDMMC1_CLK, SDMMC1_DATA_0, SDMMC1_DATA_1,
	SDMMC1_DATA_2, SDMMC1_DATA_3 */
static const unsigned mc1_a_1_pins[] = {
	STA1295_PIN_B21, STA1295_PIN_B22, STA1295_PIN_D19, STA1295_PIN_A22,
	STA1295_PIN_B19, STA1295_PIN_B20 };
/* mc1cmd_a_1 =
	SDMMC1_CMD */
static const unsigned mc1cmd_a_1_pins[] = {
	STA1295_PIN_B21 };
/* mc1clk_a_1 =
	SDMMC1_CLK */
static const unsigned mc1clk_a_1_pins[] = {
	STA1295_PIN_B22 };
/* mc1dat0_a_1 =
	SDMMC1_DATA_0 */
static const unsigned mc1dat0_a_1_pins[] = {
	STA1295_PIN_D19 };
/* sqifdbsck_a_1 =
	SQI_FDBSCK */
static const unsigned sqifdbsck_a_1_pins[] = {
	STA1295_PIN_D14 };
/* audrefclk_a_1 =
	AUDIO_REFCLK */
static const unsigned audrefclk_a_1_pins[] = {
	STA1295_PIN_T23 };
/* sai3bclk_a_1 =
	SAI3_BCLK */
static const unsigned sai3bclk_a_1_pins[] = {
	STA1295_PIN_B23 };
/* sai3fs_a_1 =
	SAI3_FS */
static const unsigned sai3fs_a_1_pins[] = {
	STA1295_PIN_A18 };
/* sai3rx2_a_1 =
	SAI3_RX2 */
static const unsigned sai3rx2_a_1_pins[] = {
	STA1295_PIN_A20 };
/* sai3tx1_a_1 =
	SAI3_TX1 */
static const unsigned sai3tx1_a_1_pins[] = {
	STA1295_PIN_A19 };
/* sai3tx2_a_1 =
	SAI3_TX2 */
static const unsigned sai3tx2_a_1_pins[] = {
	STA1295_PIN_A21 };
/* sai3rx1_a_1 =
	SAI3_RX1 */
static const unsigned sai3rx1_a_1_pins[] = {
	STA1295_PIN_C18 };
/* sai3tx0_a_1 =
	SAI3_TX0 */
static const unsigned sai3tx0_a_1_pins[] = {
	STA1295_PIN_AA23 };
/* sai3rx0_a_1 =
	SAI3_RX0 */
static const unsigned sai3rx0_a_1_pins[] = {
	STA1295_PIN_T22 };
/* sai2_a_1 =
	SAI2_BCLK, SAI2_FS, SAI2_RX/TX */
static const unsigned sai2_a_1_pins[] = {
	STA1295_PIN_R22, STA1295_PIN_Y23, STA1295_PIN_R23 };
/* sai1_a_1 =
	SAI1_BCLK, SAI1_FS, SAI1_RX */
static const unsigned sai1_a_1_pins[] = {
	STA1295_PIN_P22, STA1295_PIN_R21, STA1295_PIN_P23 };
/* i2s0_a_1 =
	I2S0_BCLK, I2S0_FS, I2S0_RX */
static const unsigned i2s0_a_1_pins[] = {
	STA1295_PIN_AB20, STA1295_PIN_Y20, STA1295_PIN_W21 };
/* i2s0tx_a_1 =
	I2S0_TX */
static const unsigned i2s0tx_a_1_pins[] = {
	STA1295_PIN_Y21 };
/* eft0_a_1 =
	EFT0_ICAP0, EFT0_ICAP1, EFT0_OCMP0, EFT0_OCMP1 */
static const unsigned eft0_a_1_pins[] = {
	STA1295_PIN_A4, STA1295_PIN_A2, STA1295_PIN_A3, STA1295_PIN_C3 };
/* eft1_a_1 =
	EFT1_ICAP0, EFT1_ICAP1, EFT1_OCMP1, EFT1_OCMP0 */
static const unsigned eft1_a_1_pins[] = {
	STA1295_PIN_AB21, STA1295_PIN_F4, STA1295_PIN_D20, STA1295_PIN_Y22 };
/* spi1ss_a_1 =
	SPI1_SS */
static const unsigned spi1ss_a_1_pins[] = {
	STA1295_PIN_C6 };
/* spi1_a_1 =
	SPI1_TXD, SPI1_RXD, SPI1_SCK */
static const unsigned spi1_a_1_pins[] = {
	STA1295_PIN_B4, STA1295_PIN_C7, STA1295_PIN_A5 };
/* ethmdio_a_1 =
	ETH_MDIO */
static const unsigned ethmdio_a_1_pins[] = {
	STA1295_PIN_B1 };
/* ethmdc_a_1 =
	ETH_MDC */
static const unsigned ethmdc_a_1_pins[] = {
	STA1295_PIN_B2 };
/* u1rxtx_a_1 =
	UART1_TX, UART1_RX */
static const unsigned u1rxtx_a_1_pins[] = {
	STA1295_PIN_C20, STA1295_PIN_C21 };
/* u2rxtx_a_1 =
	UART2_RX, UART2_TX */
static const unsigned u2rxtx_a_1_pins[] = {
	STA1295_PIN_C22, STA1295_PIN_C23 };
/* u3rxtx_a_1 =
	UART3_RX, UART3_TX */
static const unsigned u3rxtx_a_1_pins[] = {
	STA1295_PIN_F12, STA1295_PIN_F11 };
/* i2c1_a_1 =
	I2C1_SDA, I2C1_SCL */
static const unsigned i2c1_a_1_pins[] = {
	STA1295_PIN_E20, STA1295_PIN_E19 };
/* eft2icap0_a_1 =
	EFT2_ICAP0 */
static const unsigned eft2icap0_a_1_pins[] = {
	STA1295_PIN_E17 };
/* i2c1_a_2 =
	I2C1_SDA, I2C1_SCL */
static const unsigned i2c1_a_2_pins[] = {
	STA1295_PIN_A13, STA1295_PIN_A14 };
/* wdgswrstout_a_1 =
	WDG_SW_RSTOUT */
static const unsigned wdgswrstout_a_1_pins[] = {
	STA1295_PIN_C5 };
/* vip_a_1 =
	VIP_PIXCLK, VIP_HSYNCH, VIP_VSYNCH, VIP_DAT7,
	VIP_DAT6, VIP_DAT5, VIP_DAT4,
	VIP_DAT3, VIP_DAT2, VIP_DAT1,
	VIP_DAT0 */
static const unsigned vip_a_1_pins[] = {
	STA1295_PIN_D15, STA1295_PIN_A15, STA1295_PIN_D16, STA1295_PIN_C15,
	STA1295_PIN_C16, STA1295_PIN_A16, STA1295_PIN_B18,
	STA1295_PIN_A17, STA1295_PIN_B17, STA1295_PIN_C17,
	STA1295_PIN_B16 };
/* clcd_a_1 =
	CLCD_PIXCLK0, CLCD_VSYNCH, CLCD_HSYNCH, CLCD_DE,
	CLCD_COLOR23, CLCD_COLOR22, CLCD_COLOR21,
	CLCD_COLOR20, CLCD_COLOR19, CLCD_COLOR18,
	CLCD_COLOR17, CLCD_COLOR16, CLCD_COLOR15,
	CLCD_COLOR14, CLCD_COLOR13, CLCD_COLOR12,
	CLCD_COLOR11, CLCD_COLOR10, CLCD_COLOR9,
	CLCD_COLOR8, CLCD_COLOR7, CLCD_COLOR6,
	CLCD_COLOR5, CLCD_COLOR4, CLCD_COLOR3,
	CLCD_COLOR2, CLCD_COLOR1, CLCD_COLOR0,
	CLCD_PIXCLK1 */
static const unsigned clcd_a_1_pins[] = {
	STA1295_PIN_Y19, STA1295_PIN_AA19, STA1295_PIN_AB19, STA1295_PIN_AC19,
	STA1295_PIN_AC20, STA1295_PIN_Y18, STA1295_PIN_AA18,
	STA1295_PIN_AB18, STA1295_PIN_AC18, STA1295_PIN_Y17,
	STA1295_PIN_AA17, STA1295_PIN_AB17, STA1295_PIN_AC17,
	STA1295_PIN_Y16, STA1295_PIN_AA16, STA1295_PIN_AB16,
	STA1295_PIN_AC16, STA1295_PIN_Y15, STA1295_PIN_AA15,
	STA1295_PIN_AB15, STA1295_PIN_AC15, STA1295_PIN_Y14,
	STA1295_PIN_AA14, STA1295_PIN_AC14, STA1295_PIN_AB14,
	STA1295_PIN_Y13, STA1295_PIN_AA13, STA1295_PIN_AB13,
	STA1295_PIN_AC13 };
/* fsmcsmadq0to7_a_1 =
	FSMC_SMADQ_7, FSMC_SMADQ_6, FSMC_SMADQ_5, FSMC_SMADQ_4,
	FSMC_SMADQ_3, FSMC_SMADQ_2, FSMC_SMADQ_1,
	FSMC_SMADQ_0 */
static const unsigned fsmcsmadq0to7_a_1_pins[] = {
	STA1295_PIN_A10, STA1295_PIN_C10, STA1295_PIN_B11, STA1295_PIN_D10,
	STA1295_PIN_E10, STA1295_PIN_A11, STA1295_PIN_B12,
	STA1295_PIN_C11 };
/* fsmcwpn_a_1 =
	FSMC_WPn */
static const unsigned fsmcwpn_a_1_pins[] = {
	STA1295_PIN_A12 };
/* sqice1n_a_1 =
	SQI_CE1n */
static const unsigned sqice1n_a_1_pins[] = {
	STA1295_PIN_F19 };
/* fsmcbusy_a_1 =
	FSMC_BUSYn */
static const unsigned fsmcbusy_a_1_pins[] = {
	STA1295_PIN_D11 };
/* fsmcoen_a_1 =
	FSMC_OEn */
static const unsigned fsmcoen_a_1_pins[] = {
	STA1295_PIN_E11 };
/* fsmcwen_a_1 =
	FSMC_WEn */
static const unsigned fsmcwen_a_1_pins[] = {
	STA1295_PIN_D12 };
/* fsmcsmad17ale_a_1 =
	FSMC_SMAD17/ALE */
static const unsigned fsmcsmad17ale_a_1_pins[] = {
	STA1295_PIN_B13 };
/* fsmcsmad16cle_a_1 =
	FSMC_SMAD16/CLE */
static const unsigned fsmcsmad16cle_a_1_pins[] = {
	STA1295_PIN_D13 };
/* fsmcnandcs0n_a_1 =
	FSMC_NAND_CS0n */
static const unsigned fsmcnandcs0n_a_1_pins[] = {
	STA1295_PIN_E12 };
/* sqi_a_1 =
	SQI_SIO3, SQI_SIO2, SQI_SIO1, SQI_SIO0,
	SQI_SCK, SQI_CE0n */
static const unsigned sqi_a_1_pins[] = {
	STA1295_PIN_E13, STA1295_PIN_C12, STA1295_PIN_B15, STA1295_PIN_B14,
	STA1295_PIN_C14, STA1295_PIN_C13 };
/* i2c2_a_1 =
	I2C2_SDA, I2C2_SCL */
static const unsigned i2c2_a_1_pins[] = {
	STA1295_PIN_E5, STA1295_PIN_F1 };
/* ethrmii_a_1 =
	ETH_TXEN, ETH_TX[0], ETH_TX[1], ETH_RXDV_CRS,
	ETH_RX[0], ETH_RX[1] */
static const unsigned ethrmii_a_1_pins[] = {
	STA1295_PIN_C4, STA1295_PIN_E3, STA1295_PIN_E1, STA1295_PIN_E4,
	STA1295_PIN_B3, STA1295_PIN_E2 };
/* ethrmiiclk_a_1 =
	ETH_RMII_CLK */
static const unsigned ethrmiiclk_a_1_pins[] = {
	STA1295_PIN_D3 };
/* ethmiitxclk_a_1 =
	ETH_MII_TX_CLK */
static const unsigned ethmiitxclk_a_1_pins[] = {
	STA1295_PIN_C2 };
/* ethmii_a_1 =
	ETH_TX[2], ETH_TX[3], ETH_RX[2], ETH_RX[3] */
static const unsigned ethmii_a_1_pins[] = {
	STA1295_PIN_D1, STA1295_PIN_D2, STA1295_PIN_C1, STA1295_PIN_D4 };
/* eft3icap1_a_1 =
	EFT3_ICAP1 */
static const unsigned eft3icap1_a_1_pins[] = {
	STA1295_PIN_D6 };
/* eft3icap0_a_1 =
	EFT3_ICAP0 */
static const unsigned eft3icap0_a_1_pins[] = {
	STA1295_PIN_D5 };
/* eft4icap0_a_1 =
	EFT4_ICAP0 */
static const unsigned eft4icap0_a_1_pins[] = {
	STA1295_PIN_F20 };
/* eft4icap1_a_1 =
	EFT4_ICAP1 */
static const unsigned eft4icap1_a_1_pins[] = {
	STA1295_PIN_E6 };
/* eft3ocmp0_a_1 =
	EFT3_OCMP0 */
static const unsigned eft3ocmp0_a_1_pins[] = {
	STA1295_PIN_D7 };
/* eft3icap1_a_2 =
	EFT3_ICAP1 */
static const unsigned eft3icap1_a_2_pins[] = {
	STA1295_PIN_G20 };
/* spi0_a_1 =
	SPI0_TXD, SPI0_RXD, SPI0_SCK */
static const unsigned spi0_a_1_pins[] = {
	STA1295_PIN_B5, STA1295_PIN_A6, STA1295_PIN_A7 };
/* spi0ss_a_1 =
	SPI0_SS */
static const unsigned spi0ss_a_1_pins[] = {
	STA1295_PIN_A8 };
/* i2c0_a_1 =
	I2C0_SDA, I2C0_SCL */
static const unsigned i2c0_a_1_pins[] = {
	STA1295_PIN_B6, STA1295_PIN_B7 };
/* u0rxtx_a_1 =
	UART0_RX, UART0_TX */
static const unsigned u0rxtx_a_1_pins[] = {
	STA1295_PIN_W22, STA1295_PIN_W19 };
/* u0rtscts_a_1 =
	UART0_CTS, UART0_RTS */
static const unsigned u0rtscts_a_1_pins[] = {
	STA1295_PIN_AA20, STA1295_PIN_W20 };
/* can0_a_1 =
	CAN0_TX, CAN0_RX */
static const unsigned can0_a_1_pins[] = {
	STA1295_PIN_B10, STA1295_PIN_C8 };
/* u0rxtx_a_2 =
	UART0_RX, UART0_TX */
static const unsigned u0rxtx_a_2_pins[] = {
	STA1295_PIN_C9, STA1295_PIN_B9 };
/* u0rtscts_a_2 =
	UART0_CTS, UART0_RTS */
static const unsigned u0rtscts_a_2_pins[] = {
	STA1295_PIN_D8, STA1295_PIN_A9 };
/* i2c0_a_2 =
	I2C0_SDA, I2C0_SCL */
static const unsigned i2c0_a_2_pins[] = {
	STA1295_PIN_D9, STA1295_PIN_B8 };

/* Altfunction B */

/* mc2dat2dir_b_1 =
	SDMMC2_DAT2_DIR */
static const unsigned mc2dat2dir_b_1_pins[] = {
	STA1295_PIN_B21 };
/* mc2dat31dir_b_1 =
	SDMMC2_DAT31_DIR */
static const unsigned mc2dat31dir_b_1_pins[] = {
	STA1295_PIN_B22 };
/* mc0data4to7_b_1 =
	SDMMC0_DATA_4, SDMMC0_DATA_5, SDMMC0_DATA_6, SDMMC0_DATA_7 */
static const unsigned mc0data4to7_b_1_pins[] = {
	STA1295_PIN_D19, STA1295_PIN_A22, STA1295_PIN_B19, STA1295_PIN_B20 };
/* sai2tx_b_1 =
	SAI2_TX */
static const unsigned sai2tx_b_1_pins[] = {
	STA1295_PIN_T23 };
/* mc1_b_1 =
	SDMMC1_CMD, SDMMC1_CLK, SDMMC1_DATA_0, SDMMC1_DATA_1,
	SDMMC1_DATA_2, SDMMC1_DATA_3 */
static const unsigned mc1_b_1_pins[] = {
	STA1295_PIN_B23, STA1295_PIN_A18, STA1295_PIN_A20, STA1295_PIN_A19,
	STA1295_PIN_A21, STA1295_PIN_C18 };
/* mc1dat123_b_1 =
	SDMMC1_DATA_1, SDMMC1_DATA_2, SDMMC1_DATA_3 */
static const unsigned mc1dat123_b_1_pins[] = {
	STA1295_PIN_A19, STA1295_PIN_A21, STA1295_PIN_C18 };
/* mc2dat0dir_b_1 =
	SDMMC2_DAT0_DIR */
static const unsigned mc2dat0dir_b_1_pins[] = {
	STA1295_PIN_AA23 };
/* mc1dat31dir_b_1 =
	SDMMC1_DAT31_DIR */
static const unsigned mc1dat31dir_b_1_pins[] = {
	STA1295_PIN_T22 };
/* fsmcdack_b_1 =
	FSMC_DACK */
static const unsigned fsmcdack_b_1_pins[] = {
	STA1295_PIN_R22 };
/* fsmcdreq_b_1 =
	FSMC_DREQ */
static const unsigned fsmcdreq_b_1_pins[] = {
	STA1295_PIN_Y23 };
/* spdifrx_b_1 =
	SPDIF_RX */
static const unsigned spdifrx_b_1_pins[] = {
	STA1295_PIN_R23 };
/* spi2_b_1 =
	SPI2_TXD, SPI2_RXD, SPI2_SCK */
static const unsigned spi2_b_1_pins[] = {
	STA1295_PIN_P22, STA1295_PIN_R21, STA1295_PIN_P23 };
/* sai4bclk_b_1 =
	SAI4_BCLK */
static const unsigned sai4bclk_b_1_pins[] = {
	STA1295_PIN_AB20 };
/* sai4fs_b_1 =
	SAI4_FS */
static const unsigned sai4fs_b_1_pins[] = {
	STA1295_PIN_Y20 };
/* sai4tx0_b_1 =
	SAI4_TX0 */
static const unsigned sai4tx0_b_1_pins[] = {
	STA1295_PIN_Y21 };
/* sai4rx0_b_1 =
	SAI4_RX0 */
static const unsigned sai4rx0_b_1_pins[] = {
	STA1295_PIN_W21 };
/* eft0extck_b_1 =
	EFT0_EXTCK */
static const unsigned eft0extck_b_1_pins[] = {
	STA1295_PIN_A4 };
/* mc2cmddir_b_1 =
	SDMMC2_CMDDIR */
static const unsigned mc2cmddir_b_1_pins[] = {
	STA1295_PIN_A2 };
/* u1rxtx_b_1 =
	UART1_TX, UART1_RX */
static const unsigned u1rxtx_b_1_pins[] = {
	STA1295_PIN_A3, STA1295_PIN_C3 };
/* eft1extck_b_1 =
	EFT1_EXTCK */
static const unsigned eft1extck_b_1_pins[] = {
	STA1295_PIN_AB21 };
/* mc0fbclk_b_1 =
	SDMMC0_FBCLK */
static const unsigned mc0fbclk_b_1_pins[] = {
	STA1295_PIN_F4 };
/* mc0pwr_b_1 =
	SDMMC0_PWR */
static const unsigned mc0pwr_b_1_pins[] = {
	STA1295_PIN_D20 };
/* mc2dat31dir_b_2 =
	SDMMC2_DAT31_DIR */
static const unsigned mc2dat31dir_b_2_pins[] = {
	STA1295_PIN_Y22 };
/* eft0extck_b_2 =
	EFT0_EXTCK */
static const unsigned eft0extck_b_2_pins[] = {
	STA1295_PIN_C6 };
/* eft1extck_b_2 =
	EFT1_EXTCK */
static const unsigned eft1extck_b_2_pins[] = {
	STA1295_PIN_B4 };
/* eft1ocmp1_b_1 =
	EFT1_OCMP1 */
static const unsigned eft1ocmp1_b_1_pins[] = {
	STA1295_PIN_C7 };
/* eft1ocmp0_b_1 =
	EFT1_OCMP0 */
static const unsigned eft1ocmp0_b_1_pins[] = {
	STA1295_PIN_A5 };
/* u1rxtx_b_2 =
	UART1_TX, UART1_RX */
static const unsigned u1rxtx_b_2_pins[] = {
	STA1295_PIN_B1, STA1295_PIN_B2 };
/* mc0data4to7_b_2 =
	SDMMC0_DATA_4, SDMMC0_DATA_5, SDMMC0_DATA_6, SDMMC0_DATA_7 */
static const unsigned mc0data4to7_b_2_pins[] = {
	STA1295_PIN_C20, STA1295_PIN_C21, STA1295_PIN_C22, STA1295_PIN_C23 };
/* u1rtscts_b_1 =
	UART1_CTS, UART1_RTS */
static const unsigned u1rtscts_b_1_pins[] = {
	STA1295_PIN_F12, STA1295_PIN_F11 };
/* eft0ocmp0_b_1 =
	EFT0_OCMP0 */
static const unsigned eft0ocmp0_b_1_pins[] = {
	STA1295_PIN_E20 };
/* eft0ocmp1_b_1 =
	EFT0_OCMP1 */
static const unsigned eft0ocmp1_b_1_pins[] = {
	STA1295_PIN_E19 };
/* eft2ocmp0_b_1 =
	EFT2_OCMP0 */
static const unsigned eft2ocmp0_b_1_pins[] = {
	STA1295_PIN_E17 };
/* fsmcsmadq89_b_1 =
	FSMC_SMADQ_8, FSMC_SMADQ_9 */
static const unsigned fsmcsmadq89_b_1_pins[] = {
	STA1295_PIN_A13, STA1295_PIN_A14 };
/* clkout1_b_1 =
	CLKOUT1 */
static const unsigned clkout1_b_1_pins[] = {
	STA1295_PIN_C5 };
/* u1rxtx_b_3 =
	UART1_TX, UART1_RX */
static const unsigned u1rxtx_b_3_pins[] = {
	STA1295_PIN_A15, STA1295_PIN_D16 };
/* clkouthclk_b_1 =
	CLKOUT_HCLK */
static const unsigned clkouthclk_b_1_pins[] = {
	STA1295_PIN_Y19 };
/* clkoutperiph0_b_1 =
	CLKOUT_PERIPH0 */
static const unsigned clkoutperiph0_b_1_pins[] = {
	STA1295_PIN_AA19 };
/* clkoutperiph1_b_1 =
	CLKOUT_PERIPH1 */
static const unsigned clkoutperiph1_b_1_pins[] = {
	STA1295_PIN_AB19 };
/* spi1_b_1 =
	SPI1_TXD, SPI1_RXD, SPI1_SCK */
static const unsigned spi1_b_1_pins[] = {
	STA1295_PIN_AC20, STA1295_PIN_Y18, STA1295_PIN_AB18 };
/* spi1ss_b_1 =
	SPI1_SS */
static const unsigned spi1ss_b_1_pins[] = {
	STA1295_PIN_AA18 };
/* i2c2_b_1 =
	I2C2_SCL, I2C2_SDA */
static const unsigned i2c2_b_1_pins[] = {
	STA1295_PIN_AC18, STA1295_PIN_Y17 };
/* fsmcadvn_b_1 =
	FSMC_ADVn */
static const unsigned fsmcadvn_b_1_pins[] = {
	STA1295_PIN_AA14 };
/* fsmccs0n_b_1 =
	FSMC_CS0n */
static const unsigned fsmccs0n_b_1_pins[] = {
	STA1295_PIN_AC14 };
/* spi2_b_2 =
	SPI2_TXD, SPI2_RXD, SPI2_SCK */
static const unsigned spi2_b_2_pins[] = {
	STA1295_PIN_AB14, STA1295_PIN_Y13, STA1295_PIN_AB13 };
/* spi2ss_b_1 =
	SPI2_SS */
static const unsigned spi2ss_b_1_pins[] = {
	STA1295_PIN_AA13 };
/* wdgswrstout_b_1 =
	WDG_SW_RSTOUT */
static const unsigned wdgswrstout_b_1_pins[] = {
	STA1295_PIN_AC13 };
/* mc2_b_1 =
	SDMMC2_CMD, SDMMC2_CLK, SDMMC2_DATA_0, SDMMC2_DATA_1,
	SDMMC2_DATA_2, SDMMC2_DATA_3 */
static const unsigned mc2_b_1_pins[] = {
	STA1295_PIN_A10, STA1295_PIN_C10, STA1295_PIN_B11, STA1295_PIN_D10,
	STA1295_PIN_E10, STA1295_PIN_A11 };
/* mc2data4to7_b_1 =
	SDMMC2_DATA_4, SDMMC2_DATA_5, SDMMC2_DATA_6, SDMMC2_DATA_7 */
static const unsigned mc2data4to7_b_1_pins[] = {
	STA1295_PIN_B12, STA1295_PIN_C11, STA1295_PIN_A12, STA1295_PIN_D11 };
/* spi2ss_b_2 =
	SPI2_SS */
static const unsigned spi2ss_b_2_pins[] = {
	STA1295_PIN_F19 };
/* mc2fbclk_b_1 =
	SDMMC2_FBCLK */
static const unsigned mc2fbclk_b_1_pins[] = {
	STA1295_PIN_E11 };
/* i2c2_b_2 =
	I2C2_SDA, I2C2_SCL */
static const unsigned i2c2_b_2_pins[] = {
	STA1295_PIN_D12, STA1295_PIN_B13 };
/* u2rxtx_b_1 =
	UART2_RX, UART2_TX */
static const unsigned u2rxtx_b_1_pins[] = {
	STA1295_PIN_D13, STA1295_PIN_E12 };
/* sai4bclk_b_2 =
	SAI4_BCLK */
static const unsigned sai4bclk_b_2_pins[] = {
	STA1295_PIN_E13 };
/* sai4fs_b_2 =
	SAI4_FS */
static const unsigned sai4fs_b_2_pins[] = {
	STA1295_PIN_C12 };
/* sai4tx0_b_2 =
	SAI4_TX0 */
static const unsigned sai4tx0_b_2_pins[] = {
	STA1295_PIN_B15 };
/* sai4tx1_b_1 =
	SAI4_TX1 */
static const unsigned sai4tx1_b_1_pins[] = {
	STA1295_PIN_B14 };
/* sai4rx0_b_2 =
	SAI4_RX0 */
static const unsigned sai4rx0_b_2_pins[] = {
	STA1295_PIN_C14 };
/* sai4rx1_b_1 =
	SAI4_RX1 */
static const unsigned sai4rx1_b_1_pins[] = {
	STA1295_PIN_C13 };
/* mc2dat0dir_b_2 =
	SDMMC2_DAT0_DIR */
static const unsigned mc2dat0dir_b_2_pins[] = {
	STA1295_PIN_E5 };
/* mc2dat31dir_b_3 =
	SDMMC2_DAT31_DIR */
static const unsigned mc2dat31dir_b_3_pins[] = {
	STA1295_PIN_F1 };
/* spi2_b_3 =
	SPI2_TXD, SPI2_RXD, SPI2_SCK */
static const unsigned spi2_b_3_pins[] = {
	STA1295_PIN_C4, STA1295_PIN_E3, STA1295_PIN_E4 };
/* spi2ss_b_3 =
	SPI2_SS */
static const unsigned spi2ss_b_3_pins[] = {
	STA1295_PIN_E1 };
/* ethmiirxclk_b_1 =
	ETH_MII_RX_CLK */
static const unsigned ethmiirxclk_b_1_pins[] = {
	STA1295_PIN_D3 };
/* u2rxtx_b_2 =
	UART2_RX, UART2_TX */
static const unsigned u2rxtx_b_2_pins[] = {
	STA1295_PIN_D1, STA1295_PIN_D2 };
/* ethrxer_b_1 =
	ETH_RX_ER */
static const unsigned ethrxer_b_1_pins[] = {
	STA1295_PIN_C1 };
/* u3rxtx_b_1 =
	UART3_RX, UART3_TX */
static const unsigned u3rxtx_b_1_pins[] = {
	STA1295_PIN_D4, STA1295_PIN_C2 };
/* eft3ocmp0_b_1 =
	EFT3_OCMP0 */
static const unsigned eft3ocmp0_b_1_pins[] = {
	STA1295_PIN_D6 };
/* eft3ocmp1_b_1 =
	EFT3_OCMP1 */
static const unsigned eft3ocmp1_b_1_pins[] = {
	STA1295_PIN_D5 };
/* spi0rx_b_1 =
	SPI0_RXD */
static const unsigned spi0rx_b_1_pins[] = {
	STA1295_PIN_F20 };
/* eft4ocmp0_b_1 =
	EFT4_OCMP0 */
static const unsigned eft4ocmp0_b_1_pins[] = {
	STA1295_PIN_E6 };
/* spi0tx_b_1 =
	SPI0_TXD */
static const unsigned spi0tx_b_1_pins[] = {
	STA1295_PIN_D7 };
/* eft4ocmp0_b_2 =
	EFT4_OCMP0 */
static const unsigned eft4ocmp0_b_2_pins[] = {
	STA1295_PIN_G20 };
/* eft4icap0_b_1 =
	EFT4_ICAP0 */
static const unsigned eft4icap0_b_1_pins[] = {
	STA1295_PIN_B5 };
/* eft4ocmp1_b_1 =
	EFT4_OCMP1 */
static const unsigned eft4ocmp1_b_1_pins[] = {
	STA1295_PIN_A6 };
/* eft4icap1_b_1 =
	EFT4_ICAP1 */
static const unsigned eft4icap1_b_1_pins[] = {
	STA1295_PIN_B6 };
/* eft4ocmp1_b_2 =
	EFT4_OCMP1 */
static const unsigned eft4ocmp1_b_2_pins[] = {
	STA1295_PIN_B7 };
/* can2_b_1 =
	CAN2_TX, CAN2_RX */
static const unsigned can2_b_1_pins[] = {
	STA1295_PIN_W22, STA1295_PIN_W19 };
/* eft4icap1_b_2 =
	EFT4_ICAP1 */
static const unsigned eft4icap1_b_2_pins[] = {
	STA1295_PIN_AA20 };
/* eft4ocmp1_b_3 =
	EFT4_OCMP1 */
static const unsigned eft4ocmp1_b_3_pins[] = {
	STA1295_PIN_W20 };
/* usb0drvvbus_b_1 =
	USB0_DRVVBUS */
static const unsigned usb0drvvbus_b_1_pins[] = {
	STA1295_PIN_D8 };

/* Altfunction C */

/* sai4tx1_c_1 =
	SAI4_TX1 */
static const unsigned sai4tx1_c_1_pins[] = {
	STA1295_PIN_B21 };
/* sai4tx2_c_1 =
	SAI4_TX2 */
static const unsigned sai4tx2_c_1_pins[] = {
	STA1295_PIN_B22 };
/* sai4tx0_c_1 =
	SAI4_TX0 */
static const unsigned sai4tx0_c_1_pins[] = {
	STA1295_PIN_D19 };
/* sai4bclk_c_1 =
	SAI4_BCLK */
static const unsigned sai4bclk_c_1_pins[] = {
	STA1295_PIN_A22 };
/* sai4fs_c_1 =
	SAI4_FS */
static const unsigned sai4fs_c_1_pins[] = {
	STA1295_PIN_B19 };
/* sai4rx0_c_1 =
	SAI4_RX0 */
static const unsigned sai4rx0_c_1_pins[] = {
	STA1295_PIN_B20 };
/* mc1dat0dir_c_1 =
	SDMMC1_DAT0_DIR */
static const unsigned mc1dat0dir_c_1_pins[] = {
	STA1295_PIN_D14 };
/* i2s2tx_c_1 =
	I2S2_TX */
static const unsigned i2s2tx_c_1_pins[] = {
	STA1295_PIN_T23 };
/* sai4rx2_c_1 =
	SAI4_RX2 */
static const unsigned sai4rx2_c_1_pins[] = {
	STA1295_PIN_B23 };
/* sai4rx1_c_1 =
	SAI4_RX1 */
static const unsigned sai4rx1_c_1_pins[] = {
	STA1295_PIN_A18 };
/* eft2ocmp1_c_1 =
	EFT2_OCMP1 */
static const unsigned eft2ocmp1_c_1_pins[] = {
	STA1295_PIN_A20 };
/* eft2icap1_c_1 =
	EFT2_ICAP1 */
static const unsigned eft2icap1_c_1_pins[] = {
	STA1295_PIN_A19 };
/* u2rxtx_c_1 =
	UART2_TX, UART2_RX */
static const unsigned u2rxtx_c_1_pins[] = {
	STA1295_PIN_A21, STA1295_PIN_C18 };
/* i2c2_c_1 =
	I2C2_SCL, I2C2_SDA */
static const unsigned i2c2_c_1_pins[] = {
	STA1295_PIN_AA23, STA1295_PIN_T22 };
/* i2s2_c_1 =
	I2S2_BCLK, I2S2_FS, I2S2_RX */
static const unsigned i2s2_c_1_pins[] = {
	STA1295_PIN_R22, STA1295_PIN_Y23, STA1295_PIN_R23 };
/* eft2ocmp0_c_1 =
	EFT2_OCMP0 */
static const unsigned eft2ocmp0_c_1_pins[] = {
	STA1295_PIN_P22 };
/* eft2ocmp1_c_2 =
	EFT2_OCMP1 */
static const unsigned eft2ocmp1_c_2_pins[] = {
	STA1295_PIN_R21 };
/* eft2extck_c_1 =
	EFT2_EXTCK */
static const unsigned eft2extck_c_1_pins[] = {
	STA1295_PIN_P23 };
/* spi2ss_c_1 =
	SPI2_SS */
static const unsigned spi2ss_c_1_pins[] = {
	STA1295_PIN_A4 };
/* spi2_c_1 =
	SPI2_TXD, SPI2_RXD, SPI2_SCK */
static const unsigned spi2_c_1_pins[] = {
	STA1295_PIN_A2, STA1295_PIN_A3, STA1295_PIN_C3 };
/* usb1drvvbus_c_1 =
	USB1_DRVVBUS */
static const unsigned usb1drvvbus_c_1_pins[] = {
	STA1295_PIN_AB21 };
/* fsmcdack_c_1 =
	FSMC_DACK */
static const unsigned fsmcdack_c_1_pins[] = {
	STA1295_PIN_F4 };
/* mc1fbclk_c_1 =
	SDMMC1_FBCLK */
static const unsigned mc1fbclk_c_1_pins[] = {
	STA1295_PIN_D20 };
/* fsmcdreq_c_1 =
	FSMC_DREQ */
static const unsigned fsmcdreq_c_1_pins[] = {
	STA1295_PIN_Y22 };
/* i2c1_c_1 =
	I2C1_SCL, I2C1_SDA */
static const unsigned i2c1_c_1_pins[] = {
	STA1295_PIN_C6, STA1295_PIN_B4 };
/* u3rxtx_c_1 =
	UART3_TX, UART3_RX */
static const unsigned u3rxtx_c_1_pins[] = {
	STA1295_PIN_C7, STA1295_PIN_A5 };
/* eft1icap0_c_1 =
	EFT1_ICAP0 */
static const unsigned eft1icap0_c_1_pins[] = {
	STA1295_PIN_B1 };
/* eft1icap1_c_1 =
	EFT1_ICAP1 */
static const unsigned eft1icap1_c_1_pins[] = {
	STA1295_PIN_B2 };
/* mc1data4to7_c_1 =
	SDMMC1_DATA_4, SDMMC1_DATA_5, SDMMC1_DATA_6, SDMMC1_DATA_7 */
static const unsigned mc1data4to7_c_1_pins[] = {
	STA1295_PIN_C20, STA1295_PIN_C21, STA1295_PIN_C22, STA1295_PIN_C23 };
/* eft2icap0_c_1 =
	EFT2_ICAP0 */
static const unsigned eft2icap0_c_1_pins[] = {
	STA1295_PIN_F12 };
/* eft2icap1_c_2 =
	EFT2_ICAP1 */
static const unsigned eft2icap1_c_2_pins[] = {
	STA1295_PIN_F11 };
/* eft0icap1_c_1 =
	EFT0_ICAP1 */
static const unsigned eft0icap1_c_1_pins[] = {
	STA1295_PIN_E20 };
/* eft0icap0_c_1 =
	EFT0_ICAP0 */
static const unsigned eft0icap0_c_1_pins[] = {
	STA1295_PIN_E19 };
/* eft2extck_c_2 =
	EFT2_EXTCK */
static const unsigned eft2extck_c_2_pins[] = {
	STA1295_PIN_E17 };
/* mc1cmddir_c_1 =
	SDMMC1_CMDDIR */
static const unsigned mc1cmddir_c_1_pins[] = {
	STA1295_PIN_A13 };
/* mc1dat31dir_c_1 =
	SDMMC1_DAT31_DIR */
static const unsigned mc1dat31dir_c_1_pins[] = {
	STA1295_PIN_A14 };
/* mc1dat2dir_c_1 =
	SDMMC1_DAT2_DIR */
static const unsigned mc1dat2dir_c_1_pins[] = {
	STA1295_PIN_C5 };
/* wdgswrstout_c_1 =
	WDG_SW_RSTOUT */
static const unsigned wdgswrstout_c_1_pins[] = {
	STA1295_PIN_A15 };
/* u1rtscts_c_1 =
	UART1_RTS, UART1_CTS */
static const unsigned u1rtscts_c_1_pins[] = {
	STA1295_PIN_AC20, STA1295_PIN_Y18 };
/* u1rxtx_c_1 =
	UART1_TX, UART1_RX */
static const unsigned u1rxtx_c_1_pins[] = {
	STA1295_PIN_AA18, STA1295_PIN_AB18 };
/* usb1drvvbus_c_2 =
	USB1_DRVVBUS */
static const unsigned usb1drvvbus_c_2_pins[] = {
	STA1295_PIN_AC13 };
/* fsmcsmadq10to15_c_1 =
	FSMC_SMADQ_10, FSMC_SMADQ_11, FSMC_SMADQ_12, FSMC_SMADQ_13,
	FSMC_SMADQ_14, FSMC_SMADQ_15 */
static const unsigned fsmcsmadq10to15_c_1_pins[] = {
	STA1295_PIN_E13, STA1295_PIN_C12, STA1295_PIN_B15, STA1295_PIN_B14,
	STA1295_PIN_C14, STA1295_PIN_C13 };
/* ethmdio_c_1 =
	ETH_MDIO */
static const unsigned ethmdio_c_1_pins[] = {
	STA1295_PIN_E5 };
/* ethmdc_c_1 =
	ETH_MDC */
static const unsigned ethmdc_c_1_pins[] = {
	STA1295_PIN_F1 };
/* i2s2tx_c_2 =
	I2S2_TX */
static const unsigned i2s2tx_c_2_pins[] = {
	STA1295_PIN_C4 };
/* i2s2_c_2 =
	I2S2_RX, I2S2_FS, I2S2_BCLK */
static const unsigned i2s2_c_2_pins[] = {
	STA1295_PIN_E3, STA1295_PIN_E1, STA1295_PIN_E4 };
/* usb0drvvbus_c_1 =
	USB0_DRVVBUS */
static const unsigned usb0drvvbus_c_1_pins[] = {
	STA1295_PIN_C1 };
/* can1_c_1 =
	CAN1_RX, CAN1_TX */
static const unsigned can1_c_1_pins[] = {
	STA1295_PIN_D6, STA1295_PIN_D5 };
/* eft4extck_c_1 =
	EFT4_EXTCK */
static const unsigned eft4extck_c_1_pins[] = {
	STA1295_PIN_F20 };
/* can2_c_1 =
	CAN2_RX, CAN2_TX */
static const unsigned can2_c_1_pins[] = {
	STA1295_PIN_E6, STA1295_PIN_A6 };
/* eft3extck_c_1 =
	EFT3_EXTCK */
static const unsigned eft3extck_c_1_pins[] = {
	STA1295_PIN_D7 };
/* eft3ocmp1_c_1 =
	EFT3_OCMP1 */
static const unsigned eft3ocmp1_c_1_pins[] = {
	STA1295_PIN_G20 };
/* eft4extck_c_2 =
	EFT4_EXTCK */
static const unsigned eft4extck_c_2_pins[] = {
	STA1295_PIN_B5 };
#define STA1295_PIN_GROUP(a, b) { .name = #a, .pins = a##_pins,	\
	.npins = ARRAY_SIZE(a##_pins), .altsetting = b }

static const struct nmk_pingroup nmk_sta1295_groups[] = {
	STA1295_PIN_GROUP(mc1_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(mc1cmd_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(mc1clk_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(mc1dat0_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(sqifdbsck_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(audrefclk_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(sai3bclk_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(sai3fs_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(sai3rx2_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(sai3tx1_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(sai3tx2_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(sai3rx1_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(sai3tx0_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(sai3rx0_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(sai2_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(sai1_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(i2s0_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(i2s0tx_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(eft0_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(eft1_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(spi1ss_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(spi1_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(ethmdio_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(ethmdc_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(u1rxtx_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(u2rxtx_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(u3rxtx_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(i2c1_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(eft2icap0_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(i2c1_a_2, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(wdgswrstout_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(vip_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(clcd_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(fsmcsmadq0to7_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(fsmcwpn_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(sqice1n_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(fsmcbusy_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(fsmcoen_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(fsmcwen_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(fsmcsmad17ale_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(fsmcsmad16cle_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(fsmcnandcs0n_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(sqi_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(i2c2_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(ethrmii_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(ethrmiiclk_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(ethmiitxclk_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(ethmii_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(eft3icap1_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(eft3icap0_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(eft4icap0_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(eft4icap1_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(eft3ocmp0_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(eft3icap1_a_2, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(spi0_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(spi0ss_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(i2c0_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(u0rxtx_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(u0rtscts_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(can0_a_1, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(u0rxtx_a_2, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(u0rtscts_a_2, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(i2c0_a_2, NMK_GPIO_ALT_A),
	STA1295_PIN_GROUP(mc2dat2dir_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(mc2dat31dir_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(mc0data4to7_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(sai2tx_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(mc1_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(mc1dat123_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(mc2dat0dir_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(mc1dat31dir_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(fsmcdack_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(fsmcdreq_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(spdifrx_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(spi2_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(sai4bclk_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(sai4fs_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(sai4tx0_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(sai4rx0_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(eft0extck_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(mc2cmddir_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(u1rxtx_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(eft1extck_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(mc0fbclk_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(mc0pwr_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(mc2dat31dir_b_2, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(eft0extck_b_2, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(eft1extck_b_2, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(eft1ocmp1_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(eft1ocmp0_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(u1rxtx_b_2, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(mc0data4to7_b_2, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(u1rtscts_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(eft0ocmp0_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(eft0ocmp1_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(eft2ocmp0_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(fsmcsmadq89_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(clkout1_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(u1rxtx_b_3, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(clkouthclk_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(clkoutperiph0_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(clkoutperiph1_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(spi1_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(spi1ss_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(i2c2_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(fsmcadvn_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(fsmccs0n_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(spi2_b_2, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(spi2ss_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(wdgswrstout_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(mc2_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(mc2data4to7_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(spi2ss_b_2, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(mc2fbclk_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(i2c2_b_2, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(u2rxtx_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(sai4bclk_b_2, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(sai4fs_b_2, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(sai4tx0_b_2, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(sai4tx1_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(sai4rx0_b_2, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(sai4rx1_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(mc2dat0dir_b_2, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(mc2dat31dir_b_3, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(spi2_b_3, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(spi2ss_b_3, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(ethmiirxclk_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(u2rxtx_b_2, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(ethrxer_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(u3rxtx_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(eft3ocmp0_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(eft3ocmp1_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(spi0rx_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(eft4ocmp0_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(spi0tx_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(eft4ocmp0_b_2, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(eft4icap0_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(eft4ocmp1_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(eft4icap1_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(eft4ocmp1_b_2, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(can2_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(eft4icap1_b_2, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(eft4ocmp1_b_3, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(usb0drvvbus_b_1, NMK_GPIO_ALT_B),
	STA1295_PIN_GROUP(sai4tx1_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(sai4tx2_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(sai4tx0_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(sai4bclk_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(sai4fs_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(sai4rx0_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(mc1dat0dir_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(i2s2tx_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(sai4rx2_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(sai4rx1_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(eft2ocmp1_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(eft2icap1_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(u2rxtx_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(i2c2_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(i2s2_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(eft2ocmp0_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(eft2ocmp1_c_2, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(eft2extck_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(spi2ss_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(spi2_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(usb1drvvbus_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(fsmcdack_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(mc1fbclk_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(fsmcdreq_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(i2c1_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(u3rxtx_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(eft1icap0_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(eft1icap1_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(mc1data4to7_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(eft2icap0_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(eft2icap1_c_2, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(eft0icap1_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(eft0icap0_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(eft2extck_c_2, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(mc1cmddir_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(mc1dat31dir_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(mc1dat2dir_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(wdgswrstout_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(u1rtscts_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(u1rxtx_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(usb1drvvbus_c_2, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(fsmcsmadq10to15_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(ethmdio_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(ethmdc_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(i2s2tx_c_2, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(i2s2_c_2, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(usb0drvvbus_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(can1_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(eft4extck_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(can2_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(eft3extck_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(eft3ocmp1_c_1, NMK_GPIO_ALT_C),
	STA1295_PIN_GROUP(eft4extck_c_2, NMK_GPIO_ALT_C),
};

/* We use this macro to define the groups applicable to a function */
#define STA1295_FUNC_GROUPS(a, b...)	   \
static const char * const a##_groups[] = { b };

STA1295_FUNC_GROUPS(audioss,
	"audrefclk_a_1");
STA1295_FUNC_GROUPS(can0,
	"can0_a_1");
STA1295_FUNC_GROUPS(can1,
	"can1_c_1");
STA1295_FUNC_GROUPS(can2,
	"can2_b_1", "can2_c_1");
STA1295_FUNC_GROUPS(clcd,
	"clcd_a_1");
STA1295_FUNC_GROUPS(clkout,
	"clkout1_b_1", "clkouthclk_b_1", "clkoutperiph0_b_1", "clkoutperiph1_b_1");
STA1295_FUNC_GROUPS(eft0,
	"eft0_a_1", "eft0extck_b_1", "eft0extck_b_2", "eft0ocmp0_b_1",
	"eft0ocmp1_b_1", "eft0icap1_c_1", "eft0icap0_c_1");
STA1295_FUNC_GROUPS(eft1,
	"eft1_a_1", "eft1extck_b_1", "eft1extck_b_2", "eft1ocmp1_b_1",
	"eft1ocmp0_b_1", "eft1icap0_c_1", "eft1icap1_c_1");
STA1295_FUNC_GROUPS(eft2,
	"eft2icap0_a_1", "eft2ocmp0_b_1", "eft2ocmp1_c_1", "eft2icap1_c_1",
	"eft2ocmp0_c_1", "eft2ocmp1_c_2", "eft2extck_c_1",
	"eft2icap0_c_1", "eft2icap1_c_2", "eft2extck_c_2");
STA1295_FUNC_GROUPS(eft3,
	"eft3icap1_a_1", "eft3icap0_a_1", "eft3ocmp0_a_1", "eft3icap1_a_2",
	"eft3ocmp0_b_1", "eft3ocmp1_b_1", "eft3extck_c_1",
	"eft3ocmp1_c_1");
STA1295_FUNC_GROUPS(eft4,
	"eft4icap0_a_1", "eft4icap1_a_1", "eft4ocmp0_b_1", "eft4ocmp0_b_2",
	"eft4icap0_b_1", "eft4ocmp1_b_1", "eft4icap1_b_1",
	"eft4ocmp1_b_2", "eft4icap1_b_2", "eft4ocmp1_b_3",
	"eft4extck_c_1", "eft4extck_c_2");
STA1295_FUNC_GROUPS(eth,
	"ethmdio_a_1", "ethmdc_a_1", "ethrmii_a_1", "ethrmiiclk_a_1",
	"ethmii_a_1", "ethmiitxclk_a_1", "ethmiirxclk_b_1", "ethrxer_b_1",
	"ethmdio_c_1", "ethmdc_c_1");
STA1295_FUNC_GROUPS(fsmc,
	"fsmcsmadq0to7_a_1", "fsmcwpn_a_1", "fsmcbusy_a_1", "fsmcoen_a_1",
	"fsmcwen_a_1", "fsmcsmad17ale_a_1", "fsmcsmad16cle_a_1",
	"fsmcnandcs0n_a_1", "fsmcdack_b_1", "fsmcdreq_b_1",
	"fsmcsmadq89_b_1", "fsmcadvn_b_1", "fsmccs0n_b_1",
	"fsmcdack_c_1", "fsmcdreq_c_1", "fsmcsmadq10to15_c_1");
STA1295_FUNC_GROUPS(i2c0,
	"i2c0_a_1", "i2c0_a_2");
STA1295_FUNC_GROUPS(i2c1,
	"i2c1_a_1", "i2c1_a_2", "i2c1_c_1");
STA1295_FUNC_GROUPS(i2c2,
	"i2c2_a_1", "i2c2_b_1", "i2c2_b_2", "i2c2_c_1");
STA1295_FUNC_GROUPS(i2s0,
	"i2s0_a_1", "i2s0tx_a_1");
STA1295_FUNC_GROUPS(i2s2,
	"i2s2tx_c_1", "i2s2_c_1", "i2s2tx_c_2", "i2s2_c_2");
STA1295_FUNC_GROUPS(mc0,
	"mc0data4to7_b_1", "mc0fbclk_b_1", "mc0pwr_b_1", "mc0data4to7_b_2");
STA1295_FUNC_GROUPS(mc1,
	"mc1_a_1", "mc1cmd_a_1", "mc1clk_a_1", "mc1dat0_a_1",
	"mc1_b_1", "mc1dat123_b_1", "mc1dat31dir_b_1",
	"mc1dat0dir_c_1", "mc1fbclk_c_1", "mc1data4to7_c_1",
	"mc1cmddir_c_1", "mc1dat31dir_c_1", "mc1dat2dir_c_1");
STA1295_FUNC_GROUPS(mc2,
	"mc2dat2dir_b_1", "mc2dat31dir_b_1", "mc2dat0dir_b_1", "mc2cmddir_b_1",
	"mc2dat31dir_b_2", "mc2_b_1", "mc2data4to7_b_1",
	"mc2fbclk_b_1", "mc2dat0dir_b_2", "mc2dat31dir_b_3");
STA1295_FUNC_GROUPS(sai1,
	"sai1_a_1");
STA1295_FUNC_GROUPS(sai2,
	"sai2_a_1", "sai2tx_b_1");
STA1295_FUNC_GROUPS(sai3,
	"sai3bclk_a_1", "sai3fs_a_1", "sai3rx2_a_1", "sai3tx1_a_1",
	"sai3tx2_a_1", "sai3rx1_a_1", "sai3tx0_a_1",
	"sai3rx0_a_1");
STA1295_FUNC_GROUPS(sai4,
	"sai4bclk_b_1", "sai4fs_b_1", "sai4tx0_b_1", "sai4rx0_b_1",
	"sai4bclk_b_2", "sai4fs_b_2", "sai4tx0_b_2",
	"sai4tx1_b_1", "sai4rx0_b_2", "sai4rx1_b_1",
	"sai4tx1_c_1", "sai4tx2_c_1", "sai4tx0_c_1",
	"sai4bclk_c_1", "sai4fs_c_1", "sai4rx0_c_1",
	"sai4rx2_c_1", "sai4rx1_c_1");
STA1295_FUNC_GROUPS(spdif,
	"spdifrx_b_1");
STA1295_FUNC_GROUPS(spi0,
	"spi0_a_1", "spi0ss_a_1", "spi0rx_b_1", "spi0tx_b_1");
STA1295_FUNC_GROUPS(spi1,
	"spi1ss_a_1", "spi1_a_1", "spi1_b_1", "spi1ss_b_1");
STA1295_FUNC_GROUPS(spi2,
	"spi2_b_1", "spi2_b_2", "spi2ss_b_1", "spi2ss_b_2",
	"spi2_b_3", "spi2ss_b_3", "spi2ss_c_1",
	"spi2_c_1");
STA1295_FUNC_GROUPS(sqi,
	"sqifdbsck_a_1", "sqice1n_a_1", "sqi_a_1");
STA1295_FUNC_GROUPS(u0,
	"u0rxtx_a_1", "u0rtscts_a_1", "u0rxtx_a_2", "u0rtscts_a_2");
STA1295_FUNC_GROUPS(u1,
	"u1rxtx_a_1", "u1rxtx_b_1", "u1rxtx_b_2", "u1rtscts_b_1",
	"u1rxtx_b_3", "u1rtscts_c_1", "u1rxtx_c_1");
STA1295_FUNC_GROUPS(u2,
	"u2rxtx_a_1", "u2rxtx_b_1", "u2rxtx_b_2", "u2rxtx_c_1");
STA1295_FUNC_GROUPS(u3,
	"u3rxtx_a_1", "u3rxtx_b_1", "u3rxtx_c_1");
STA1295_FUNC_GROUPS(usb,
		    "usb0drvvbus_b_1", "usb0drvvbus_c_1",
		    "usb1drvvbus_c_1", "usb1drvvbus_c_2");
STA1295_FUNC_GROUPS(vip,
	"vip_a_1");
STA1295_FUNC_GROUPS(wdg,
	"wdgswrstout_a_1", "wdgswrstout_b_1", "wdgswrstout_c_1");

#define FUNCTION(fname)					\
	{						\
		.name = #fname,				\
		.groups = fname##_groups,		\
		.ngroups = ARRAY_SIZE(fname##_groups),	\
	}

static const struct nmk_function nmk_sta1295_functions[] = {
	FUNCTION(audioss),
	FUNCTION(can0),
	FUNCTION(can1),
	FUNCTION(can2),
	FUNCTION(clcd),
	FUNCTION(clkout),
	FUNCTION(eft0),
	FUNCTION(eft1),
	FUNCTION(eft2),
	FUNCTION(eft3),
	FUNCTION(eft4),
	FUNCTION(eth),
	FUNCTION(fsmc),
	FUNCTION(i2c0),
	FUNCTION(i2c1),
	FUNCTION(i2c2),
	FUNCTION(i2s0),
	FUNCTION(i2s2),
	FUNCTION(mc0),
	FUNCTION(mc1),
	FUNCTION(mc2),
	FUNCTION(sai1),
	FUNCTION(sai2),
	FUNCTION(sai3),
	FUNCTION(sai4),
	FUNCTION(spdif),
	FUNCTION(spi0),
	FUNCTION(spi1),
	FUNCTION(spi2),
	FUNCTION(sqi),
	FUNCTION(u0),
	FUNCTION(u1),
	FUNCTION(u2),
	FUNCTION(u3),
	FUNCTION(usb),
	FUNCTION(vip),
	FUNCTION(wdg),
};

static const struct nmk_pinctrl_soc_data nmk_sta1295_soc = {
	.pins = nmk_sta1295_pins,
	.npins = ARRAY_SIZE(nmk_sta1295_pins),
	.functions = nmk_sta1295_functions,
	.nfunctions = ARRAY_SIZE(nmk_sta1295_functions),
	.groups = nmk_sta1295_groups,
	.ngroups = ARRAY_SIZE(nmk_sta1295_groups),
};

void nmk_pinctrl_sta1295_init(const struct nmk_pinctrl_soc_data **soc)
{
	*soc = &nmk_sta1295_soc;
}
