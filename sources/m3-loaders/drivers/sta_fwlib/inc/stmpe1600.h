/**
  ******************************************************************************
  * @file    stmpe1600.h
  * @author  MCD Application Team and APG/MID Application Team
  * @version V2.0.0
  * @date    25-May-2015
  * @brief   This file contains all the functions prototypes for the
  *          stmpe1600.c IO expander driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STMPE1600_H
#define __STMPE1600_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

/** @defgroup STMPE1600
  * @{
  */

/* Exported constants --------------------------------------------------------*/

/** @defgroup STMPE1600_Exported_Constants
  * @{
  */

typedef enum
{
   IO_MODE_INPUT = 0,
   IO_MODE_OUTPUT,
   IO_MODE_IT_RISING_EDGE,
   IO_MODE_IT_FALLING_EDGE,
   IO_MODE_IT_LOW_LEVEL,
   IO_MODE_IT_HIGH_LEVEL,
} IO_ModeTypedef;

/**
  * @brief STMPE1600 chip IDs
  */
#define STMPE1600_ID                     0x1600

/**
  * @brief  Interrupt enable
  */
#define STMPE1600_IT_ENABLE              0x04

/**
  * @brief  Identification registers & System Control
  */
#define STMPE1600_REG_CHP_ID             0x00
#define STMPE1600_REG_ID_VERSION         0x02
#define STMPE1600_REG_SYS_CTRL           0x03

/**
  * @brief  IO Registers
  */

#define STMPE1600_REG_GPMR               0x10
#define STMPE1600_REG_GPSR               0x12
#define STMPE1600_REG_GPDR               0x14
#define STMPE1600_REG_GPPIR              0x16

/**
  * @brief  Interrupt Control registers
  */
#define STMPE1600_REG_IEGPIOR            0x08
#define STMPE1600_REG_ISGPIOR            0x0A

/**
  * @brief  IO Pins direction
  */
#define STMPE1600_DIRECTION_IN           0x00
#define STMPE1600_DIRECTION_OUT          0x01

/**
  * @brief  IO IT polarity
  */
#define STMPE1600_POLARITY_LOW           0x00
#define STMPE1600_POLARITY_HIGH          0x01

/**
  * @brief  IO Pins
  */
#define STMPE1600_PIN_0                  0x0001
#define STMPE1600_PIN_1                  0x0002
#define STMPE1600_PIN_2                  0x0004
#define STMPE1600_PIN_3                  0x0008
#define STMPE1600_PIN_4                  0x0010
#define STMPE1600_PIN_5                  0x0020
#define STMPE1600_PIN_6                  0x0040
#define STMPE1600_PIN_7                  0x0080
#define STMPE1600_PIN_8                  0x0100
#define STMPE1600_PIN_9                  0x0200
#define STMPE1600_PIN_10                 0x0400
#define STMPE1600_PIN_11                 0x0800
#define STMPE1600_PIN_12                 0x1000
#define STMPE1600_PIN_13                 0x2000
#define STMPE1600_PIN_14                 0x4000
#define STMPE1600_PIN_15                 0x8000
#define STMPE1600_PIN_ALL                0xFFFF

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/

/** @defgroup STMPE1600_Exported_Functions
  * @{
  */

/**
  * @brief STMPE1600 Control functions
  */
int      stmpe1600_Init(uint16_t dev_addr);
int	 stmpe1600_Exit(uint16_t dev_addr);
int      stmpe1600_Reset(uint16_t dev_addr);
uint16_t stmpe1600_ReadID(uint16_t dev_addr);
void     stmpe1600_SetITPolarity(uint16_t dev_addr, uint8_t polarity);
void     stmpe1600_EnableGlobalIT(uint16_t dev_addr);
void     stmpe1600_DisableGlobalIT(uint16_t dev_addr);

/**
  * @brief STMPE1600 IO functionalities functions
  */
void     stmpe1600_IO_InitPin(uint16_t dev_addr, uint16_t IO_Pin, uint8_t direction);
void     stmpe1600_IO_Config(uint16_t dev_addr, uint16_t IO_Pin, IO_ModeTypedef IO_Mode);
void     stmpe1600_IO_PolarityInv_Enable(uint16_t dev_addr, uint16_t IO_Pin);
void     stmpe1600_IO_PolarityInv_Disable(uint16_t dev_addr, uint16_t IO_Pin);
void     stmpe1600_IO_WritePin(uint16_t dev_addr, uint16_t IO_Pin, uint8_t pin_state);
uint16_t stmpe1600_IO_ReadPin(uint16_t dev_addr, uint16_t IO_Pin);
void     stmpe1600_IO_EnablePinIT(uint16_t dev_addr, uint16_t IO_Pin);
void     stmpe1600_IO_DisablePinIT(uint16_t dev_addr, uint16_t IO_Pin);
uint8_t  stmpe1600_IO_ITStatus(uint16_t dev_addr, uint16_t IO_Pin);
uint8_t  stmpe1600_IO_ReadIT(uint16_t dev_addr, uint16_t IO_Pin);
void     stmpe1600_IO_ClearIT(uint16_t dev_addr, uint16_t IO_Pin);

#ifdef __cplusplus
}
#endif
#endif /* __STMPE1600_H */

/**
  * @}
  */

/**
  * @}
  */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
