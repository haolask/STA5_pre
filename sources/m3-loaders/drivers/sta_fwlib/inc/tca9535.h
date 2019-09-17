/**
 * @file tca9535.c
 * @brief TI TCA9535 GPIO expander driver interface
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: ADG team
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TCA9535_H
#define __TCA9535_H

/** @defgroup TCA9535
 * @{
 */

/* Exported constants --------------------------------------------------------*/

/** @defgroup TCA9535_Exported_Constants
 * @{TCA9535
 */

/**
 * @brief  IO Registers
 */

#define TCA9535_REG_RD_PORT_0          0x00
#define TCA9535_REG_RD_PORT_1          0x01
#define TCA9535_REG_WR_PORT_0          0x02
#define TCA9535_REG_WR_PORT_1          0x03
#define TCA9535_REG_CFG_PORT_0         0x06
#define TCA9535_REG_CFG_PORT_1         0x07

/**
 * @brief  IO Pins direction
 */
#define TCA9535_DIRECTION_IN           0x01
#define TCA9535_DIRECTION_OUT          0x00

/**
 * @brief  IO IT polarity
 */
#define TCA9535_POLARITY_RETAINED      0x00
#define TCA9535_POLARITY_INVERTED      0x01

/**
 * @brief  IO Pins
 */
#define TCA9535_PIN_0                  0x0001
#define TCA9535_PIN_1                  0x0002
#define TCA9535_PIN_2                  0x0004
#define TCA9535_PIN_3                  0x0008
#define TCA9535_PIN_4                  0x0010
#define TCA9535_PIN_5                  0x0020
#define TCA9535_PIN_6                  0x0040
#define TCA9535_PIN_7                  0x0080
#define TCA9535_PIN_8                  0x0100
#define TCA9535_PIN_9                  0x0200
#define TCA9535_PIN_10                 0x0400
#define TCA9535_PIN_11                 0x0800
#define TCA9535_PIN_12                 0x1000
#define TCA9535_PIN_13                 0x2000
#define TCA9535_PIN_14                 0x4000
#define TCA9535_PIN_15                 0x8000
#define TCA9535_PIN_ALL                0xFFFF

/**
 * @}
 */

/* Exported functions --------------------------------------------------------*/

/** @defgroup STA9535_Exported_Functions
 * @{
 */

/**
 * @brief STA9535 Control functions
 */
int tca9535_init(uint16_t dev_addr);
int tca9535_exit(uint16_t dev_addr);

/**
 * @brief STA9535 IO functionalities functions
 */
int tca9535_io_init_pin(uint16_t dev_addr, uint16_t io_pin, uint8_t direction);
void tca9535_io_config(uint16_t dev_addr, uint16_t io_pin, uint8_t direction);
void tca9535_io_polarityinv_enable(uint16_t dev_addr, uint16_t io_pin);
void tca9535_io_polarityinv_disable(uint16_t dev_addr, uint16_t io_pin);
void tca9535_io_write_pin(uint16_t dev_addr, uint16_t io_pin,
			  uint8_t pin_state);
uint16_t tca9535_io_read_pin(uint16_t dev_addr, uint16_t io_pin);

/**
 * @}
 */

/**
 * @}
 */

#endif /* __TCA9535_H */
