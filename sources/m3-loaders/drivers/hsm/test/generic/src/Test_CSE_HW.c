/*
    CSE driver - Copyright (C) 2014-2015 STMicroelectronics

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
 * @file    Test_CSE_HW.c
 * @brief   Simple driver test scenario
 * @details
 *
 *
 * @addtogroup CSE_driver_test
 * @{
 */

#include <string.h> /* for memcmp */

#include "cse_client.h"

/* General commands */
#include "CSE_Constants.h"
#include "CSE_Manager.h"

/* AES operations & keys dedicated CSE commands */
#include "CSE_AES_HW_Modes.h"
#include "CSE_Key.h"

/* TRNG dedicated CSE commands */
#include "CSE_RNG.h"

/* User input/output via serial port */
#include "serialprintf.h"
#include "serial_input.h"

/* CSE AES operations tests functions */
#include "CSE_AES_HW_test.h"

/* CSE personalization data generation functions */
#include "CSE_RAM_KEY_test.h"
#include "CSE_RNG_test.h"

#include "test_support.h"
#include "test_values.h"

void test_CSE_driver( uint32_t verbose )
{
	int pass = 1;

	/*
	 * CSE driver Test
	 */
/*
 *
 	printf("\n");
	printf("This test uses the CSE driver functions to perform AES operations using the RAM key (ECB, CBC and CMAC modes) \n");
	printf("It can run on any CSE enabled chip without knowledge of personalized non volatile keys\n");
	printf("It also uses the export_ram function which is generating the encrypted and signed messages required to load a protected key\n");
	printf("\n");

	printf("Test are done 2 times:\n");
	printf("- once in a silent way, only telling if test passed or failed\n");
	printf("- the 2nd time, it displays all input values, computed and expected results\n");
	printf("\n\n");
*/

	display_CSE_status();

	{
		uint32_t UID[4] = { 0, 0, 0, 0 };
		uint32_t status;
		uint32_t MAC[4] = { 0, 0, 0, 0 };
		uint32_t ret;
		int i;

		printf("Get ID \n");

		/* Clear buffers */
		for(i=0;i<4;i++)
		{
		  UID[i] = 0x00000000;
		  MAC[i] = 0x00000000;
		}
		status = 0;

		ret = CSE_GetId( challenge, UID, &status, MAC);

		printf("ret : %x\n", ret );
		display_buf("challenge ", (uint8_t*)challenge, 16 );
		display_buf("UID       ", (uint8_t*)UID, 16 );
		display_buf("MAC       ", (uint8_t*)MAC, 16 );
		printf(     "status    %x", status);
		printf("\n");
	}

          /* Application main loop.*/
          {
            //int verbose = 1;

            //for (verbose = 0; verbose < 2; verbose++)
            {

            if(verbose==0){
                printf("\n******************************\n");
                printf("\nRunning tests in 'silent' mode\n");
                printf("\n******************************\n\n");
            }
            else
            {
                printf("\n*******************************\n");
                printf("\nRunning tests in 'verbose' mode\n");
                printf("\n*******************************\n\n");
            }

            /*
             * CSE driver AES computation functions
             */
            printf("*\n* AES CSE test (NIST ref vectors)\n*\n");
            pass = CSE_AES_HW_test(verbose);
            display_pass_fail(pass);

            /*
             * Test CSE export RAM
             */
            printf("*\n* RAM key export test \n*\n");
            pass = CSE_export_RAM_test(verbose);
            display_pass_fail(pass);

            /* TRNG test */
			printf("*\n* Random Number Generators\n*\n");
			pass = CSE_TRNG_test(verbose);
			display_pass_fail(pass);

			pass = CSE_PRNG_test(verbose);
			display_pass_fail(pass);


          }
	}
}

/**
 * @}
 */
