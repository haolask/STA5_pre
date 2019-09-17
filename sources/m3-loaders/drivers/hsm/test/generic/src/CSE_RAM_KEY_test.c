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
 * @file    CSE_RAM_KEY_test.c
 * @brief   CSE load and export Key test
 * @details
 *
 *
 * @addtogroup CSE_driver_test
 * @{
 */

#include "CSE_RAM_KEY_test.h"
#include "CSE_Key.h"
#include "CSE_Constants.h"

#include "CSE_AES_HW_Modes.h"

#include <string.h> /* for memcmp */
#include "serialprintf.h"

#include "cse_client.h"
#include "test_support.h"
#include "test_values.h"


/**
 * @brief          Test export RAM key function
 * @details        Load a known key and compare the export_RAM generated message with expected ones
 *
 * @param[in]      verbose        enable display of input, computed and expected values when set to 1
 *
 * @return         Error code
 * @retval 0        When test failed
 * @retval 1         When test succeeded
 *
 * @note           The generated message is only valid for a particular chip since it involves its unique ID
 */
int CSE_export_RAM_test(int verbose) {
  uint32_t ret = 0;
  uint8_t M1[16];
  uint8_t M2[32];
  uint8_t M3[16];
  uint8_t M4[32];
  uint8_t M5[16];
  int i;
  int pass = 1;

  const uint8_t key1[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  if (verbose) {
    printf("\n* Test Load & export RAM Key :\n\n");
    printf("-Load RAM Key :\n\r");
  }

  CSE_LoadRamKey((uint8_t*)key1);

  if (verbose) {
    display_CSE_status();
    serialprintf("\n");
  }

  if (verbose) {
    printf("-Export RAM Key :\n\r");
  }

  for(i=0; i<16; i++)
  {
      M1[i] = 0x00;
      M2[i] = 0x00;
      M3[i] = 0x00;
      M4[i] = 0x00;
      M5[i] = 0x00;
  }
  for(i=16; i<32; i++)
  {
      M2[i] = 0x00;
      M4[i] = 0x00;
  }

  ret = CSE_ExportRamKey(M1, M2, M3, M4, M5);
  if (verbose) {
    serialprintf("err = %x\n", ret);
    display_CSE_status();

    serialprintf("M1 = ");
    serialdisplay_array(M1, 16);
    serialprintf("M2 = ");
    serialdisplay_array(M2, 32);
    serialprintf("M3 = ");
    serialdisplay_array(M3, 16);
    serialprintf("M4 = ");
    serialdisplay_array(M4, 32);
    serialprintf("M5 = ");
    serialdisplay_array(M5, 16);
    serialprintf("-Done \n\r");
  }

  if( ret != CSE_NO_ERR)
  {
      pass = 0;
  }

  /* Compare with expected reference */

  return (pass);
}

/**
 * @brief          Perform RAM key functions test (load_cleartext, export, load_protected)
 * @details        Perform an AES operation to check that key has really been updated
 *
 * @param[in]      verbose        enable display of input, computed and expected values when set to 1
 *
 * @return         Error code
 * @retval 0         When test failed
 * @retval 1          When test succeeded
 *
 * @note           The test may not work if debugger is attached
 */
void ram_key_test( uint32_t verbose )
{
    uint32_t empty_key[4] = { 0, 0, 0, 0 };
    uint32_t dummy_key[4] = { 0x01234567, 0x89ABCDEF, 0x01234567, 0x89ABCDEF };

    uint8_t M1[16];
    uint8_t M2[32];
    uint8_t M3[16];

    uint8_t M4[32] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
    uint8_t M5[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
    uint8_t exp_M4[32] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
    uint8_t exp_M5[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

    uint32_t src[4] = { 0, 0, 0, 0 };
    uint32_t dst[4] = { 0, 0, 0, 0 };
    uint32_t ref_cypher[4] = { 0, 0, 0, 0 };
    uint32_t ret;

    printf("*\n* RAM Key load cleartext/export/load protected test\n*\n");
    printf("\n Note that the test Must fail if secure boot was not successful or if debugger is / was attached to the platform\n");
    printf("\n");

    printf("-Loading Key (cleartext) : \n");
    display_buf(" Key : ", (uint8_t*)empty_key, 16 );

    printf(" return = %x\n", CSE_LoadRamKey((uint8_t*)empty_key) );

    printf("-ECB Encryption test : \n");
    display_buf(" Plaintext :  ", (uint8_t*)src, 16 );
    CSE_AES_encrypt_ECB(CSE_RAM_KEY, src, ref_cypher, 1, 0);
    display_buf(" Cyphertext : ", (uint8_t*)ref_cypher, 16 );
    printf("\n");

    if(verbose)
    {
        printf("-Exporting Key : \n");
    }
    ret = CSE_ExportRamKey(M1, M2, M3, exp_M4, exp_M5);
    if(verbose)
    {
        display_buf("  M1 = ", M1, 16);
        display_buf("  M2 = ", M2, 32);
        display_buf("  M3 = ", M3, 16);
        display_buf("  M4 = ", exp_M4, 32);
        display_buf("  M5 = ", exp_M5, 16);
        printf("\n");
    }

    printf("-Loading another Key (cleartext) : \n");
    display_buf(" Key : ", (uint8_t*)dummy_key, 16 );
    printf(" return = %x\n", CSE_LoadRamKey((uint8_t*)dummy_key) );

    printf("-ECB Encryption test : \n");
    display_buf(" Plaintext :  ", (uint8_t*)src, 16 );
    CSE_AES_encrypt_ECB(CSE_RAM_KEY, src, dst, 1, 0);
    display_buf(" Cyphertext : ", (uint8_t*)dst, 16 );
    printf("\n");

    printf("-Loading RAM Key ( with protected material returned by export_ram_key) : \n");
    ret = CSE_LoadKey(M1, M2, M3, M4, M5, 0);
    printf(" ret : %x\n", ret );
    display_buf(" result M4 ", M4, 32 );
    display_buf(" result M5 ", M5, 16 );

    if( ret != 0 )
    {
        /* Check status */
        printf("\n");
        printf(" CSE status : %02x\n", CSE->SR.R);
        if( CSE->SR.B.BOK != 1 )
        {
            printf(" Secure boot is not successful (or not ran)\n");
        }
        if( CSE->SR.B.EDB == 1 )
        {
            printf(" Debugger is/was attached\n");
        }
    }

    printf("-ECB Encryption test : \n");
    display_buf(" Plaintext :  ", (uint8_t*)src, 16 );
    CSE_AES_encrypt_ECB(CSE_RAM_KEY, src, dst, 1, 0);
    display_buf(" Cyphertext : ", (uint8_t*)dst, 16 );
    printf("\n");
    if( memcmp( dst, ref_cypher, 16 ) == 0)
    {
        printf(" Key load with protected material was successful \n");
    }
    else
    {
        printf(" Key load with protected material failed - key was not updated \n");
    }

}

/**
 * @}
 */
