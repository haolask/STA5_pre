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
 * @file    CSE_RNG.h
 * @brief   CSE Random Number Generator management module.
 * @details Set of functions used to manage Physical Random Number Generator
 *          embedded in CSE and AES based Pseudo random generator.
 *
 *
 * @addtogroup CSE_driver
 * @{
 * @addtogroup API
 * @{
 */
#ifndef _CSE_TRNG_H_
#define _CSE_TRNG_H_

#include "cse_typedefs.h"

/*============================================================================*/
/**
 * @brief         Get a Random value from Physical TRNG
 * @details       returns a 128 bits random value
 *
 * @param[in]     random	pointer to buffer wher the random value will be
 *                              written
 *
 * @return        Error code
 * @retval 0	  When the random number was extracted
 * @retval 1..21  In case of error - the error code values are the CSE returned
 *                ones
 *
 * @pre           The internal divider must be properly set before asking for a
 *                random number
 *
 * @note          The function is blocking and waits for operation completion
 */
extern uint32_t CSE_TRNG_Getrand(uint8_t *random);

/*============================================================================*/
/**
 * @brief         Initialize AES based Pseudo Random Generator
 * @details       Initialize the internal PRNG state with a seed value form the
 *                TRNG and sets the CSE_SR[RIN] status bit
 *
 * @return        Error code
 * @retval 0	  When the initialization was done properly
 * @retval 1..21  In case of error - the error code values are the CSE returned
 *                ones
 *
 * @pre           The internal divider must be properly set before asking for a
 *                random number
 *
 * @note          The function is blocking and waits for operation completion
 *
 */
extern uint32_t CSE_InitPRNG(void);

/*============================================================================*/
/**
 * @brief         Provides additional entropy to the PRNG
 * @details       Extends the state of PRNG using a 128bits entropy input value
 *
 * @param[in]     entropy_buffer pointer to input entropy buffer to use for
 *                PRNG re-seeding (128bits)
 *
 * @return        Error code
 * @retval 0	  When the initialization was done properly
 * @retval 1..21  In case of error - the error code values are the CSE returned
 *                ones
 *
 * @note          The function is blocking and waits for operation completion
 *
 */
extern uint32_t CSE_ExtendPRNGSeed(uint8_t *entropy_buffer);

/*============================================================================*/
/**
 * @brief         Initialize AES based Pseudo Random Generator
 * @details       Initialize the internal PRNG state with a seed value form the
 *		  TRNG and sets the CSE_SR[RIN] status bit
 *
 * @return        Error code
 * @retval 0	  When the initialization was done properly
 * @retval 1..21  In case of error - the error code values are the CSE returned
 *		  ones
 *
 * @pre		  The internal divider must be properly set before asking
 *		  for a random number
 *
 * @note          The function is blocking and waits for operation completion
 *
 */
extern uint32_t CSE_InitPRNG(void);

/*============================================================================*/
/**
 * @brief          Generates a 128 bits random value with AES based PRNG
 * @details        Computes a 128bits result and updates the internal PRNG state
 *
 * @param[out]     random	pointer to buffer where the random value will be
 *				written
 *
 * @return         Error code
 * @retval 0	   When the initialization was done properly
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *		   ones
 *
 * @pre		   The internal divider must be properly set before asking for
 *		   a random number
 *
 * @note           The function is blocking and waits for operation completion
 *
 */
extern uint32_t CSE_PRNG_GetRand(uint8_t *random);

/*============================================================================*/
/**
 * @brief          Run the TRNG online Test command
 * @details        Calls TRNG online test command, result is read in P1register
 *		   (0 -> failed, 1 -> passed)
 *
 * @param[out]     passed pointer to a variable to store the result of the
 *		   online test (0 :failed, 1 : test passed)
 *
 * @return         Error code
 * @retval 0	   When the initialization was done properly
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *		   ones
 *
 */
extern uint32_t CSE_TRNG_OnlineTest(uint32_t *passed);

#endif //_CSE_TRNG_H_

/**
 * @}
 * @}
 */
