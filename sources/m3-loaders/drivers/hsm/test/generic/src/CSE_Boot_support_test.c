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
 * @file    CSE_Key_support_test.h
 * @brief   CSE key personalization support functions test - header
 * @details
 *
 *
 * @addtogroup CSE_support_test
 * @{
 */

#include <string.h> /* for memcmp */

#include "cse_typedefs.h"
#include "serialprintf.h"

#include "CSE_Constants.h"
#include "CSE_AES_HW_Modes.h"
#include "CSE_BMAC_update.h"
#include "CSE_Debug.h"

#include "cse_client.h"	/* for CSE structure, used in display_cse_status */


static void display_CSE_status(void) {
  uint32_t status;
  uint32_t err;
  uint32_t cmd;

  cmd = CSE->CMD.R;
  status = CSE->SR.R;
  err = CSE->ECR.R;
  printf("CMD : %08X, SR : %08X, ECR : %08X\n", cmd, status, err);
}

/**
 * @brief         Computes BMAC as used in Bolero chip, differs from SHE Boot MAC computation
 * @details       Example of use of the BMAC computation support function, used to compute updated BMAC value for Bolero
 *
 * @param[in] verbose   enable display of intermediate results when set to 1
 * @return         Error code
 * @retval 0  	  When test failed
 * @retval 1   	  When test succeeded
 *
 */
int test_compute_BMAC(int verbose) {
  uint32_t bitlength = 0;
  int i;
  uint32_t err;
  int pass = 1;
  uint8_t computed1[64];

  const uint8_t bmk[16] = {0x12, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		  	  	  	  	   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x56, 0x78};

  uint8_t* start_ad = (uint8_t*)0x00FC0000; /* Start of application core flash */

  uint32_t bootloader_bytesize = 0x10;

  if (verbose) {
    printf("Compute MAC with BOOT_MAC_KEY index :\n\r");
  }

  for (i = 0; i < 16; i++) {
    computed1[i] = 0;
  }

  bitlength = bootloader_bytesize * 8;

  err = CSE_AES_generate_MAC(CSE_BOOT_MAC_KEY, (uint64_t*)&bitlength,
                             (uint32_t*)0x00000010, (uint32_t*)computed1, 0);
  if (verbose) {
    printf("Err=%x, ", err);
    display_CSE_status();
    printf("Computed MAC : ");
    serialdisplay_array((uint8_t*)computed1, 16);
    printf("\n\r");

    printf("Compute MAC with BOOT_MAC_KEY value:\n\r");
  }

  Compute_BMAC(start_ad, bootloader_bytesize, (uint8_t*)bmk,
               (uint8_t*)computed1);
  if (verbose) {
    printf("Computed MAC : ");
    serialdisplay_array((uint8_t*)computed1, 16);
    printf("\n\r");
  }
  return(pass);
}

/**
 * @}
 */
