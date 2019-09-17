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
 * @brief   CSE key personalization support functions test
 * @details
 *
 *
 * @addtogroup CSE_support_test
 * @{
 */

#include "cse_typedefs.h"

#include <string.h> /* for memcmp */
#include "serialprintf.h"
#include "CSE_Key_update.h"

/**
 * @brief         Tests M1-M5 messages generation function, part of the support function set
 * @details       Example of use of the MA-M5 messages generation function to be used when loading a key with Mey_LOAD
 *
 * @param[in] 	  verbose   enable display of intermediate results when set to 1
 *
 * @return         Error code
 * @retval 0  	  When test failed
 * @retval 1   	  When test succeeded
 *
 */
int M1_M5_generation_test( int verbose ) {


  /* SHE memory update protocol reference vector */
  
  const uint8_t UID[15 + 1] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01, 0x00 };

  const uint8_t new_key[16] = {0x0f,0x0e,0x0d,0x0c,0x0b,0x0a,0x09,0x08,0x07,0x06,0x05,0x04,0x03,0x02,0x01,0x00 };
  const uint8_t k_authid[16]= {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f };

  const uint8_t ref_m1[16] =  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x41 };

  const uint8_t ref_m2[32] =  {0x2b,0x11,0x1e,0x2d,0x93,0xf4,0x86,0x56,0x6b,0xcb,0xba,0x1d,0x7f,0x7a,0x97,0x97,
		  	  	  	  	  	   0xc9,0x46,0x43,0xb0,0x50,0xfc,0x5d,0x4d,0x7d,0xe1,0x4c,0xff,0x68,0x22,0x03,0xc3};

  const uint8_t ref_m3[16] =  {0xb9,0xd7,0x45,0xe5,0xac,0xe7,0xd4,0x18,0x60,0xbc,0x63,0xc2,0xb9,0xf5,0xbb,0x46};
  const uint8_t ref_m4[32] =  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x41,
                               0xb4,0x72,0xe8,0xd8,0x72,0x7d,0x70,0xd5,0x72,0x95,0xe7,0x48,0x49,0xa2,0x79,0x17};
  const uint8_t ref_m5[16] =  {0x82,0x0d,0x8d,0x95,0xdc,0x11,0xb4,0x66,0x88,0x78,0x16,0x0c,0xb2,0xa4,0xe2,0x3e};

  char authid = 0x1;	/* MASTER_ECU_KEY */
  uint32_t id = 0x4; 	/* KEY_1 */
  uint32_t counter = 1;
  uint32_t fid = 0;

  uint8_t m1[16];
  uint8_t m2[32];
  uint8_t m3[16];
  uint8_t m4[32];
  uint8_t m5[16];

  int pass1 = 1;
  int pass2 = 1;
  int pass3 = 1;
  int pass4 = 1;
  int pass5 = 1;

  if (verbose) 
  {
    display_buf("new key    : ", (uint8_t*)new_key, 16);
    display_buf("auth key   : ", (uint8_t*)k_authid, 16);
    printf("key id     : 0x%02x\n", id);
    printf("authkey id : 0x%02x\n", authid);
    display_buf("UID        : ", (uint8_t*)UID, 15);
    printf("counter    : 0x%02x\n", counter);
    printf("fid        : 0x%02x\n", fid);
  }
  generate_M1_to_M5((uint32_t *)new_key, (uint32_t *)k_authid, (uint32_t *)UID,
                    id, authid, counter, fid,
                    0,	/* Test with nomal key set, not extended */
                    (uint32_t *)m1, (uint32_t *)m2, (uint32_t *)m3,
                    (uint32_t *)m4, (uint32_t *)m5);
  if (verbose) {
    display_buf("M1 : ", m1, 16);
    display_buf("M2 : ", m2, 32);
    display_buf("M3 : ", m3, 16);
    display_buf("M4 : ", m4, 32);
    display_buf("M5 : ", m5, 16);
    printf("\r\n");
  }

  /* Compare with expected result */
  pass1 = (memcmp(m1, ref_m1, 16) == 0);
  pass2 = (memcmp(m2, ref_m2, 32) == 0);
  pass3 = (memcmp(m3, ref_m3, 16) == 0);
  pass4 = (memcmp(m4, ref_m4, 32) == 0);
  pass5 = (memcmp(m5, ref_m5, 16) == 0);

  if (verbose) {
    printf(" expected :\n");
    display_buf("M1 : ", (uint8_t*)ref_m1, 16);
    display_buf("M2 : ", (uint8_t*)ref_m2, 32);
    display_buf("M3 : ", (uint8_t*)ref_m3, 16);
    display_buf("M4 : ", (uint8_t*)ref_m4, 32);
    display_buf("M5 : ", (uint8_t*)ref_m5, 16);
    printf("\r\n");

    if (!pass1)
      printf("M1 does not matches\n");
    if (!pass2)
      printf("M2 does not matches\n");
    if (!pass3)
      printf("M3 does not matches\n");
    if (!pass4)
      printf("M4 does not matches\n");
    if (!pass5)
      printf("M5 does not matches\n");
  }

  return (pass1 && pass2 && pass3 && pass4 && pass5);
}

/**
 * @}
 */
