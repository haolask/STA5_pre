/**
 * Copyright (C) 2018 ST Microelectronics
 *
 * Jean-Nicolas Graux <jean-nicolas.graux@st.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 *
 * 2018-06-12 10:49:13: Automatic generation with pinout2pinctrl.py tool.
 */

#include <linux/kernel.h>
#include <linux/pinctrl/pinctrl.h>
#include "pinctrl-nomadik.h"

/* All the pins that can be used for GPIO and some other functions */
#define _GPIO(offset)		(offset)
#define S_GPIO(offset)		((offset) + 96)
#define M3_GPIO(offset)		((offset) + 128)

/* main gpios */

#define STA1385_PIN_K5		_GPIO(0)
#define STA1385_PIN_M4		_GPIO(1)
#define STA1385_PIN_R2		_GPIO(2)
#define STA1385_PIN_P2		_GPIO(3)
#define STA1385_PIN_L5		_GPIO(4)
#define STA1385_PIN_T2		_GPIO(5)
#define STA1385_PIN_P1		_GPIO(6)
#define STA1385_PIN_U2		_GPIO(7)
#define STA1385_PIN_L4		_GPIO(8)
#define STA1385_PIN_R1		_GPIO(9)
#define STA1385_PIN_T1		_GPIO(10)
#define STA1385_PIN_U1		_GPIO(11)
#define STA1385_PIN_J6		_GPIO(12)
#define STA1385_PIN_V1		_GPIO(13)
#define STA1385_PIN_K2		_GPIO(14)
#define STA1385_PIN_J2		_GPIO(15)
#define STA1385_PIN_J3		_GPIO(16)
#define STA1385_PIN_H1		_GPIO(17)
#define STA1385_PIN_K4		_GPIO(18)
#define STA1385_PIN_J1		_GPIO(19)
#define STA1385_PIN_K3		_GPIO(20)
#define STA1385_PIN_K1		_GPIO(21)
#define STA1385_PIN_F3		_GPIO(22)
#define STA1385_PIN_G6		_GPIO(23)
#define STA1385_PIN_E2		_GPIO(24)
#define STA1385_PIN_H6		_GPIO(25)
#define STA1385_PIN_D2		_GPIO(26)
#define STA1385_PIN_F4		_GPIO(27)
#define STA1385_PIN_E1		_GPIO(28)
#define STA1385_PIN_H5		_GPIO(29)
#define STA1385_PIN_G5		_GPIO(30)
#define STA1385_PIN_J5		_GPIO(31)
#define STA1385_PIN_F5		_GPIO(32)
#define STA1385_PIN_E3		_GPIO(33)
#define STA1385_PIN_D3		_GPIO(34)
#define STA1385_PIN_D1		_GPIO(35)
#define STA1385_PIN_F6		_GPIO(36)
#define STA1385_PIN_G4		_GPIO(37)
#define STA1385_PIN_A8		_GPIO(38)
#define STA1385_PIN_A4		_GPIO(39)
#define STA1385_PIN_A9		_GPIO(40)
#define STA1385_PIN_A5		_GPIO(41)
#define STA1385_PIN_A6		_GPIO(42)
#define STA1385_PIN_A14		_GPIO(43)
#define STA1385_PIN_A15		_GPIO(44)
#define STA1385_PIN_A7		_GPIO(45)
#define STA1385_PIN_A17		_GPIO(46)
#define STA1385_PIN_A16		_GPIO(47)
#define STA1385_PIN_A3		_GPIO(48)
#define STA1385_PIN_C10		_GPIO(49)
#define STA1385_PIN_B16		_GPIO(50)
#define STA1385_PIN_B18		_GPIO(51)
#define STA1385_PIN_C9		_GPIO(52)
#define STA1385_PIN_B17		_GPIO(53)
#define STA1385_PIN_A11		_GPIO(54)
#define STA1385_PIN_A12		_GPIO(55)
#define STA1385_PIN_E10		_GPIO(56)
#define STA1385_PIN_A18		_GPIO(57)
#define STA1385_PIN_B14		_GPIO(58)
#define STA1385_PIN_A10		_GPIO(59)
#define STA1385_PIN_B15		_GPIO(60)
#define STA1385_PIN_B8		_GPIO(61)
#define STA1385_PIN_B9		_GPIO(62)
#define STA1385_PIN_A13		_GPIO(63)
#define STA1385_PIN_B10		_GPIO(64)
#define STA1385_PIN_C2		_GPIO(65)
#define STA1385_PIN_B1		_GPIO(66)
#define STA1385_PIN_F10		_GPIO(67)
#define STA1385_PIN_C1		_GPIO(68)
#define STA1385_PIN_F9		_GPIO(69)
#define STA1385_PIN_A2		_GPIO(70)
#define STA1385_PIN_E9		_GPIO(71)
#define STA1385_PIN_B2		_GPIO(72)
#define STA1385_PIN_E7		_GPIO(73)
#define STA1385_PIN_C4		_GPIO(74)
#define STA1385_PIN_C3		_GPIO(75)
#define STA1385_PIN_C5		_GPIO(76)
#define STA1385_PIN_B3		_GPIO(77)
#define STA1385_PIN_B4		_GPIO(78)
#define STA1385_PIN_D6		_GPIO(79)
#define STA1385_PIN_B5		_GPIO(80)
#define STA1385_PIN_E6		_GPIO(81)
#define STA1385_PIN_F8		_GPIO(82)
#define STA1385_PIN_E5		_GPIO(83)
#define STA1385_PIN_D5		_GPIO(84)
#define STA1385_PIN_F7		_GPIO(85)
#define STA1385_PIN_D7		_GPIO(86)
#define STA1385_PIN_C6		_GPIO(87)

/* shared gpios */

#define STA1385_PIN_H4		S_GPIO(0)
#define STA1385_PIN_F2		S_GPIO(1)
#define STA1385_PIN_B6		S_GPIO(2)
#define STA1385_PIN_D9		S_GPIO(3)
#define STA1385_PIN_B7		S_GPIO(4)
#define STA1385_PIN_C7		S_GPIO(5)
#define STA1385_PIN_C8		S_GPIO(6)
#define STA1385_PIN_G3		S_GPIO(7)
#define STA1385_PIN_J4		S_GPIO(8)
#define STA1385_PIN_H2		S_GPIO(9)
#define STA1385_PIN_D10		S_GPIO(10)
#define STA1385_PIN_B19		S_GPIO(11)
#define STA1385_PIN_C17		S_GPIO(12)
#define STA1385_PIN_C18		S_GPIO(13)
#define STA1385_PIN_H3		S_GPIO(14)
#define STA1385_PIN_G2		S_GPIO(15)

/* m3 gpios */

#define STA1385_PIN_D8		M3_GPIO(0)
#define STA1385_PIN_E8		M3_GPIO(1)
#define STA1385_PIN_E18		M3_GPIO(2)
#define STA1385_PIN_G16		M3_GPIO(3)
#define STA1385_PIN_C19		M3_GPIO(4)
#define STA1385_PIN_G17		M3_GPIO(5)
#define STA1385_PIN_D18		M3_GPIO(6)
#define STA1385_PIN_F19		M3_GPIO(7)
#define STA1385_PIN_D17		M3_GPIO(8)
#define STA1385_PIN_E19		M3_GPIO(9)
#define STA1385_PIN_F18		M3_GPIO(10)
#define STA1385_PIN_F1		M3_GPIO(11)
#define STA1385_PIN_G1		M3_GPIO(12)
#define STA1385_PIN_V3		M3_GPIO(13)
#define STA1385_PIN_W3		M3_GPIO(14)
#define STA1385_PIN_D19		M3_GPIO(15)

/*
 * The names of the pins are denoted by GPIO number and ball name, even
 * though they can be used for other things than GPIO, this is the first
 * column in the table of the data sheet and often used on schematics and
 * such.
 */
static const struct pinctrl_pin_desc nmk_sta1385_pins[] = {
	PINCTRL_PIN(STA1385_PIN_K5,		"GPIO0_K5"),
	PINCTRL_PIN(STA1385_PIN_M4,		"GPIO1_M4"),
	PINCTRL_PIN(STA1385_PIN_R2,		"GPIO2_R2"),
	PINCTRL_PIN(STA1385_PIN_P2,		"GPIO3_P2"),
	PINCTRL_PIN(STA1385_PIN_L5,		"GPIO4_L5"),
	PINCTRL_PIN(STA1385_PIN_T2,		"GPIO5_T2"),
	PINCTRL_PIN(STA1385_PIN_P1,		"GPIO6_P1"),
	PINCTRL_PIN(STA1385_PIN_U2,		"GPIO7_U2"),
	PINCTRL_PIN(STA1385_PIN_L4,		"GPIO8_L4"),
	PINCTRL_PIN(STA1385_PIN_R1,		"GPIO9_R1"),
	PINCTRL_PIN(STA1385_PIN_T1,		"GPIO10_T1"),
	PINCTRL_PIN(STA1385_PIN_U1,		"GPIO11_U1"),
	PINCTRL_PIN(STA1385_PIN_J6,		"GPIO12_J6"),
	PINCTRL_PIN(STA1385_PIN_V1,		"GPIO13_V1"),
	PINCTRL_PIN(STA1385_PIN_K2,		"GPIO14_K2"),
	PINCTRL_PIN(STA1385_PIN_J2,		"GPIO15_J2"),
	PINCTRL_PIN(STA1385_PIN_J3,		"GPIO16_J3"),
	PINCTRL_PIN(STA1385_PIN_H1,		"GPIO17_H1"),
	PINCTRL_PIN(STA1385_PIN_K4,		"GPIO18_K4"),
	PINCTRL_PIN(STA1385_PIN_J1,		"GPIO19_J1"),
	PINCTRL_PIN(STA1385_PIN_K3,		"GPIO20_K3"),
	PINCTRL_PIN(STA1385_PIN_K1,		"GPIO21_K1"),
	PINCTRL_PIN(STA1385_PIN_F3,		"GPIO22_F3"),
	PINCTRL_PIN(STA1385_PIN_G6,		"GPIO23_G6"),
	PINCTRL_PIN(STA1385_PIN_E2,		"GPIO24_E2"),
	PINCTRL_PIN(STA1385_PIN_H6,		"GPIO25_H6"),
	PINCTRL_PIN(STA1385_PIN_D2,		"GPIO26_D2"),
	PINCTRL_PIN(STA1385_PIN_F4,		"GPIO27_F4"),
	PINCTRL_PIN(STA1385_PIN_E1,		"GPIO28_E1"),
	PINCTRL_PIN(STA1385_PIN_H5,		"GPIO29_H5"),
	PINCTRL_PIN(STA1385_PIN_G5,		"GPIO30_G5"),
	PINCTRL_PIN(STA1385_PIN_J5,		"GPIO31_J5"),
	PINCTRL_PIN(STA1385_PIN_F5,		"GPIO32_F5"),
	PINCTRL_PIN(STA1385_PIN_E3,		"GPIO33_E3"),
	PINCTRL_PIN(STA1385_PIN_D3,		"GPIO34_D3"),
	PINCTRL_PIN(STA1385_PIN_D1,		"GPIO35_D1"),
	PINCTRL_PIN(STA1385_PIN_F6,		"GPIO36_F6"),
	PINCTRL_PIN(STA1385_PIN_G4,		"GPIO37_G4"),
	PINCTRL_PIN(STA1385_PIN_A8,		"GPIO38_A8"),
	PINCTRL_PIN(STA1385_PIN_A4,		"GPIO39_A4"),
	PINCTRL_PIN(STA1385_PIN_A9,		"GPIO40_A9"),
	PINCTRL_PIN(STA1385_PIN_A5,		"GPIO41_A5"),
	PINCTRL_PIN(STA1385_PIN_A6,		"GPIO42_A6"),
	PINCTRL_PIN(STA1385_PIN_A14,		"GPIO43_A14"),
	PINCTRL_PIN(STA1385_PIN_A15,		"GPIO44_A15"),
	PINCTRL_PIN(STA1385_PIN_A7,		"GPIO45_A7"),
	PINCTRL_PIN(STA1385_PIN_A17,		"GPIO46_A17"),
	PINCTRL_PIN(STA1385_PIN_A16,		"GPIO47_A16"),
	PINCTRL_PIN(STA1385_PIN_A3,		"GPIO48_A3"),
	PINCTRL_PIN(STA1385_PIN_C10,		"GPIO49_C10"),
	PINCTRL_PIN(STA1385_PIN_B16,		"GPIO50_B16"),
	PINCTRL_PIN(STA1385_PIN_B18,		"GPIO51_B18"),
	PINCTRL_PIN(STA1385_PIN_C9,		"GPIO52_C9"),
	PINCTRL_PIN(STA1385_PIN_B17,		"GPIO53_B17"),
	PINCTRL_PIN(STA1385_PIN_A11,		"GPIO54_A11"),
	PINCTRL_PIN(STA1385_PIN_A12,		"GPIO55_A12"),
	PINCTRL_PIN(STA1385_PIN_E10,		"GPIO56_E10"),
	PINCTRL_PIN(STA1385_PIN_A18,		"GPIO57_A18"),
	PINCTRL_PIN(STA1385_PIN_B14,		"GPIO58_B14"),
	PINCTRL_PIN(STA1385_PIN_A10,		"GPIO59_A10"),
	PINCTRL_PIN(STA1385_PIN_B15,		"GPIO60_B15"),
	PINCTRL_PIN(STA1385_PIN_B8,		"GPIO61_B8"),
	PINCTRL_PIN(STA1385_PIN_B9,		"GPIO62_B9"),
	PINCTRL_PIN(STA1385_PIN_A13,		"GPIO63_A13"),
	PINCTRL_PIN(STA1385_PIN_B10,		"GPIO64_B10"),
	PINCTRL_PIN(STA1385_PIN_C2,		"GPIO65_C2"),
	PINCTRL_PIN(STA1385_PIN_B1,		"GPIO66_B1"),
	PINCTRL_PIN(STA1385_PIN_F10,		"GPIO67_F10"),
	PINCTRL_PIN(STA1385_PIN_C1,		"GPIO68_C1"),
	PINCTRL_PIN(STA1385_PIN_F9,		"GPIO69_F9"),
	PINCTRL_PIN(STA1385_PIN_A2,		"GPIO70_A2"),
	PINCTRL_PIN(STA1385_PIN_E9,		"GPIO71_E9"),
	PINCTRL_PIN(STA1385_PIN_B2,		"GPIO72_B2"),
	PINCTRL_PIN(STA1385_PIN_E7,		"GPIO73_E7"),
	PINCTRL_PIN(STA1385_PIN_C4,		"GPIO74_C4"),
	PINCTRL_PIN(STA1385_PIN_C3,		"GPIO75_C3"),
	PINCTRL_PIN(STA1385_PIN_C5,		"GPIO76_C5"),
	PINCTRL_PIN(STA1385_PIN_B3,		"GPIO77_B3"),
	PINCTRL_PIN(STA1385_PIN_B4,		"GPIO78_B4"),
	PINCTRL_PIN(STA1385_PIN_D6,		"GPIO79_D6"),
	PINCTRL_PIN(STA1385_PIN_B5,		"GPIO80_B5"),
	PINCTRL_PIN(STA1385_PIN_E6,		"GPIO81_E6"),
	PINCTRL_PIN(STA1385_PIN_F8,		"GPIO82_F8"),
	PINCTRL_PIN(STA1385_PIN_E5,		"GPIO83_E5"),
	PINCTRL_PIN(STA1385_PIN_D5,		"GPIO84_D5"),
	PINCTRL_PIN(STA1385_PIN_F7,		"GPIO85_F7"),
	PINCTRL_PIN(STA1385_PIN_D7,		"GPIO86_D7"),
	PINCTRL_PIN(STA1385_PIN_C6,		"GPIO87_C6"),
	PINCTRL_PIN(STA1385_PIN_H4,		"S_GPIO0_H4"),
	PINCTRL_PIN(STA1385_PIN_F2,		"S_GPIO1_F2"),
	PINCTRL_PIN(STA1385_PIN_B6,		"S_GPIO2_B6"),
	PINCTRL_PIN(STA1385_PIN_D9,		"S_GPIO3_D9"),
	PINCTRL_PIN(STA1385_PIN_B7,		"S_GPIO4_B7"),
	PINCTRL_PIN(STA1385_PIN_C7,		"S_GPIO5_C7"),
	PINCTRL_PIN(STA1385_PIN_C8,		"S_GPIO6_C8"),
	PINCTRL_PIN(STA1385_PIN_G3,		"S_GPIO7_G3"),
	PINCTRL_PIN(STA1385_PIN_J4,		"S_GPIO8_J4"),
	PINCTRL_PIN(STA1385_PIN_H2,		"S_GPIO9_H2"),
	PINCTRL_PIN(STA1385_PIN_D10,		"S_GPIO10_D10"),
	PINCTRL_PIN(STA1385_PIN_B19,		"S_GPIO11_B19"),
	PINCTRL_PIN(STA1385_PIN_C17,		"S_GPIO12_C17"),
	PINCTRL_PIN(STA1385_PIN_C18,		"S_GPIO13_C18"),
	PINCTRL_PIN(STA1385_PIN_H3,		"S_GPIO14_H3"),
	PINCTRL_PIN(STA1385_PIN_G2,		"S_GPIO15_G2"),
	PINCTRL_PIN(STA1385_PIN_D8,		"M3_GPIO0_D8"),
	PINCTRL_PIN(STA1385_PIN_E8,		"M3_GPIO1_E8"),
	PINCTRL_PIN(STA1385_PIN_E18,		"M3_GPIO2_E18"),
	PINCTRL_PIN(STA1385_PIN_G16,		"M3_GPIO3_G16"),
	PINCTRL_PIN(STA1385_PIN_C19,		"M3_GPIO4_C19"),
	PINCTRL_PIN(STA1385_PIN_G17,		"M3_GPIO5_G17"),
	PINCTRL_PIN(STA1385_PIN_D18,		"M3_GPIO6_D18"),
	PINCTRL_PIN(STA1385_PIN_F19,		"M3_GPIO7_F19"),
	PINCTRL_PIN(STA1385_PIN_D17,		"M3_GPIO8_D17"),
	PINCTRL_PIN(STA1385_PIN_E19,		"M3_GPIO9_E19"),
	PINCTRL_PIN(STA1385_PIN_F18,		"M3_GPIO10_F18"),
	PINCTRL_PIN(STA1385_PIN_F1,		"M3_GPIO11_F1"),
	PINCTRL_PIN(STA1385_PIN_G1,		"M3_GPIO12_G1"),
	PINCTRL_PIN(STA1385_PIN_V3,		"M3_GPIO13_V3"),
	PINCTRL_PIN(STA1385_PIN_W3,		"M3_GPIO14_W3"),
	PINCTRL_PIN(STA1385_PIN_D19,		"M3_GPIO15_D19"),
};

/*
 * Read the pin group names like this:
 * u0_a_1    = first group of pins for uart0 on alt function a
 * i2c2_b_2  = second group of pins for i2c2 on alt function b
 */

/* Altfunction A */

/* eth0_a_1 =
 * ETH0_TXEN, ETH0_TX[0], ETH0_TX[1], ETH0_TX[2],
 * ETH0_TX[3], ETH0_RXDV_CRS, ETH0_RX[0],
 * ETH0_RX[1], ETH0_RX[2], ETH0_RX[3],
 * ETH0_MDIO, ETH0_MDC, ETH0_RMII_CLK,
 * ETH0_MII_TX_CLK
 */
static const unsigned int eth0_a_1_pins[] = {
	STA1385_PIN_K5, STA1385_PIN_M4, STA1385_PIN_R2, STA1385_PIN_P2,
	STA1385_PIN_L5, STA1385_PIN_T2, STA1385_PIN_P1,
	STA1385_PIN_U2, STA1385_PIN_L4, STA1385_PIN_R1,
	STA1385_PIN_T1, STA1385_PIN_U1, STA1385_PIN_J6,
	STA1385_PIN_V1 };
/* eth0tx_a_1 =
 * ETH0_TXEN, ETH0_TX[0], ETH0_TX[1], ETH0_TX[2],
 * ETH0_TX[3]
 */
static const unsigned int eth0tx_a_1_pins[] = {
	STA1385_PIN_K5, STA1385_PIN_M4, STA1385_PIN_R2, STA1385_PIN_P2,
	STA1385_PIN_L5 };
/* eth0rx_a_1 =
 * ETH0_RXDV_CRS, ETH0_RX[0], ETH0_RX[1], ETH0_RX[2],
 * ETH0_RX[3]
 */
static const unsigned int eth0rx_a_1_pins[] = {
	STA1385_PIN_T2, STA1385_PIN_P1, STA1385_PIN_U2, STA1385_PIN_L4,
	STA1385_PIN_R1 };
/* eth0mdio_a_1 =
 * ETH0_MDIO
 */
static const unsigned int eth0mdio_a_1_pins[] = {
	STA1385_PIN_T1 };
/* eth0mdc_a_1 =
 * ETH0_MDC
 */
static const unsigned int eth0mdc_a_1_pins[] = {
	STA1385_PIN_U1 };
/* eth0rmiiclk_a_1 =
 * ETH0_RMII_CLK
 */
static const unsigned int eth0rmiiclk_a_1_pins[] = {
	STA1385_PIN_J6 };
/* eth0miitxclk_a_1 =
 * ETH0_MII_TX_CLK
 */
static const unsigned int eth0miitxclk_a_1_pins[] = {
	STA1385_PIN_V1 };
/* u2rxtx_a_1 =
 * UART2_RX, UART2_TX
 */
static const unsigned int u2rxtx_a_1_pins[] = {
	STA1385_PIN_K2, STA1385_PIN_J2 };
/* i2c2_a_1 =
 * I2C2_SCL, I2C2_SDA
 */
static const unsigned int i2c2_a_1_pins[] = {
	STA1385_PIN_J3, STA1385_PIN_H1 };
/* spi2_a_1 =
 * SPI2_SCK, SPI2_RXD, SPI2_TXD
 */
static const unsigned int spi2_a_1_pins[] = {
	STA1385_PIN_K4, STA1385_PIN_K3, STA1385_PIN_K1 };
/* spi2ss_a_1 =
 * SPI2_SS
 */
static const unsigned int spi2ss_a_1_pins[] = {
	STA1385_PIN_J1 };
/* mc0clk_a_1 =
 * SDMMC0_CLK
 */
static const unsigned int mc0clk_a_1_pins[] = {
	STA1385_PIN_F3 };
/* mc0cmd_a_1 =
 * SDMMC0_CMD
 */
static const unsigned int mc0cmd_a_1_pins[] = {
	STA1385_PIN_G6 };
/* mc0dat0to3_a_1 =
 * SDMMC0_DATA_0, SDMMC0_DATA_1, SDMMC0_DATA_2, SDMMC0_DATA_3
 */
static const unsigned int mc0dat0to3_a_1_pins[] = {
	STA1385_PIN_E2, STA1385_PIN_H6, STA1385_PIN_D2, STA1385_PIN_F4 };
/* mc0fbclk_a_1 =
 * SDMMC0_FBCLK
 */
static const unsigned int mc0fbclk_a_1_pins[] = {
	STA1385_PIN_E1 };
/* mc0pwr_a_1 =
 * SDMMC0_PWR
 */
static const unsigned int mc0pwr_a_1_pins[] = {
	STA1385_PIN_H5 };
/* u1rxtx_a_1 =
 * UART1_RX, UART1_TX
 */
static const unsigned int u1rxtx_a_1_pins[] = {
	STA1385_PIN_G5, STA1385_PIN_J5 };
/* u1rtscts_a_1 =
 * UART1_CTS, UART1_RTS
 */
static const unsigned int u1rtscts_a_1_pins[] = {
	STA1385_PIN_F5, STA1385_PIN_E3 };
/* i2s0_a_1 =
 * I2S0_BCLK, I2S0_FS, I2S0_TX, I2S0_RX
 */
static const unsigned int i2s0_a_1_pins[] = {
	STA1385_PIN_D3, STA1385_PIN_D1, STA1385_PIN_F6, STA1385_PIN_G4 };
/* mc1clk_a_1 =
 * SDMMC1_CLK
 */
static const unsigned int mc1clk_a_1_pins[] = {
	STA1385_PIN_A15 };
/* mc1cmd_a_1 =
 * SDMMC1_CMD
 */
static const unsigned int mc1cmd_a_1_pins[] = {
	STA1385_PIN_A9 };
/* mc1dat0to7_a_1 =
 * SDMMC1_DATA_2, SDMMC1_DATA_3, SDMMC1_DATA_0, SDMMC1_DATA_1,
 * SDMMC1_DATA_4, SDMMC1_DATA_5, SDMMC1_DATA_6,
 * SDMMC1_DATA_7
 */
static const unsigned int mc1dat0to7_a_1_pins[] = {
	STA1385_PIN_A8, STA1385_PIN_A4, STA1385_PIN_A5, STA1385_PIN_A6,
	STA1385_PIN_A7, STA1385_PIN_A17, STA1385_PIN_A16,
	STA1385_PIN_A3 };
/* mc1fbclk_a_1 =
 * SDMMC1_FBCLK
 */
static const unsigned int mc1fbclk_a_1_pins[] = {
	STA1385_PIN_A14 };
/* i2s1_a_1 =
 * I2S1_BCLK, I2S1_FS, I2S1_TX, I2S1_RX
 */
static const unsigned int i2s1_a_1_pins[] = {
	STA1385_PIN_C2, STA1385_PIN_B1, STA1385_PIN_F10, STA1385_PIN_C1 };
/* i2s0_a_2 =
 * I2S0_BCLK, I2S0_FS, I2S0_TX, I2S0_RX
 */
static const unsigned int i2s0_a_2_pins[] = {
	STA1385_PIN_F9, STA1385_PIN_A2, STA1385_PIN_E9, STA1385_PIN_B2 };
/* fsmcsmad16cle_a_1 =
 * FSMC_SMAD16/CLE
 */
static const unsigned int fsmcsmad16cle_a_1_pins[] = {
	STA1385_PIN_E7 };
/* fsmcsmad17ale_a_1 =
 * FSMC_SMAD17/ALE
 */
static const unsigned int fsmcsmad17ale_a_1_pins[] = {
	STA1385_PIN_C4 };
/* fsmcnandcs0n_a_1 =
 * FSMC_NAND_CS0N
 */
static const unsigned int fsmcnandcs0n_a_1_pins[] = {
	STA1385_PIN_C3 };
/* fsmcsmadq0to7_a_1 =
 * FSMC_SMADQ_0, FSMC_SMADQ_1, FSMC_SMADQ_2, FSMC_SMADQ_3,
 * FSMC_SMADQ_4, FSMC_SMADQ_5, FSMC_SMADQ_6,
 * FSMC_SMADQ_7
 */
static const unsigned int fsmcsmadq0to7_a_1_pins[] = {
	STA1385_PIN_C5, STA1385_PIN_B3, STA1385_PIN_B4, STA1385_PIN_D6,
	STA1385_PIN_B5, STA1385_PIN_E6, STA1385_PIN_F8,
	STA1385_PIN_E5 };
/* fsmcoen_a_1 =
 * FSMC_OEn
 */
static const unsigned int fsmcoen_a_1_pins[] = {
	STA1385_PIN_D5 };
/* fsmcbusy_a_1 =
 * FSMC_BUSYn
 */
static const unsigned int fsmcbusy_a_1_pins[] = {
	STA1385_PIN_F7 };
/* fsmcwen_a_1 =
 * FSMC_WEn
 */
static const unsigned int fsmcwen_a_1_pins[] = {
	STA1385_PIN_D7 };
/* fsmcwpn_a_1 =
 * FSMC_WPn
 */
static const unsigned int fsmcwpn_a_1_pins[] = {
	STA1385_PIN_C6 };
/* u1rxtx_a_2 =
 * UART1_RX, UART1_TX
 */
static const unsigned int u1rxtx_a_2_pins[] = {
	STA1385_PIN_H4, STA1385_PIN_F2 };
/* eft1_a_1 =
 * EFT1_EXTCK, EFT1_ICAP1, EFT1_OCMP0, EFT1_OCMP1
 */
static const unsigned int eft1_a_1_pins[] = {
	STA1385_PIN_B6, STA1385_PIN_D9, STA1385_PIN_B7, STA1385_PIN_C7 };
/* i2c1_a_1 =
 * I2C1_SCL, I2C1_SDA
 */
static const unsigned int i2c1_a_1_pins[] = {
	STA1385_PIN_C8, STA1385_PIN_G3 };
/* can1_a_1 =
 * CAN1_RX, CAN1_TX
 */
static const unsigned int can1_a_1_pins[] = {
	STA1385_PIN_J4, STA1385_PIN_H2 };
/* spi1_a_1 =
 * SPI1_SCK, SPI1_RXD, SPI1_TXD
 */
static const unsigned int spi1_a_1_pins[] = {
	STA1385_PIN_D10, STA1385_PIN_C17, STA1385_PIN_C18 };
/* spi1sck_a_1 =
 * SPI1_SCK
 */
static const unsigned int spi1sck_a_1_pins[] = {
	STA1385_PIN_D10 };
/* spi1rxtx_a_1 =
 * SPI1_RXD, SPI1_TXD
 */
static const unsigned int spi1rxtx_a_1_pins[] = {
	STA1385_PIN_C17, STA1385_PIN_C18 };
/* spi1ss_a_1 =
 * SPI1_SS
 */
static const unsigned int spi1ss_a_1_pins[] = {
	STA1385_PIN_B19 };
/* can2_a_1 =
 * CAN2_RX, CAN2_TX
 */
static const unsigned int can2_a_1_pins[] = {
	STA1385_PIN_H3, STA1385_PIN_G2 };
/* i2c0_a_1 =
 * I2C0_SCL, I2C0_SDA
 */
static const unsigned int i2c0_a_1_pins[] = {
	STA1385_PIN_D8, STA1385_PIN_E8 };
/* eft0_a_1 =
 * EFT0_EXTCK, EFT0_ICAP0, EFT0_OCMP0, EFT0_ICAP1,
 * EFT0_OCMP1
 */
static const unsigned int eft0_a_1_pins[] = {
	STA1385_PIN_E18, STA1385_PIN_G16, STA1385_PIN_C19, STA1385_PIN_G17,
	STA1385_PIN_D18 };
/* spi0_a_1 =
 * SPI0_SCK, SPI0_RXD, SPI0_TXD
 */
static const unsigned int spi0_a_1_pins[] = {
	STA1385_PIN_F19, STA1385_PIN_E19, STA1385_PIN_F18 };
/* spi0ss_a_1 =
 * SPI0_SS
 */
static const unsigned int spi0ss_a_1_pins[] = {
	STA1385_PIN_D17 };
/* u0rxtx_a_1 =
 * UART0_RX, UART0_TX
 */
static const unsigned int u0rxtx_a_1_pins[] = {
	STA1385_PIN_F1, STA1385_PIN_G1 };
/* u0rtscts_a_1 =
 * UART0_CTS, UART0_RTS
 */
static const unsigned int u0rtscts_a_1_pins[] = {
	STA1385_PIN_V3, STA1385_PIN_W3 };
/* clkout0_a_1 =
 * CLOCKOUT0
 */
static const unsigned int clkout0_a_1_pins[] = {
	STA1385_PIN_D19 };

/* Altfunction B */

/* fra_b_1 =
 * FR_A_RX, FR_A_TX, FR_A_TX_EN_N
 */
static const unsigned int fra_b_1_pins[] = {
	STA1385_PIN_K5, STA1385_PIN_M4, STA1385_PIN_R2 };
/* frdbg_b_1 =
 * FR_DBG[1], FR_DBG[2], FR_DBG[3], FR_DBG[0]
 */
static const unsigned int frdbg_b_1_pins[] = {
	STA1385_PIN_P2, STA1385_PIN_L5, STA1385_PIN_T2, STA1385_PIN_R1 };
/* frb_b_1 =
 * FR_B_TX, FR_B_RX, FR_B_TX_EN
 */
static const unsigned int frb_b_1_pins[] = {
	STA1385_PIN_P1, STA1385_PIN_U2, STA1385_PIN_L4 };
/* i2c2_b_1 =
 * I2C2_SDA, I2C2_SCL
 */
static const unsigned int i2c2_b_1_pins[] = {
	STA1385_PIN_T1, STA1385_PIN_U1 };
/* usb0drvvbus_b_1 =
 * USB0_DRVVBUS
 */
static const unsigned int usb0drvvbus_b_1_pins[] = {
	STA1385_PIN_K2 };
/* usb1drvvbus_b_1 =
 * USB1_DRVVBUS
 */
static const unsigned int usb1drvvbus_b_1_pins[] = {
	STA1385_PIN_J2 };
/* usb0drvvbus_b_2 =
 * USB0_DRVVBUS
 */
static const unsigned int usb0drvvbus_b_2_pins[] = {
	STA1385_PIN_J3 };
/* usb1drvvbus_b_2 =
 * USB1_DRVVBUS
 */
static const unsigned int usb1drvvbus_b_2_pins[] = {
	STA1385_PIN_H1 };
/* i2c2_b_2 =
 * I2C2_SCL, I2C2_SDA
 */
static const unsigned int i2c2_b_2_pins[] = {
	STA1385_PIN_K4, STA1385_PIN_J1 };
/* u3rxtx_b_1 =
 * UART3_RX, UART3_TX
 */
static const unsigned int u3rxtx_b_1_pins[] = {
	STA1385_PIN_K3, STA1385_PIN_K1 };
/* mc0dat4to7_b_1 =
 * SDMMC0_DATA_4, SDMMC0_DATA_5, SDMMC0_DATA_6, SDMMC0_DATA_7
 */
static const unsigned int mc0dat4to7_b_1_pins[] = {
	STA1385_PIN_G5, STA1385_PIN_J5, STA1385_PIN_F5, STA1385_PIN_E3 };
/* u1rxtx_b_1 =
 * UART1_RX, UART1_TX
 */
static const unsigned int u1rxtx_b_1_pins[] = {
	STA1385_PIN_D3, STA1385_PIN_D1 };
/* u1rtscts_b_1 =
 * UART1_CTS, UART1_RTS
 */
static const unsigned int u1rtscts_b_1_pins[] = {
	STA1385_PIN_F6, STA1385_PIN_G4 };
/* eth1_b_1 =
 * ETH1_TXEN, ETH1_TX[0], ETH1_TX[1], ETH1_MDIO,
 * ETH1_MDC, ETH1_MII_TX_CLK, ETH1_RMII_CLK,
 * ETH1_RXDV_CRS, ETH1_RX[0], ETH1_RX[1]
 */
static const unsigned int eth1_b_1_pins[] = {
	STA1385_PIN_A8, STA1385_PIN_A4, STA1385_PIN_A9, STA1385_PIN_A5,
	STA1385_PIN_A6, STA1385_PIN_A14, STA1385_PIN_A15,
	STA1385_PIN_A7, STA1385_PIN_A17, STA1385_PIN_A16 };
/* eth1rxtx_b_1 =
 * ETH1_TXEN, ETH1_TX[0], ETH1_TX[1], ETH1_RXDV_CRS,
 * ETH1_RX[0], ETH1_RX[1]
 */
static const unsigned int eth1rxtx_b_1_pins[] = {
	STA1385_PIN_A8, STA1385_PIN_A4, STA1385_PIN_A9, STA1385_PIN_A7,
	STA1385_PIN_A17, STA1385_PIN_A16 };
/* eth1mdio_b_1 =
 * ETH1_MDIO
 */
static const unsigned int eth1mdio_b_1_pins[] = {
	STA1385_PIN_A5 };
/* eth1mdc_b_1 =
 * ETH1_MDC
 */
static const unsigned int eth1mdc_b_1_pins[] = {
	STA1385_PIN_A6 };
/* eth1rmiiclk_b_1 =
 * ETH1_RMII_CLK
 */
static const unsigned int eth1rmiiclk_b_1_pins[] = {
	STA1385_PIN_A15 };
/* eth1miitxclk_b_1 =
 * ETH1_MII_TX_CLK
 */
static const unsigned int eth1miitxclk_b_1_pins[] = {
	STA1385_PIN_A14 };
/* i2s2_b_1 =
 * I2S2_BCLK, I2S2_FS, I2S2_TX, I2S2_RX
 */
static const unsigned int i2s2_b_1_pins[] = {
	STA1385_PIN_C10, STA1385_PIN_B16, STA1385_PIN_B18, STA1385_PIN_C9 };
/* u4rxtx_b_1 =
 * UART4_RX, UART4_TX
 */
static const unsigned int u4rxtx_b_1_pins[] = {
	STA1385_PIN_B14, STA1385_PIN_A10 };
/* u2rxtx_b_1 =
 * UART2_RX, UART2_TX
 */
static const unsigned int u2rxtx_b_1_pins[] = {
	STA1385_PIN_B15, STA1385_PIN_B8 };
/* clkinref_b_1 =
 * CLOCKINREF
 */
static const unsigned int clkinref_b_1_pins[] = {
	STA1385_PIN_B9 };
/* u5rxtx_b_1 =
 * UART5_RX, UART5_TX
 */
static const unsigned int u5rxtx_b_1_pins[] = {
	STA1385_PIN_A13, STA1385_PIN_B10 };
/* eft2_b_1 =
 * EFT2_EXTCK, EFT2_ICAP0, EFT2_ICAP1, EFT2_OCMP0
 */
static const unsigned int eft2_b_1_pins[] = {
	STA1385_PIN_C2, STA1385_PIN_B1, STA1385_PIN_F10, STA1385_PIN_C1 };
/* eft3_b_1 =
 * EFT3_EXTCK, EFT3_ICAP0, EFT3_OCMP0, EFT3_OCMP1
 */
static const unsigned int eft3_b_1_pins[] = {
	STA1385_PIN_F9, STA1385_PIN_A2, STA1385_PIN_E9, STA1385_PIN_B2 };
/* sqifdbsck_b_1 =
 * SQI_FDBSCK
 */
static const unsigned int sqifdbsck_b_1_pins[] = {
	STA1385_PIN_E7 };
/* sqice0n_b_1 =
 * SQI_CE0n
 */
static const unsigned int sqice0n_b_1_pins[] = {
	STA1385_PIN_C4 };
/* fsmcnandcs0n_b_1 =
 * FSMC_NAND_CS0N
 */
static const unsigned int fsmcnandcs0n_b_1_pins[] = {
	STA1385_PIN_C5 };
/* sqi_b_1 =
 * SQI_SCK, SQI_SIO0, SQI_SIO1, SQI_SIO2,
 * SQI_SIO3
 */
static const unsigned int sqi_b_1_pins[] = {
	STA1385_PIN_C3, STA1385_PIN_B4, STA1385_PIN_D6, STA1385_PIN_B5,
	STA1385_PIN_E6 };
/* u3rxtx_b_2 =
 * UART3_RX, UART3_TX
 */
static const unsigned int u3rxtx_b_2_pins[] = {
	STA1385_PIN_F8, STA1385_PIN_E5 };
/* i2c2_b_3 =
 * I2C2_SCL, I2C2_SDA
 */
static const unsigned int i2c2_b_3_pins[] = {
	STA1385_PIN_D5, STA1385_PIN_F7 };
/* u2rxtx_b_2 =
 * UART2_RX, UART2_TX
 */
static const unsigned int u2rxtx_b_2_pins[] = {
	STA1385_PIN_D7, STA1385_PIN_C6 };
/* u1rtscts_b_2 =
 * UART1_CTS, UART1_RTS
 */
static const unsigned int u1rtscts_b_2_pins[] = {
	STA1385_PIN_B6, STA1385_PIN_D9 };
/* u4rxtx_b_2 =
 * UART4_RX, UART4_TX
 */
static const unsigned int u4rxtx_b_2_pins[] = {
	STA1385_PIN_H3, STA1385_PIN_G2 };
/* wdgswrstout_b_1 =
 * WDG_SW_RSTOUT
 */
static const unsigned int wdgswrstout_b_1_pins[] = {
	STA1385_PIN_C19 };
/* wdgswrstout_b_2 =
 * WDG_SW_RSTOUT
 */
static const unsigned int wdgswrstout_b_2_pins[] = {
	STA1385_PIN_D18 };
/* spi0rxtx_b_1 =
 * SPI0_RXD, SPI0_TXD
 */
static const unsigned int spi0rxtx_b_1_pins[] = {
	STA1385_PIN_F18, STA1385_PIN_E19 };
/* fsmcsmad16cle_b_1 =
 * FSMC_SMAD16/CLE
 */
static const unsigned int fsmcsmad16cle_b_1_pins[] = {
	STA1385_PIN_V3 };
/* fsmcsmad17ale_b_1 =
 * FSMC_SMAD17/ALE
 */
static const unsigned int fsmcsmad17ale_b_1_pins[] = {
	STA1385_PIN_W3 };

/* Altfunction C */

/* sqice0n_c_1 =
 * SQI_CE0n
 */
static const unsigned int sqice0n_c_1_pins[] = {
	STA1385_PIN_L4 };
/* sqisio0_c_1 =
 * SQI_SIO0
 */
static const unsigned int sqisio0_c_1_pins[] = {
	STA1385_PIN_R1 };
/* sqifdbsck_c_1 =
 * SQI_FDBSCK
 */
static const unsigned int sqifdbsck_c_1_pins[] = {
	STA1385_PIN_J6 };
/* sqisck_c_1 =
 * SQI_SCK
 */
static const unsigned int sqisck_c_1_pins[] = {
	STA1385_PIN_V1 };
/* u4rxtx_c_1 =
 * UART4_RX, UART4_TX
 */
static const unsigned int u4rxtx_c_1_pins[] = {
	STA1385_PIN_K2, STA1385_PIN_J2 };
/* u5rxtx_c_1 =
 * UART5_RX, UART5_TX
 */
static const unsigned int u5rxtx_c_1_pins[] = {
	STA1385_PIN_J3, STA1385_PIN_H1 };
/* eft2_c_1 =
 * EFT2_EXTCK, EFT2_ICAP0, EFT2_ICAP1, EFT2_OCMP0,
 * EFT2_OCMP1
 */
static const unsigned int eft2_c_1_pins[] = {
	STA1385_PIN_F3, STA1385_PIN_G6, STA1385_PIN_E2, STA1385_PIN_H6,
	STA1385_PIN_D2 };
/* eft3_c_1 =
 * EFT3_EXTCK, EFT3_ICAP0, EFT3_OCMP0, EFT3_OCMP1
 */
static const unsigned int eft3_c_1_pins[] = {
	STA1385_PIN_F4, STA1385_PIN_E1, STA1385_PIN_H5, STA1385_PIN_G5 };
/* eft4_c_1 =
 * EFT4_EXTCK, EFT4_ICAP0, EFT4_ICAP1, EFT4_OCMP1,
 * EFT4_OCMP0, EFT4_ICAP0
 */
static const unsigned int eft4_c_1_pins[] = {
	STA1385_PIN_J5, STA1385_PIN_F5, STA1385_PIN_E3, STA1385_PIN_D3,
	STA1385_PIN_D1, STA1385_PIN_F6 };
/* clkinref_c_1 =
 * CLOCKINREF
 */
static const unsigned int clkinref_c_1_pins[] = {
	STA1385_PIN_G4 };
/* mc1dat31dir_c_1 =
 * SDMMC1_DAT31_DIR
 */
static const unsigned int mc1dat31dir_c_1_pins[] = {
	STA1385_PIN_A7 };
/* mc1cmddir_c_1 =
 * SDMMC1_CMDDIR
 */
static const unsigned int mc1cmddir_c_1_pins[] = {
	STA1385_PIN_A17 };
/* mc1dat0dir_c_1 =
 * SDMMC1_DAT0_DIR
 */
static const unsigned int mc1dat0dir_c_1_pins[] = {
	STA1385_PIN_A16 };
/* mc1dat2dir_c_1 =
 * SDMMC1_DAT2_DIR
 */
static const unsigned int mc1dat2dir_c_1_pins[] = {
	STA1385_PIN_A3 };
/* eth1_c_1 =
 * ETH1_MII_TX_CLK, ETH1_RX[0], ETH1_RX[1], ETH1_RXDV_CRS,
 * ETH1_TX[0], ETH1_TX[1], ETH1_TXEN,
 * ETH1_CLOCKREF, ETH1_RX[2], ETH1_RX[3],
 * ETH1_TX[2], ETH1_TX[3], ETH1_RMII_CLK,
 * ETH1_MDIO, ETH1_MDC
 */
static const unsigned int eth1_c_1_pins[] = {
	STA1385_PIN_C10, STA1385_PIN_B16, STA1385_PIN_B18, STA1385_PIN_C9,
	STA1385_PIN_B17, STA1385_PIN_A11, STA1385_PIN_A12,
	STA1385_PIN_E10, STA1385_PIN_A18, STA1385_PIN_B14,
	STA1385_PIN_A10, STA1385_PIN_B15, STA1385_PIN_B9,
	STA1385_PIN_A13, STA1385_PIN_B10 };
/* eth1rxtx_c_1 =
 * ETH1_RX[2], ETH1_RX[3], ETH1_TX[2], ETH1_TX[3]
 */
static const unsigned int eth1rxtx_c_1_pins[] = {
	STA1385_PIN_A18, STA1385_PIN_B14, STA1385_PIN_A10, STA1385_PIN_B15 };
/* u2rxtx_c_1 =
 * UART2_TX, UART2_RX
 */
static const unsigned int u2rxtx_c_1_pins[] = {
	STA1385_PIN_F9, STA1385_PIN_A2 };
/* dmareqack_c_1 =
 * DMA_REQ, DMA_ACK
 */
static const unsigned int dmareqack_c_1_pins[] = {
	STA1385_PIN_E9, STA1385_PIN_B2 };
/* mc0clk_c_1 =
 * SDMMC0_CLK
 */
static const unsigned int mc0clk_c_1_pins[] = {
	STA1385_PIN_E7 };
/* mc0cmd_c_1 =
 * SDMMC0_CMD
 */
static const unsigned int mc0cmd_c_1_pins[] = {
	STA1385_PIN_C4 };
/* mc0dat0to3_c_1 =
 * SDMMC0_DATA_0, SDMMC0_DATA_1, SDMMC0_DATA_2, SDMMC0_DATA_3
 */
static const unsigned int mc0dat0to3_c_1_pins[] = {
	STA1385_PIN_C5, STA1385_PIN_B3, STA1385_PIN_B4, STA1385_PIN_D6 };
/* mc0dat4to7_c_1 =
 * SDMMC0_DATA_4, SDMMC0_DATA_5, SDMMC0_DATA_6, SDMMC0_DATA_7
 */
static const unsigned int mc0dat4to7_c_1_pins[] = {
	STA1385_PIN_B5, STA1385_PIN_E6, STA1385_PIN_F8, STA1385_PIN_E5 };
/* mc0pwr_c_1 =
 * SDMMC0_PWR
 */
static const unsigned int mc0pwr_c_1_pins[] = {
	STA1385_PIN_D5 };
/* mc0cmd_c_2 =
 * SDMMC0_CMD
 */
static const unsigned int mc0cmd_c_2_pins[] = {
	STA1385_PIN_F7 };
/* mc0fbclk_c_1 =
 * SDMMC0_FBCLK
 */
static const unsigned int mc0fbclk_c_1_pins[] = {
	STA1385_PIN_C6 };
/* mc0dat0to3_c_2 =
 * SDMMC0_DATA_0, SDMMC0_DATA_1, SDMMC0_DATA_2, SDMMC0_DATA_3
 */
static const unsigned int mc0dat0to3_c_2_pins[] = {
	STA1385_PIN_C5, STA1385_PIN_B3, STA1385_PIN_C3, STA1385_PIN_D7 };
/* mc0dat4to7_c_2 =
 * SDMMC0_DATA_4, SDMMC0_DATA_5, SDMMC0_DATA_6, SDMMC0_DATA_7
 */
static const unsigned int mc0dat4to7_c_2_pins[] = {
	STA1385_PIN_C8, STA1385_PIN_G3, STA1385_PIN_F8, STA1385_PIN_E5 };
/* u3rxtx_c_1 =
 * UART3_RX, UART3_TX
 */
static const unsigned int u3rxtx_c_1_pins[] = {
	STA1385_PIN_H4, STA1385_PIN_F2 };
/* u5rxtx_c_2 =
 * UART5_RX, UART5_TX
 */
static const unsigned int u5rxtx_c_2_pins[] = {
	STA1385_PIN_B6, STA1385_PIN_D9 };
/* eft1icap0_c_1 =
 * EFT1_ICAP0
 */
static const unsigned int eft1icap0_c_1_pins[] = {
	STA1385_PIN_C7 };
/* spi1rxtx_c_1 =
 * SPI1_RXD, SPI1_TXD
 */
static const unsigned int spi1rxtx_c_1_pins[] = {
	STA1385_PIN_C18, STA1385_PIN_C17 };
#define STA1385_PIN_GROUP(a, b) { .name = #a, .pins = a##_pins,	\
	.npins = ARRAY_SIZE(a##_pins), .altsetting = b }

static const struct nmk_pingroup nmk_sta1385_groups[] = {
	STA1385_PIN_GROUP(eth0_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(eth0tx_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(eth0rx_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(eth0mdio_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(eth0mdc_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(eth0rmiiclk_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(eth0miitxclk_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(u2rxtx_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(i2c2_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(spi2_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(spi2ss_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(mc0clk_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(mc0cmd_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(mc0dat0to3_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(mc0fbclk_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(mc0pwr_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(u1rxtx_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(u1rtscts_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(i2s0_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(mc1clk_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(mc1cmd_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(mc1dat0to7_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(mc1fbclk_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(i2s1_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(i2s0_a_2, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(fsmcsmad16cle_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(fsmcsmad17ale_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(fsmcnandcs0n_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(fsmcsmadq0to7_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(fsmcoen_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(fsmcbusy_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(fsmcwen_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(fsmcwpn_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(u1rxtx_a_2, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(eft1_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(i2c1_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(can1_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(spi1_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(spi1sck_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(spi1rxtx_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(spi1ss_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(can2_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(i2c0_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(eft0_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(spi0_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(spi0ss_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(u0rxtx_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(u0rtscts_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(clkout0_a_1, NMK_GPIO_ALT_A),
	STA1385_PIN_GROUP(fra_b_1, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(frdbg_b_1, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(frb_b_1, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(i2c2_b_1, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(usb0drvvbus_b_1, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(usb1drvvbus_b_1, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(usb0drvvbus_b_2, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(usb1drvvbus_b_2, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(i2c2_b_2, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(u3rxtx_b_1, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(mc0dat4to7_b_1, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(u1rxtx_b_1, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(u1rtscts_b_1, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(eth1_b_1, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(eth1rxtx_b_1, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(eth1mdio_b_1, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(eth1mdc_b_1, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(eth1rmiiclk_b_1, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(eth1miitxclk_b_1, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(i2s2_b_1, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(u4rxtx_b_1, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(u2rxtx_b_1, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(clkinref_b_1, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(u5rxtx_b_1, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(eft2_b_1, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(eft3_b_1, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(sqifdbsck_b_1, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(sqice0n_b_1, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(fsmcnandcs0n_b_1, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(sqi_b_1, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(u3rxtx_b_2, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(i2c2_b_3, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(u2rxtx_b_2, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(u1rtscts_b_2, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(u4rxtx_b_2, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(wdgswrstout_b_1, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(wdgswrstout_b_2, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(spi0rxtx_b_1, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(fsmcsmad16cle_b_1, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(fsmcsmad17ale_b_1, NMK_GPIO_ALT_B),
	STA1385_PIN_GROUP(sqice0n_c_1, NMK_GPIO_ALT_C),
	STA1385_PIN_GROUP(sqisio0_c_1, NMK_GPIO_ALT_C),
	STA1385_PIN_GROUP(sqifdbsck_c_1, NMK_GPIO_ALT_C),
	STA1385_PIN_GROUP(sqisck_c_1, NMK_GPIO_ALT_C),
	STA1385_PIN_GROUP(u4rxtx_c_1, NMK_GPIO_ALT_C),
	STA1385_PIN_GROUP(u5rxtx_c_1, NMK_GPIO_ALT_C),
	STA1385_PIN_GROUP(eft2_c_1, NMK_GPIO_ALT_C),
	STA1385_PIN_GROUP(eft3_c_1, NMK_GPIO_ALT_C),
	STA1385_PIN_GROUP(eft4_c_1, NMK_GPIO_ALT_C),
	STA1385_PIN_GROUP(clkinref_c_1, NMK_GPIO_ALT_C),
	STA1385_PIN_GROUP(mc1dat31dir_c_1, NMK_GPIO_ALT_C),
	STA1385_PIN_GROUP(mc1cmddir_c_1, NMK_GPIO_ALT_C),
	STA1385_PIN_GROUP(mc1dat0dir_c_1, NMK_GPIO_ALT_C),
	STA1385_PIN_GROUP(mc1dat2dir_c_1, NMK_GPIO_ALT_C),
	STA1385_PIN_GROUP(eth1_c_1, NMK_GPIO_ALT_C),
	STA1385_PIN_GROUP(eth1rxtx_c_1, NMK_GPIO_ALT_C),
	STA1385_PIN_GROUP(u2rxtx_c_1, NMK_GPIO_ALT_C),
	STA1385_PIN_GROUP(dmareqack_c_1, NMK_GPIO_ALT_C),
	STA1385_PIN_GROUP(mc0clk_c_1, NMK_GPIO_ALT_C),
	STA1385_PIN_GROUP(mc0cmd_c_1, NMK_GPIO_ALT_C),
	STA1385_PIN_GROUP(mc0dat0to3_c_1, NMK_GPIO_ALT_C),
	STA1385_PIN_GROUP(mc0dat4to7_c_1, NMK_GPIO_ALT_C),
	STA1385_PIN_GROUP(mc0pwr_c_1, NMK_GPIO_ALT_C),
	STA1385_PIN_GROUP(mc0cmd_c_2, NMK_GPIO_ALT_C),
	STA1385_PIN_GROUP(mc0fbclk_c_1, NMK_GPIO_ALT_C),
	STA1385_PIN_GROUP(mc0dat0to3_c_2, NMK_GPIO_ALT_C),
	STA1385_PIN_GROUP(mc0dat4to7_c_2, NMK_GPIO_ALT_C),
	STA1385_PIN_GROUP(u3rxtx_c_1, NMK_GPIO_ALT_C),
	STA1385_PIN_GROUP(u5rxtx_c_2, NMK_GPIO_ALT_C),
	STA1385_PIN_GROUP(eft1icap0_c_1, NMK_GPIO_ALT_C),
	STA1385_PIN_GROUP(spi1rxtx_c_1, NMK_GPIO_ALT_C),
};

/* We use this macro to define the groups applicable to a function */
#define STA1385_FUNC_GROUPS(a, b...)	   \
static const char * const a##_groups[] = { b };

STA1385_FUNC_GROUPS(can1,
	"can1_a_1");
STA1385_FUNC_GROUPS(can2,
	"can2_a_1");
STA1385_FUNC_GROUPS(clkinref,
	"clkinref_b_1", "clkinref_c_1");
STA1385_FUNC_GROUPS(clkout,
	"clkout0_a_1");
STA1385_FUNC_GROUPS(dma,
	"dmareqack_c_1");
STA1385_FUNC_GROUPS(eft0,
	"eft0_a_1");
STA1385_FUNC_GROUPS(eft1,
	"eft1_a_1", "eft1icap0_c_1");
STA1385_FUNC_GROUPS(eft2,
	"eft2_b_1", "eft2_c_1");
STA1385_FUNC_GROUPS(eft3,
	"eft3_b_1", "eft3_c_1");
STA1385_FUNC_GROUPS(eft4,
	"eft4_c_1");
STA1385_FUNC_GROUPS(eth0,
	"eth0_a_1", "eth0tx_a_1", "eth0rx_a_1", "eth0mdio_a_1",
	"eth0mdc_a_1", "eth0rmiiclk_a_1", "eth0miitxclk_a_1");
STA1385_FUNC_GROUPS(eth1,
	"eth1_b_1", "eth1rxtx_b_1", "eth1mdio_b_1", "eth1mdc_b_1",
	"eth1rmiiclk_b_1", "eth1miitxclk_b_1", "eth1_c_1",
	"eth1rxtx_c_1");
STA1385_FUNC_GROUPS(fra,
	"fra_b_1");
STA1385_FUNC_GROUPS(frb,
	"frb_b_1");
STA1385_FUNC_GROUPS(frdbg,
	"frdbg_b_1");
STA1385_FUNC_GROUPS(fsmc,
	"fsmcsmad16cle_a_1", "fsmcsmad17ale_a_1", "fsmcnandcs0n_a_1", "fsmcsmadq0to7_a_1",
	"fsmcoen_a_1", "fsmcbusy_a_1", "fsmcwen_a_1",
	"fsmcwpn_a_1", "fsmcnandcs0n_b_1", "fsmcsmad16cle_b_1",
	"fsmcsmad17ale_b_1");
STA1385_FUNC_GROUPS(i2c0,
	"i2c0_a_1");
STA1385_FUNC_GROUPS(i2c1,
	"i2c1_a_1");
STA1385_FUNC_GROUPS(i2c2,
	"i2c2_a_1", "i2c2_b_1", "i2c2_b_2", "i2c2_b_3");
STA1385_FUNC_GROUPS(i2s0,
	"i2s0_a_1", "i2s0_a_2");
STA1385_FUNC_GROUPS(i2s1,
	"i2s1_a_1");
STA1385_FUNC_GROUPS(i2s2,
	"i2s2_b_1");
STA1385_FUNC_GROUPS(mc0,
	"mc0clk_a_1", "mc0cmd_a_1", "mc0dat0to3_a_1", "mc0fbclk_a_1",
	"mc0pwr_a_1", "mc0dat4to7_b_1", "mc0clk_c_1",
	"mc0cmd_c_1", "mc0dat0to3_c_1", "mc0dat4to7_c_1",
	"mc0pwr_c_1", "mc0cmd_c_2", "mc0fbclk_c_1",
	"mc0dat0to3_c_2", "mc0dat4to7_c_2");
STA1385_FUNC_GROUPS(mc1,
	"mc1clk_a_1", "mc1cmd_a_1", "mc1dat0to7_a_1", "mc1fbclk_a_1",
	"mc1dat31dir_c_1", "mc1cmddir_c_1", "mc1dat0dir_c_1",
	"mc1dat2dir_c_1");
STA1385_FUNC_GROUPS(spi0,
	"spi0_a_1", "spi0ss_a_1", "spi0rxtx_b_1");
STA1385_FUNC_GROUPS(spi1,
	"spi1_a_1", "spi1sck_a_1", "spi1rxtx_a_1", "spi1ss_a_1",
	"spi1rxtx_c_1");
STA1385_FUNC_GROUPS(spi2,
	"spi2_a_1", "spi2ss_a_1");
STA1385_FUNC_GROUPS(sqi,
	"sqifdbsck_b_1", "sqice0n_b_1", "sqi_b_1", "sqice0n_c_1",
	"sqisio0_c_1", "sqifdbsck_c_1", "sqisck_c_1");
STA1385_FUNC_GROUPS(u0,
	"u0rxtx_a_1", "u0rtscts_a_1");
STA1385_FUNC_GROUPS(u1,
	"u1rxtx_a_1", "u1rtscts_a_1", "u1rxtx_a_2", "u1rxtx_b_1",
	"u1rtscts_b_1", "u1rtscts_b_2");
STA1385_FUNC_GROUPS(u2,
	"u2rxtx_a_1", "u2rxtx_b_1", "u2rxtx_b_2", "u2rxtx_c_1");
STA1385_FUNC_GROUPS(u3,
	"u3rxtx_b_1", "u3rxtx_b_2", "u3rxtx_c_1");
STA1385_FUNC_GROUPS(u4,
	"u4rxtx_b_1", "u4rxtx_b_2", "u4rxtx_c_1");
STA1385_FUNC_GROUPS(u5,
	"u5rxtx_b_1", "u5rxtx_c_1", "u5rxtx_c_2");
STA1385_FUNC_GROUPS(usb,
	"usb0drvvbus_b_1", "usb1drvvbus_b_1", "usb0drvvbus_b_2", "usb1drvvbus_b_2");
STA1385_FUNC_GROUPS(wdg,
	"wdgswrstout_b_1", "wdgswrstout_b_2");

#define FUNCTION(fname)					\
	{						\
		.name = #fname,				\
		.groups = fname##_groups,		\
		.ngroups = ARRAY_SIZE(fname##_groups),	\
	}

static const struct nmk_function nmk_sta1385_functions[] = {
	FUNCTION(can1),
	FUNCTION(can2),
	FUNCTION(clkinref),
	FUNCTION(clkout),
	FUNCTION(dma),
	FUNCTION(eft0),
	FUNCTION(eft1),
	FUNCTION(eft2),
	FUNCTION(eft3),
	FUNCTION(eft4),
	FUNCTION(eth0),
	FUNCTION(eth1),
	FUNCTION(fra),
	FUNCTION(frb),
	FUNCTION(frdbg),
	FUNCTION(fsmc),
	FUNCTION(i2c0),
	FUNCTION(i2c1),
	FUNCTION(i2c2),
	FUNCTION(i2s0),
	FUNCTION(i2s1),
	FUNCTION(i2s2),
	FUNCTION(mc0),
	FUNCTION(mc1),
	FUNCTION(spi0),
	FUNCTION(spi1),
	FUNCTION(spi2),
	FUNCTION(sqi),
	FUNCTION(u0),
	FUNCTION(u1),
	FUNCTION(u2),
	FUNCTION(u3),
	FUNCTION(u4),
	FUNCTION(u5),
	FUNCTION(usb),
	FUNCTION(wdg),
};

static const struct nmk_pinctrl_soc_data nmk_sta1385_soc = {
	.pins = nmk_sta1385_pins,
	.npins = ARRAY_SIZE(nmk_sta1385_pins),
	.functions = nmk_sta1385_functions,
	.nfunctions = ARRAY_SIZE(nmk_sta1385_functions),
	.groups = nmk_sta1385_groups,
	.ngroups = ARRAY_SIZE(nmk_sta1385_groups),
};

void nmk_pinctrl_sta1385_init(const struct nmk_pinctrl_soc_data **soc)
{
	*soc = &nmk_sta1385_soc;
}
