/*
    CSE driver test - Copyright (C) 2014-2015 STMicroelectronics

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    CSE_RNG_test.c
 * @brief   CSE RNG commands tests
 * @details
 *
 *
 * @addtogroup CSE_driver_test
 * @{
 */


#include <string.h> /* for memcmp */
#include "serialprintf.h"

#include "CSE_Constants.h"
#include "CSE_RNG_test.h"
#include "CSE_RNG.h"

#include "err_codes.h"

#define CRL_AES128_KEY 16
#define CRL_AES_BLOCK 16

/**
 * @brief          True Random Number generator test
 * @details        Get a random stream from TRNG and perform online test
 *
 * @param[in]      verbose		enable display of input, computed and expected values when set to 1
 *
 * @return         Error code
 * @retval 0  	   When test failed
 * @retval 1   	   When test succeeded
 *
 */
int CSE_TRNG_test(int verbose) {
  uint32_t ret = 0;
  uint8_t random_stream[16];
  int success = 0;
  uint32_t pass;

  /* TRNG test */
  if(verbose)
  {
	printf("*\n* True Random Number Generator test\n*\n");
  }
  
  if (verbose) {
    printf("\n");
    printf("Get 128 random bits from TRNG\n\r");
  }

  ret = CSE_TRNG_Getrand( random_stream );
  if (verbose) {
	serialprintf("err = %x\n", ret);
  }
  //display_CSE_status();

  success = (ret == CSE_NO_ERR);
  if (verbose) {
    display_buf("Random   : ", random_stream, 16);
    printf("\n");
  }

  if(success)
  {
	  ret = CSE_TRNG_OnlineTest( &pass );
	  if (verbose) {
		  printf("Online test result (1: pass, 0 : failed): %d\n", pass);
		  printf("\n");
	  }
	  success = ((ret == CSE_NO_ERR) && (pass ==1));
  }

  return (success);
}

/**
 * @brief          Pseudo Random Number generator test
 * @details        Init, get a random stream, re-seed and ask for another stream
 *
 * @param[in]      verbose		enable display of input, computed and expected values when set to 1
 *
 * @return         Error code
 * @retval 0  	   When test failed
 * @retval 1   	   When test succeeded
 *
 */
int CSE_PRNG_test(int verbose) {
	uint32_t ret = 0;
	uint8_t random_stream[16];
	int success = 1;

	uint8_t entropy[16] = { 0x8D, 0x68, 0x83, 0x16, 0x8F, 0x77, 0x97, 0x34,
						  0x76, 0x0E, 0x59, 0x20, 0x06, 0xBE, 0x6C, 0x03  };

	/* TRNG test */
	if (verbose) {
		printf("*\n* Pseudo Random Number Generator test\n*\n");
	}

	if (verbose) {
		printf("\n");
		printf("Get 128 random bits from PRNG\n\r");
	}


	if (verbose) {
		printf("Initialize RNG\n");
	}
	ret = CSE_InitPRNG();

	ret = CSE_PRNG_GetRand( random_stream );
	if (verbose) {
		serialprintf("err = %x\n", ret);
		//display_CSE_status();
	}

	success &= (ret == CSE_NO_ERR);
	if (verbose) {
		display_buf("Random   : ", random_stream, 16);
		printf("\n");

		printf("Extending seed\n");
	}

	if (verbose) {
		serialprintf("err = %x\n", ret);
		//display_CSE_status();
	}
	

	ret = CSE_ExtendPRNGSeed(entropy);
	success &= (ret == CSE_NO_ERR);
	if (verbose) {
		printf("Getting a new random from PRNG\n");
	}
	ret = CSE_PRNG_GetRand( random_stream );
	if (verbose) {
		serialprintf("err = %x\n", ret);
		//display_CSE_status();
	}
	
	success &= (ret == CSE_NO_ERR);
	if (verbose) {
		display_buf("Random   : ", random_stream, 16);
		printf("\n");
	}

	return (success);
}

/**
 * @}
 */
