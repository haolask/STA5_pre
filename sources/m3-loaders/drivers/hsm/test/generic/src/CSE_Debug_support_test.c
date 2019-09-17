/*
    SPC5-CRYPTO - Copyright (C) 2014 STMicroelectronics

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
 * @file    CSE_Key_support_test.c
 * @brief   CSE Debug support functions test
 * @details
 *
 *
 * @addtogroup CSE_support_test
 * @{
 */

#include "cse_typedefs.h"

#include <string.h> /* for memcmp */
#include "serialprintf.h"
#include "CSE_Debug.h"

/**
 * @brief         Tests Authorization value computation function, part of the support function set
 * @details       Example of use of the authorization computation function  to be provided to run debug challenge/authorization
 *
 * @param[in] verbose   enable display of intermediate results when set to 1
 * @return              Error code
 * @retval 0  	        When test failed
 * @retval 1   	        When test succeeded
 *
 */
int CSE_Compute_authorization_test( int verbose )
{
	uint32_t UID[4] =           { 0x04090808, 0x00160800, 0x0FF00FF0, 0x00000000 };
	uint32_t authorization[4] = { 0x00000000, 0x00000000, 0x00000000, 0x00000000 };
	uint32_t master_key[4] =    { 0xD275F12C, 0xA863A7B5, 0xF933DF92, 0x6498FB4D };

	uint32_t challengeTest[4] = { 0x05B27B58, 0x6D8653FC, 0x5404112D, 0x8A19A6B5 };
	uint32_t exp_auth[4] =      { 0xEEE5116E, 0x68F1A077, 0xA7C15E6D, 0x9301B2B3 };
	int fail;
	int i;

	if(verbose)
	{
		printf("\n");
		printf("Test compute authorization function (backend) \n");
	}
	Compute_authorization(challengeTest, UID, authorization, master_key);

	if(verbose)
	{
		display_buf("challenge : ", (uint8_t*)challengeTest, 16);
		display_buf("UID       : ", (uint8_t*)UID, 16);
		display_buf("authoriz  : ", (uint8_t*)authorization, 16);
		display_buf("exp_auth  : ", (uint8_t*)exp_auth, 16);
	}

	/* Compare with expected results */
	fail =0;
	for(i=0; i<4;i++)
	{
		if(exp_auth[i] != authorization[i])
		{
			fail = 1;
		}
	}
	if(verbose)
	{
		if(fail)
		{
			printf("Result does not match expected one\n");
		}
		else
		{
			printf("Result as expected\n");
		}
	}

	return(fail == 0);
}

/**
 * @}
 */
