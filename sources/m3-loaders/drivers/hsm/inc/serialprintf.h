/*
 *  Copyright (C) 2014 STMicroelectronics
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/**
 * @file	printf.h
 * @brief	Mini printf-like functionality.
 * @details	Minimal OS less printf support, using serial port. Code comes
 *		from chibiOS chprintf.c/.h module
 *
 *
 * @addtogroup printf
 * @{
 */

#ifndef SERIALPRINTF_H_
#define SERIALPRINTF_H_

#include "os_hal.h"

#include <stdarg.h>
#include "cse_typedefs.h"

#define serialprintf printf

/**
 * @brief	System formatted 32bits word output function.
 * @details	This function displays a 32bits integer value in hexadecimal,
 *		with 8 digits on serial port.
 *
 * @param[in]	word value to display
 *
 * @api
 */
extern void serialdisplay_word(uint32_t word);

/**
 * @brief	System formatted data buffer output function.
 * @details	This function displays a specified number of bytes of the
 *		provided buffer array in hexadecimal (2 digits per byte)
 *
 * @param[in]	buffer pointer to the data location
 * @param[in]	byte_count number of bytes to dispaly (2 hex digits per bytes)
 *
 * @api
 */
extern void serialdisplay_array(uint8_t *buffer, int byte_count);

/**
 * @brief	System formatted string output function.
 * @details	This function displays a text string (it displays a maximum of
 *		MAX_LENGTH chars, or stops if it finds an endofstring char (0)
 *		before)
 *
 * @param[in]	string pointer to the data location
 *
 * @api
 */
extern void serialprint_string(char *string);

/**
 * @brief	System formatted data buffer output function.
 * @details	This function displays a specified number of bytes of the
 *		provided buffer array in hexadecimal (2 digits per byte) with
 *		the provided comment string before
 *
 * @param[in]	cmt pointer to a comment string (muts be terminated by a
 *		end_of_string '\0' character)
 * @param[in]	buffer pointer to the data location
 * @param[in]	byte_count number of bytes to dispaly (2 hex digits per bytes)
 *
 * @api
 */
extern void display_buf(char *cmt, uint8_t *buf, uint32_t size);

#endif /* SERIALPRINTF_H_ */
