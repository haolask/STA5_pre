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
 * @file    CSE_NVM_KEY_test.c
 * @brief   CSE NVM key load, update, erase test
 * @details
 *
 * @addtogroup CSE_driver_test
 * @{
 */

#include <string.h> /* for memcmp */

/* User input/output via serial port */
#include "serialprintf.h"
#include "serial_input.h"

#include "CSE_Constants.h"
#include "CSE_NVM_KEY_test.h"
#include "CSE_Key.h"
#include "CSE_AES_HW_Modes.h"
#include "CSE_Manager.h"
#include "CSE_Debug.h"
#include "CSE_Key_update.h"

#include "test_values.h"
#include "err_codes.h"

#include "cse_client.h"	// for CSE structure, used in display_cse_status


static uint8_t computed1[64];
static const uint8_t cleartext[64] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
                                      0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
                                      0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14,
                                      0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b,
                                      0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22,
                                      0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
                                      0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
                                      0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
                                      0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e,
                                      0x3f};


/**
 * @brief          Test the load key function for NVM keys
 * @details        Load keys in the NVM locations in a virgin chip ( or at least a chip whose user keys are empty)
 *
 * @param[in]      verbose		enable display of input, computed and expected values when set to 1
 *
 * @return         Error code
 * @retval 0  	  When test failed
 * @retval 1   	  When test succeeded
 *
 * @note			  The expected message are only valid for a particular chip since it involves its unique ID
 */
int CSE_NVM_key_update_interactive_test(int verbose) {
  uint8_t UID[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  uint8_t k_authid[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  uint32_t err;

  int pass = 1;

  uint32_t authid = 0xB;
  uint32_t id = 0xB;
  uint32_t cid = 1;
  uint32_t fid = 0;

  uint8_t key_val[16];

  uint8_t m1[16];
  uint8_t m2[32];
  uint8_t m3[16];
  uint32_t m4_w[8];
  uint32_t m5_w[4];
  uint32_t confirm;
  uint8_t flags;
  int end = 0;
  int i;
  volatile uint64_t bitlength = 128;

  uint32_t status;
  uint32_t MAC[4];

  uint32_t M4[8];
  uint32_t M5[4];
  uint32_t match;
  uint32_t ext_key_set;
  uint32_t wildcard_uid;

  /* Test CSE NVM Key update */
  printf("*\n* CSE NVM key update test\n*\n");

  if (verbose) {
    printf("Test Update key with user provided values\n");

    printf(" It will compute the M1..M5 messages required to load a key in a NVM location.\n");
    printf(" then you'll have to confirm if you want to perform the write operation or not\n");
  }

  while (!end) {
	/* Ask for extended key set or normal Key set*/
	get_int("Extended key set (ie. key11..20) (1) or normal Key (0) : ", &ext_key_set);
	if(ext_key_set != 0 )
	{
		ext_key_set = 1;
	}

	get_int("Wildcard UID (1) or chip UID (0) : ", &wildcard_uid);
	if(wildcard_uid == 0 )
	{
		/* Read Chip UID */
		  CSE_GetId(challenge, (uint32_t*)UID, &status, MAC);
	}
	else
	{
		for(i=0; i<15; i++)
		{
			UID[i] = 0U;
		}
		wildcard_uid = 1;
	}

    /* Ask for key index */
    get_int("Key index (dec) : ", &id);

    /* Ask for new key value */
    get_hex("Key value (hex) - 16 bytes : ", key_val, 16);

    /* Ask for auth key index */
    get_int("Auth Key index (dec) : ", &authid);

    /* Ask for auth key value */
    get_hex("Auth Key value (hex) - 16 bytes : ", k_authid, 16);

    /* Ask for new flags */
    get_hex("Flags (hex) - 1 byte (binary format = WP|BP|DP|KU|WC|VO|00b): ", &flags, 1);
    fid = (uint32_t)flags;

    /* Ask for counter value */
    get_int("Counter value (dec) - !!! 28 bits max (268 435 456) : ", &cid);

    /* display summary */
    display_buf("UID      : ", (uint8_t*)UID, 15);
    display_buf("new key  : ", key_val, 16);
    display_buf("aut key  : ", k_authid, 16);
    printf("key id : %d ", id);
    printf("aut id : %d ", authid);
    printf("flags  : %x ", fid);
    printf("count  : %x ", cid);
    printf("\r\n");

    generate_M1_to_M5((uint32_t *)key_val, (uint32_t *)k_authid,
                      (uint32_t *)UID, id, authid, cid, fid,
                      ext_key_set,
                      (uint32_t *)m1, (uint32_t *)m2, (uint32_t *)m3,
					  m4_w, m5_w);

    display_buf("M1 : ", m1, 16);
    display_buf("M2 : ", m2, 32);
    display_buf("M3 : ", m3, 16);
    display_buf("M4 : ", (uint8_t*)m4_w, 32);
    display_buf("M5 : ", (uint8_t*)m5_w, 16);
    printf("\r\n");

    /* ask for confirmation */
    get_int("Do you want to write key (1 : yes, any other value : no) ? ",
            &confirm);

    if (confirm != 1) {
      printf(" write operation skipped\n");
    }
    else {
      printf("Write key\n");

      for (i = 0; i < 8; i++)
      {
        M4[i] = 0x00000000;
      }
	  for (i = 0; i < 4; i++)
      {
        M5[i] = 0x00000000;
      }

      err = CSE_LoadKey(m1, m2, m3, (uint8_t*)M4, (uint8_t*)M5, ext_key_set);
      printf("Err=%x, ", err);
      display_CSE_status();
      printf("\n\r");

      	printf("Received update verification messages \n");
        printf("M4 : ");
		serialdisplay_array((uint8_t*)M4, 32);
		printf("M5 : ");
		serialdisplay_array((uint8_t*)M5, 16);


		/* Check if they match with expected messages */
		match = 1;

		for (i = 0; i < 8; i++)
		{
      	  if(m4_w[i] != M4[i])
      	  {
      		  match = 0;
      	  }
      	}
      	if( match )
      	{
      		printf("M4 as expected\n");
      	}
      	else
      	{
      		printf("M4 does not match expected result\n");
      	}

      	match = 1;
      	for (i = 0; i < 4; i++)
      	{
      	  if(m5_w[i] != M5[i])
      	  {
      		  match = 0;
      	  }
      	}
      	if( match )
      	{
      		printf("M5 as expected\n");
      	}
      	else
      	{
      		printf("M5 does not match expected result\n");
      	}

      printf("\nNow try to use key we just updated \n");

      /* Now check if the key changed */

      /* encryption test */
      printf("AES ECB encryption with key %d (0x%X) :\n\r", id, id);
      for (i = 0; i < 16 * 2; i++) {
        computed1[i] = 0;
      }
      err = CSE_AES_encrypt_ECB(id, (uint32_t*)cleartext, (uint32_t*)computed1,
                                2, 0);
      printf("Err=%x, ", err);
      display_CSE_status();
      serialdisplay_array((uint8_t*)computed1, 16 * 2);

      printf("AES CMAC with key %d (0x%X) :\n\r", id, id);
      for (i = 0; i < 16; i++) {
        computed1[i] = 0;
      }
      err = CSE_AES_generate_MAC(id, (uint64_t*)&bitlength,
                                 (uint32_t*)cleartext, (uint32_t*)computed1, 0);
      printf("Err=%x, ", err);
      display_CSE_status();
      serialdisplay_array((uint8_t*)computed1, 16);
      printf("\n\r");

    }

    /* ask for confirmation */
    get_int(
        "Do you want to write another key ? (1 : yes, any other value : no) ? ",
        &confirm);
    if (confirm != 1) {
      end = 1;
    }
  }
  return (pass);
}

/**
 * @brief          Test the NVM keys to check if they're present and available for encryption or MAC operation
 * @details        perform an ECB and CMAC operation attempt for each key and display the low level status and error report
 *
 * @param[in]      verbose		enable display of input, computed and expected values when set to 1
 *
 * @return         Error code
 * @retval 0  	  When test failed
 * @retval 1   	  When test succeeded
 *
 */
int CSE_NVM_Key_AES_test(int verbose) {
  int i;
  uint32_t key_id;
  int pass = 1;
  uint32_t ret;
  uint64_t bitlength = 128;
  uint32_t ext_key_set = 0;

  /* Test use of all NVM Keys, reporting the status and type for each of them */
  printf("*\n* CSE check AES NVM key status\n*\n");


  if (verbose) {
    printf("\n\rTest AES encryption and MAC with NVM Keys \n\r");

    printf("\n--Testing special locations (ROM, MASTER, BOOT_MAC_KEY and BOOTMAC)\n");
    printf(" Should return empty (0x05) or Invalid Key (0x04) when trying to use them for encryption\n");
  }

  /*
   *  Test with special keys
   */
  ext_key_set = 0;

  for (key_id = 0; key_id <= 3; key_id++) {
    if (verbose)
    {
      printf("HW AES ECB encryption with NVM Key %d (0x%x) - MUST NOT SUCCEED:\n\r", key_id, key_id);
    }
    for (i = 0; i < 16; i++) {
      computed1[i] = 0;
    }
    ret = CSE_AES_encrypt_ECB(key_id, (uint32_t*)cleartext,
                              (uint32_t*)computed1, 1, ext_key_set);
    if (verbose) {
      display_CSE_status();
      switch (ret) {
      case CSE_NO_ERR:
        printf(" - Key is available for encryption\n");
        serialdisplay_array((uint8_t*)computed1, 16);
        break;
      case CSE_KEY_NOT_AVAIL:
        printf(" - Key not available - due to Boot or debug prot\n");
        break;
      case CSE_INV_KEY:
        printf(" - Key invalid\n");
        break;
      case CSE_EMPTY_KEY:
        printf(" - Key empty\n");
        break;
      default:
        printf(" - Other error : 0x%02x\n", ret);
        break;
      }
    }
  }

  if (verbose)
  {
    printf("\n--Testing User NVM Keys  (KEY_1 .. KEY_10) for AES encryption operation\n");
  }
  for (key_id = CSE_KEY_1; key_id <= CSE_KEY_10; key_id++) {
    if (verbose) {
      printf("HW AES ECB encryption with NVM_Key Key_%d (idx = 0x%x):\n\r",
             key_id - 3, key_id);
    }

    for (i = 0; i < 16; i++) {
      computed1[i] = 0;
    }
    ret = CSE_AES_encrypt_ECB(key_id, (uint32_t*)cleartext,
                              (uint32_t*)computed1, 1, ext_key_set);
    if (verbose) {
      display_CSE_status();
      switch (ret) {
      case CSE_NO_ERR:
        printf(" - Key is available for encryption\n");
        serialdisplay_array((uint8_t*)computed1, 16);
        break;
      case CSE_KEY_NOT_AVAIL:
        printf(" - Key not available - due to Boot or debug prot\n");
        break;
      case CSE_INV_KEY:
        printf(" - Key invalid\n");
        break;
      case CSE_EMPTY_KEY:
        printf(" - Key empty\n");
        break;
      default:
        printf(" - Other error : 0x%02x\n", ret);
        break;
      }
    }
  }

  if (verbose) {
    printf(
        "\n--Testing User NVM Keys  (KEY_1 .. KEY_10) for AES CMAC operation\n");
  }
  for (key_id = CSE_KEY_1; key_id <= CSE_KEY_10; key_id++) {
    if (verbose) {
      printf("HW AES CMAC generation with NVM_Key Key_%d (idx = 0x%x):\n\r",
             key_id - 3, key_id);
    }
    for (i = 0; i < 16; i++) {
      computed1[i] = 0;
    }

    ret = CSE_AES_generate_MAC(key_id, (uint64_t*)&bitlength,
                               (uint32_t*)cleartext, (uint32_t*)computed1, 
                               ext_key_set);
    if (verbose) {
      display_CSE_status();
      switch (ret) {
      case CSE_NO_ERR:
        printf(" - Key is available for CMAC\n");
        serialdisplay_array((uint8_t*)computed1, 16);
        break;
      case CSE_KEY_NOT_AVAIL:
        printf(" - Key not available - due to Boot or debug prot\n");
        break;
      case CSE_INV_KEY:
        printf(" - Key invalid\n");
        break;
      case CSE_EMPTY_KEY:
        printf(" - Key empty\n");
        break;
      default:
        printf(" - Other error : 0x%02x\n", ret);
        break;
      }
    }
  }



  /*
   *  All the following keys are extended ones (KEY_11 ... KEY_20)
   */
  ext_key_set = 1;


  if (verbose) {
      printf("\n--Testing Extended User NVM Keys  (KEY_11 .. KEY_20) for AES encryption operation\n");
    }
    for (key_id = CSE_KEY_11; key_id <= CSE_KEY_20; key_id++)
    {

    	if (verbose)
    	{
    		printf("HW AES ECB encryption with NVM_Key Key_%d (idx = 0x%x):\n\r",
	               10+key_id - 3, key_id);
    	}

    	for (i = 0; i < 16; i++)
    	{
    		computed1[i] = 0;
    	}
    	ret = CSE_AES_encrypt_ECB(key_id, (uint32_t*)cleartext,
                                (uint32_t*)computed1, 1, ext_key_set);
      if (verbose) {
        display_CSE_status();
        switch (ret) {
        case CSE_NO_ERR:
          printf(" - Key is available for encryption\n");
          serialdisplay_array((uint8_t*)computed1, 16);
          break;
        case CSE_KEY_NOT_AVAIL:
          printf(" - Key not available - due to Boot or debug prot\n");
          break;
        case CSE_INV_KEY:
          printf(" - Key invalid\n");
          break;
        case CSE_EMPTY_KEY:
          printf(" - Key empty\n");
          break;
        default:
          printf(" - Other error : 0x%02x\n", ret);
          break;
        }
      }
    }

    if (verbose) {
      printf("\n--Testing Extended User NVM Keys  (KEY_11 .. KEY_20) for AES CMAC operation\n");
    }
    for (key_id = CSE_KEY_11; key_id <= CSE_KEY_20; key_id++) {
      if (verbose) {
        printf("HW AES CMAC generation with NVM_Key Key_%d (idx = 0x%x):\n\r",
               10+key_id - 3, key_id);
      }
      for (i = 0; i < 16; i++) {
        computed1[i] = 0;
      }

      ret = CSE_AES_generate_MAC(key_id, (uint64_t*)&bitlength,
                                 (uint32_t*)cleartext, (uint32_t*)computed1, ext_key_set);
      if (verbose) {
        display_CSE_status();
        switch (ret) {
        case CSE_NO_ERR:
          printf(" - Key is available for CMAC\n");
          serialdisplay_array((uint8_t*)computed1, 16);
          break;
        case CSE_KEY_NOT_AVAIL:
          printf(" - Key not available - due to Boot or debug prot\n");
          break;
        case CSE_INV_KEY:
          printf(" - Key invalid\n");
          break;
        case CSE_EMPTY_KEY:
          printf(" - Key empty\n");
          break;
        default:
          printf(" - Other error : 0x%02x\n", ret);
          break;
        }
      }
    }


  return (pass);
}

/**
 * @brief          Test the NVM key load function with already pre-computed messages
 * @details        Let the user select which key he wants to load and perform an ECB and CMAC test before and after load
 *
 * @param[in]      verbose		enable display of input, computed and expected values when set to 1
 *
 * @return         Error code
 * @retval 0  	  When test failed
 * @retval 1   	  When test succeeded
 *
 * @note		      The pre-computed messages are only valid for one chip a particular chip since involving its unique ID
 */
int CSE_NVM_precomputed_key_load_test(int verbose) {
  typedef struct {
    uint32_t key_idx;
    uint32_t M1[4];
    uint32_t M2[8];
    uint32_t M3[4];
    uint32_t M4[8];
    uint32_t M5[4];
  } LoadKeyMessages_t;

  /* Pre-computed messages */

  /* MASTER ECU Key */
  LoadKeyMessages_t MASTER_D24D_aut_0000_ID4000 = {0x1,
                                                   {0x04090808, 0x00160800,
                                                    0x0FF00FF0, 0x00000011},
                                                   {0xff8b75f7, 0x3e6ad5a1,
                                                    0x729423c6, 0xe9311f1a,
                                                    0xf42e083b, 0x7045d311,
                                                    0x9ed19270, 0xf0cc28ec},
                                                   {0x6955f240, 0x434d30b7,
                                                    0xa4a526fd, 0x5d77b697},
                                                   {0x04090808, 0x00160800,
                                                    0x0FF00FF0, 0x00000011,
                                                    0xe3c31150, 0x70edb849,
                                                    0x28cfb0be, 0x55077a8f},
                                                   {0x03f95ef6, 0xa0071e90,
                                                    0x96227cd1, 0xd81c9968}};

  /* UID = 0409...00, new = 2b7e151628aed2a6abf7158809cf4f3c,  previous Key = 000000000*/
  LoadKeyMessages_t K7_2B3C_aut_0000_ID0400 = {0xA, {0x04090808, 0x00160800,
                                                     0x0FF00FF0, 0x000000AA},
                                               {0x858967fd, 0xf0f59179,
                                                0x026b1891, 0xf94fd37b,
                                                0xf5cbc83d, 0x3c5dc8f3,
                                                0x4b1a5c84, 0x63431329},
                                               {0x66536b7f, 0x335d11b0,
                                                0x97eef0cc, 0xfba6da88},
                                               {0x04090808, 0x00160800,
                                                0x0FF00FF0, 0x000000AA,
                                                0x406ed0b6, 0x0009e4ef,
                                                0x866507d1, 0xfe13e52d},
                                               {0x4acf88a2, 0x858036ad,
                                                0xbe04c559, 0xfc7f2c11}};

  /* UID = 0409...00, new = 10AF4B5B024195B91730D7F594C87E19, previous Key = 0 , MAC */
  LoadKeyMessages_t K8_1019_aut_0000_ID0400 = {0xB, {0x04090808, 0x00160800,
                                                     0x0FF00FF0, 0x000000BB},
                                               {0xff8b75f7, 0x3e6ad5a1,
                                                0x729423c6, 0xe9311f1a,
                                                0x679ba7ec, 0xcbdb0160,
                                                0xb747665f, 0xf9f6bf8f},
                                               {0xc7fc3fb7, 0x6f2e3acd,
                                                0xc5025fc9, 0xde83e7f9},
                                               {0x04090808, 0x00160800,
                                                0x0FF00FF0, 0x000000BB,
                                                0x7a07321d, 0xdf82c5d4,
                                                0x72b52be0, 0x9a109e88},
                                               {0x5684db05, 0x912d5475,
                                                0x8163862e, 0x7516843a}};

  /* UID = 0409...00, new =0, previous Key = 0*/
  LoadKeyMessages_t K8_0000_aut_0000_ID0400 = {0xB, {0x04090808, 0x00160800,
                                                     0x0FF00FF0, 0x000000BB},
                                               {0xff8b75f7, 0x3e6ad5a1,
                                                0x729423c6, 0xe9311f1a,
                                                0x69ccff39, 0x3e17baed,
                                                0xa2952454, 0xaf1b2311},
                                               {0x6d67fe75, 0xdaa31ecc,
                                                0x3fa24dbc, 0x03110b13},
                                               {0x04090808, 0x00160800,
                                                0x0FF00FF0, 0x000000BB,
                                                0xee52f069, 0x492a9d3d,
                                                0x1283557e, 0xa645746c},
                                               {0xf4bf00b6, 0x75dbaee7,
                                                0x5d55e1be, 0xed3d2db9}};

  uint32_t err;

  LoadKeyMessages_t* key;
  uint32_t keyIndex = 3;

  volatile uint64_t bitlength = 128;

#define INDEX_MAX 4

  LoadKeyMessages_t* keyArray[INDEX_MAX] = {&MASTER_D24D_aut_0000_ID4000,
                                             &K7_2B3C_aut_0000_ID0400,
                                             &K8_1019_aut_0000_ID0400,
                                             &K8_0000_aut_0000_ID0400};
  uint32_t M4[8];
  uint32_t M5[4];
  uint32_t match;

  uint32_t key_idx = CSE_RAM_KEY;
  int i;

  if (verbose) {
    printf("Test NVM key load  :\n\r");
  }

  /*
   *
   */printf(" Please select which key to load :\n");
  printf(" - 0 : Load master Key - will only work for a virgin chip\n");
  printf(" - 1 : Load KEY_7 (2B..3C) for encryption - will only work for a virgin chip\n");
  printf(" - 2 : Load KEY_8 (10..19) for encryption - will only work for a virgin chip\n");
  printf(" - 3 : Load KEY_8 (00..00) for encryption - will only work for a virgin chip\n");
  printf(" - 4 : exit this test\n");

  printf("Your choice ? : ");
  get_int("", &keyIndex);
  printf("selected index : %d\n\r", keyIndex);

  if (keyIndex < INDEX_MAX) {
    key = keyArray[keyIndex];
    key_idx = key->key_idx;

    printf("Before Loading NVM Key %X\n\r", key_idx);
    printf("encryption attempt\n\r");
    for (i = 0; i < 16; i++) {
      computed1[i] = 0;
    }
    err = CSE_AES_encrypt_ECB(key_idx, (uint32_t*)cleartext,
                              (uint32_t*)computed1, 1, 0);
    printf("Err=%x, ", err);
    display_CSE_status();
    serialdisplay_array((uint8_t*)computed1, 16);

    printf("MAC attempt :\n\r");
    for (i = 0; i < 16; i++) {
      computed1[i] = 0;
    }
    err = CSE_AES_generate_MAC(key_idx, (uint64_t*)&bitlength,
                               (uint32_t*)cleartext, (uint32_t*)computed1, 0);
    printf("Err=%x, ", err);
    display_CSE_status();
    serialdisplay_array((uint8_t*)computed1, 16);
    printf("\n\r");

    printf("Load key :\n\r");
    printf("M1 : ");
    serialdisplay_array((uint8_t*)key->M1, 16);
    printf("M2 : ");
    serialdisplay_array((uint8_t*)key->M2, 32);
    printf("M3 : ");
    serialdisplay_array((uint8_t*)key->M3, 16);
    printf("M4 : ");
    serialdisplay_array((uint8_t*)key->M4, 32);
    printf("M5 : ");
    serialdisplay_array((uint8_t*)key->M5, 16);

    for (i = 0; i < 8; i++)
	{
	  M4[i] = 0x00000000;
	}
    for (i = 0; i < 4; i++)
	{
	  M5[i] = 0x00000000;
	}

    err = CSE_LoadKey((uint8_t*)key->M1, (uint8_t*)key->M2, (uint8_t*)key->M3,
                      (uint8_t*)M4, (uint8_t*)M5, 0);
    printf("Err=%x, ", err);
    display_CSE_status();
    printf("\n\r");

    printf("Received update verification messages \n");
    printf("M4 : ");
	serialdisplay_array((uint8_t*)M4, 32);
	printf("M5 : ");
	serialdisplay_array((uint8_t*)M5, 16);


	/* Check if they match with expected messages */
	match = 1;
	for (i = 0; i < 8; i++)
	{
	  if(key->M4[i] != M4[i])
	  {
		  match = 0;
	  }
	}
	if( match )
	{
		printf("M4 as expected\n");
	}
	else
	{
		printf("M4 does not match expected result\n");
	}


	match = 1;
	for (i = 0; i < 4; i++)
	{
	  if(key->M5[i] != M5[i])
	  {
		  match = 0;
	  }
	}
	if( match )
	{
		printf("M5 as expected\n");
	}
	else
	{
		printf("M5 does not match expected result\n");
	}

    printf("\nNow try to use key we just updated \n");
	printf("encryption attempt :\n\r");
    for (i = 0; i < 16; i++) {
      computed1[i] = 0;
    }
    err = CSE_AES_encrypt_ECB(key_idx, (uint32_t*)cleartext,
                              (uint32_t*)computed1, 1, 0);
    printf("Err=%x, ", err);
    display_CSE_status();
    serialdisplay_array((uint8_t*)computed1, 16);

    printf("MAC attempt :\n\r");
    for (i = 0; i < 16; i++) {
      computed1[i] = 0;
    }
    err = CSE_AES_generate_MAC(key_idx, (uint64_t*)&bitlength,
                               (uint32_t*)cleartext, (uint32_t*)computed1, 0);
    printf("Err=%x, ", err);
    display_CSE_status();
    serialdisplay_array((uint8_t*)computed1, 16);
  }
  else {
    if (keyIndex == INDEX_MAX) {
      printf(" skipping test\n");
    }
    else {
      printf("Invalid index (max is %d) : \n", INDEX_MAX);
    }
  }
  printf("-Done \n\r");

  return (1);
}

/**
 * @brief          Erase all NVM user keys
 * @details        Perform the DEBUG_CHALLENGE/DEBUG_AUTHORIZATION sequence to erase the user keys in non volatile memory
 *
 * @param[in]      verbose		enable display of input, computed and expected values when set to 1
 * @param[in]      P_pMasterKey	pointer to master_key value
 *
 * @return         Error code
 * @retval 0  	  When test failed
 * @retval 1   	  When test succeeded
 *
 * @note			  Will only work if no key has the Write Protect flag set
 */
int CSE_erase_all_user_keys_test(int verbose, uint32_t* P_pMasterKey) {
  uint32_t err;
  uint32_t authorization[4];
  uint32_t debugChallenge[4];

  uint8_t UID[16];
  uint32_t status;
  uint32_t MAC[4];
  uint32_t confirm;

  uint32_t pass = 0;

  if (verbose) {
    printf(" Erase all User keys\n");
    printf(" - get chip UID\n");
  }

  /* Read Chip UID */
  err = CSE_GetId(debugChallenge, (uint32_t*)UID, &status, MAC);
  if (verbose) {
    printf("Err=%x\n", err);
    display_CSE_status();
    display_buf("UID      : ", (uint8_t*)UID, 15);
  }

  err = CSE_DebugChallenge(debugChallenge);
  if (verbose) {
    printf("Err=%x\n", err);
    display_CSE_status();
    display_buf("Challenge: ", (uint8_t*)debugChallenge, 16);
  }
  if (err == CSE_NO_ERR) {
    err = Compute_authorization(debugChallenge, (uint32_t*)UID, authorization, P_pMasterKey);
    if (verbose) {
      printf("Err=%x\n", err);
      display_CSE_status();
      display_buf("Authoriz.: ", (uint8_t*)authorization, 16);
    }
    if (err == AES_SUCCESS)
    {
      get_int("Do you really want to erase NVM user keys ? (1 : yes, any other value : no) ? ",
			&confirm);
      if (confirm == 1)
      {
        err = CSE_DebugAuth(authorization);
        if (verbose) {
          printf("Err=%x\n", err);
          display_CSE_status();
        }

        pass = (err == CSE_NO_ERR );
	  }
      else
      {
    	  printf("User Keys erasure skipped\n");
      }
    }
  }
  return ( pass );
}

/**
 * @brief          Erase all NVM user keys - with selection of the master key
 * @details        Perform the DEBUG_CHALLENGE/DEBUG_AUTHORIZATION sequence to erase the user keys in non volatile memory
 *
 * @param[in]      verbose		enable display of input, computed and expected values when set to 1
 *
 * @note		   Will only work if no key has the Write Protect flag set
 */
void erase_keys_test( uint32_t verbose )
{
	int pass;
	uint32_t choice;

	if(0 == CSE->SR.B.RIN)
	{
		printf("PRNG is not yet initialized, erase will fail\n");
	}

	get_int("Which master key do you want to use (1: master_key, 2 : empty master_key) ? ", &choice);
	if( 1 == choice )
	{
		printf("\n");
		/* Erase all NVM Keys */
		printf("*\n* Test reset of all user keys\n*\n");
		pass = CSE_erase_all_user_keys_test(verbose, master_key);
		display_pass_fail(pass);
	}
	else
	{
		if( 2 == choice )
		{
			printf("\n");
			printf("trying to erase without prng init, empty master key\n");
			/* Erase all NVM Keys */
			printf("*\n* Test reset of all user keys\n*\n");
			pass = CSE_erase_all_user_keys_test(verbose, empty_key);
			display_pass_fail(pass);
		}
		else
		{
			printf("invalid choice, test aborted\n");
		}
	}
}


/**
 * @brief          Iterative key update test
 * @details        perform 64 updates of 2 keys starting from user specified counter
 *
 * @param[in]      verbose		enable display of input, computed and expected values when set to 1
 *
 * @note		   Will only work if key has not the Write Protect flag set, and counter is greater than current one
 */
void iterative_nvm_key_update_test( uint32_t verbose )
{
#define NB_ITERATIVE_UPDATES 65

	uint32_t M1[4];
	uint32_t M2[8];
	uint32_t M3[4];
	uint32_t M4[8];
	uint32_t M5[4];
	uint32_t array_idx;
	uint32_t CSE_Key_idx;
	vuint32_t UID[4]  = { 0x00000000, 0x00000000, 0x00000000, 0x00000000 };
	vuint32_t status = 0;
	vuint32_t MAC[4] = { 0x00000000, 0x00000000, 0x00000000, 0x00000000 };
	uint32_t ret;
	int i;
	uint32_t counter;
	uint32_t ext_key_set = 0;
	uint32_t initial_counter;

	printf("\n\nIterative (%d x) updates of key_1 and Key_18\n\n", NB_ITERATIVE_UPDATES);

	get_int("Initial counter value ? : ", &initial_counter);

	/* Clear buffers */
	for(i=0;i<4;i++)
	{
		UID[i] = 0x00000000;
		MAC[i] = 0x00000000;
	}
	status = 0;

	ret = CSE_GetId( challenge, (uint32_t*)UID, (uint32_t*)&status, (uint32_t*)MAC);


	for( counter = initial_counter; counter < (initial_counter + NB_ITERATIVE_UPDATES); counter ++)
	{
		printf("Loop nb %d\n", counter);

		/*
		 * Update NVM_Key_3
		 */
		array_idx = 2;
		ext_key_set = 0;
		CSE_Key_idx = array_idx + CSE_KEY_1;

		printf("Load User Key %d : ", array_idx + 1);
		generate_M1_to_M5((uint32_t*)key_array[array_idx].key, master_key,
					  (uint32_t *)UID, CSE_Key_idx, CSE_MASTER_ECU_KEY,
					  counter, key_array[array_idx].fid,
					  ext_key_set,
					  M1, M2, M3, M4, M5);

		ret = CSE_LoadKey((uint8_t*)M1, (uint8_t*)M2, (uint8_t*)M3, (uint8_t*)M4, (uint8_t*)M5, ext_key_set);
		if(verbose)
		{
			display_CSE_status();
		}
		else
		{
			printf("\n");
		}


		/*
		 * Update NVM_Key_18
		 */
		array_idx = 17;
		ext_key_set = 1;

		CSE_Key_idx = array_idx + CSE_KEY_11 - 10;

		printf("Load User Key %d : ", array_idx + 1);
		generate_M1_to_M5((uint32_t*)key_array[array_idx].key, master_key,
					  (uint32_t *)UID, CSE_Key_idx, CSE_MASTER_ECU_KEY,
					  counter, key_array[array_idx].fid,
					  ext_key_set,
					  M1, M2, M3, M4, M5);

		ret = CSE_LoadKey((uint8_t*)M1, (uint8_t*)M2, (uint8_t*)M3, (uint8_t*)M4, (uint8_t*)M5, ext_key_set);
		ret = ret; /* only to remove compilation warning */
		if(verbose)
		{
			display_CSE_status();
			printf("\n");
		}
		else
		{
			printf("\n");
		}

	}
}


/**
 * @brief          Loads default key set, with known values and different flags
 * @details        Loads a Set of key allowing to test various usage restriction conditions
 *
 * @param[in]      verbose		enable display of input, computed and expected values when set to 1
 *
 * @note		   Will only work if keys are empty
 * @note		   No key has the Write Protect flag
 */
void load_default_key_set( uint32_t verbose )
{
	uint32_t M1[4];
	uint32_t M2[8];
	uint32_t M3[4];
	uint32_t M4[8];
	uint32_t M5[4];
	uint32_t array_idx;
	uint32_t CSE_Key_idx;
	vuint32_t UID[4]  = { 0x00000000, 0x00000000, 0x00000000, 0x00000000 };
	vuint32_t status = 0;
	vuint32_t MAC[4] = { 0x00000000, 0x00000000, 0x00000000, 0x00000000 };
	uint32_t ret;
	int i;
	uint32_t ext_key_set;
	uint32_t cid = 1;
	uint32_t fid = 0;
	uint32_t use_wildcard = 0;

if(verbose)
{
	printf("\n\nWriting default key values \n\n");
	printf(" Note that this function will only update empty keys since new counter is 1 \n");
	printf(" And should return ERC 8 if key is already set (due to counter) or ERC 7 (key Write protected) \n");
	printf(" Depending on key status, some update may not succeed\n");
	printf("\n");
}
	/* Clear buffers */
	for(i=0;i<4;i++)
	{
		UID[i] = 0x00000000;
		MAC[i] = 0x00000000;
	}
	status = 0;

	get_int("Using Chip UID (0) or Wildcard UID (1) ?  ", &use_wildcard);

	if( use_wildcard == 1 )
	{
		for(i=0;i<4;i++) {
			UID[i] = 0x00000000;
		}
	}
	else
	{
		ret = CSE_GetId( challenge, (uint32_t*)UID, (uint32_t*)&status, (uint32_t*)MAC);
	}
    if(verbose)
    {
        display_buf("Used UID : ", (uint8_t*)UID, 15);

        printf("Load Master Key\n");
    }
	/* load master key, assuming it is empty */
	ext_key_set = 0;
#if 0
    display_buf("UID      : ", (uint8_t*)UID, 15);
    display_buf("new key  : ", (uint8_t*)master_key, 16);
    display_buf("aut key  : ", (uint8_t*)empty_key, 16);
    printf("key id : %d ", CSE_MASTER_ECU_KEY);
    printf("aut id : %d ", CSE_MASTER_ECU_KEY);
    printf("flags  : %x ", fid);
    printf("count  : %x ", cid);
    printf("ext key set : %x ", ext_key_set);
    printf("\r\n");
#endif
	generate_M1_to_M5(master_key, empty_key,
                      (uint32_t *)UID,
                      CSE_MASTER_ECU_KEY, CSE_MASTER_ECU_KEY,
                      cid, fid,
                      ext_key_set,
                      M1, M2, M3, M4, M5);
	if(verbose)
	{
        display_buf("M1 : ", (uint8_t*)M1, 16);
        display_buf("M2 : ", (uint8_t*)M2, 32);
        display_buf("M3 : ", (uint8_t*)M3, 16);
        display_buf("M4 : ", (uint8_t*)M4, 32);
        display_buf("M5 : ", (uint8_t*)M5, 16);
        printf("\r\n");
	}
	ret = CSE_LoadKey((uint8_t*)M1, (uint8_t*)M2, (uint8_t*)M3, (uint8_t*)M4, (uint8_t*)M5, ext_key_set);
	if(verbose)
	{
		printf("ret : %x\n", ret );
		display_CSE_status();
        printf("\n");

    /* load boot mac key */
        printf("Boot Mac Key\n");
	}
	ext_key_set = 0;
#if 0
    display_buf("UID      : ", (uint8_t*)UID, 15);
    display_buf("new key  : ", bootmac_key, 16);
    display_buf("aut key  : ", (uint8_t*)master_key, 16);
    printf("key id : %d ", CSE_BOOT_MAC_KEY);
    printf("aut id : %d ", CSE_MASTER_ECU_KEY);
    printf("flags  : %x ", fid);
    printf("count  : %x ", cid);
    printf("ext key set : %x ", ext_key_set);
    printf("\r\n");
#endif
	generate_M1_to_M5((uint32_t*)bootmac_key, master_key,
					  (uint32_t *)UID,
					  CSE_BOOT_MAC_KEY, CSE_MASTER_ECU_KEY,
					  cid, 0,
					  ext_key_set,
					  M1, M2, M3, M4, M5);
    if(verbose)
    {
        display_buf("M1 : ", (uint8_t*)M1, 16);
        display_buf("M2 : ", (uint8_t*)M2, 32);
        display_buf("M3 : ", (uint8_t*)M3, 16);
        display_buf("M4 : ", (uint8_t*)M4, 32);
        display_buf("M5 : ", (uint8_t*)M5, 16);
	    printf("\r\n");
    }
	ret = CSE_LoadKey((uint8_t*)M1, (uint8_t*)M2, (uint8_t*)M3, (uint8_t*)M4, (uint8_t*)M5, ext_key_set);
	if(verbose)
	{
		printf("ret : %x\n", ret );
		display_CSE_status();
	}
	printf("\n");

	/* load user keys 1..10 */
	ext_key_set = 0;
	for(array_idx=0; array_idx<10; array_idx++)
	{
		CSE_Key_idx = array_idx + CSE_KEY_1;


		printf("Load User Key %d\n", array_idx + 1);
#if 0
	    display_buf("UID      : ", (uint8_t*)UID, 15);
	    display_buf("new key  : ", key_array[array_idx].key, 16);
	    display_buf("aut key  : ", (uint8_t*)master_key, 16);
	    printf("key id : %d ", CSE_Key_idx);
	    printf("aut id : %d ", CSE_MASTER_ECU_KEY);
	    printf("flags  : %x ", key_array[array_idx].fid);
	    printf("count  : %x ", cid);
	    printf("ext key set : %x ", ext_key_set);
	    printf("\r\n");
#endif
	    generate_M1_to_M5((uint32_t*)key_array[array_idx].key, master_key,
                      (uint32_t *)UID, CSE_Key_idx, CSE_MASTER_ECU_KEY,
                      cid, key_array[array_idx].fid,
                      ext_key_set,
                      M1, M2, M3, M4, M5);
		display_buf("M1 : ", (uint8_t*)M1, 16);
		display_buf("M2 : ", (uint8_t*)M2, 32);
		display_buf("M3 : ", (uint8_t*)M3, 16);
		display_buf("M4 : ", (uint8_t*)M4, 32);
		display_buf("M5 : ", (uint8_t*)M5, 16);
		printf("\r\n");

		ret = CSE_LoadKey((uint8_t*)M1, (uint8_t*)M2, (uint8_t*)M3, (uint8_t*)M4, (uint8_t*)M5, ext_key_set);
		printf("ret : %x\n", ret );
		if(verbose)
		{
			display_CSE_status();
		}
		printf("\n");
	}

	/* load user keys 11..20 */
	ext_key_set = 1;
	for(array_idx=10; array_idx<20; array_idx++)
	{
		CSE_Key_idx = array_idx + CSE_KEY_11 - 10;

		printf("Load User Key %d\n", array_idx + 1);
#if 0
		display_buf("UID      : ", (uint8_t*)UID, 15);
	    display_buf("new key  : ", key_array[array_idx].key, 16);
	    display_buf("aut key  : ", (uint8_t*)master_key, 16);
	    printf("key id : %d ", CSE_Key_idx);
	    printf("aut id : %d ", CSE_MASTER_ECU_KEY);
	    printf("flags  : %x ", key_array[array_idx].fid);
	    printf("count  : %x ", cid);
	    printf("ext key set : %x ", ext_key_set);
	    printf("\r\n");
#endif
		generate_M1_to_M5((uint32_t*)key_array[array_idx].key, master_key,
					  (uint32_t *)UID, CSE_Key_idx, CSE_MASTER_ECU_KEY,
					  cid, key_array[array_idx].fid,
					  ext_key_set,
					  M1, M2, M3, M4, M5);
		display_buf("M1 : ", (uint8_t*)M1, 16);
		display_buf("M2 : ", (uint8_t*)M2, 32);
		display_buf("M3 : ", (uint8_t*)M3, 16);
		display_buf("M4 : ", (uint8_t*)M4, 32);
		display_buf("M5 : ", (uint8_t*)M5, 16);
		printf("\r\n");

		ret = CSE_LoadKey((uint8_t*)M1, (uint8_t*)M2, (uint8_t*)M3, (uint8_t*)M4, (uint8_t*)M5, ext_key_set);
		printf("ret : %x\n", ret );
		//if(verbose)
		{
			display_CSE_status();
		}
		printf("\n");
	}
}


/**
 * @}
 */
