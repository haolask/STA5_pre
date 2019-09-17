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
 * @file    CSE_Key_update.c
 * @brief   CSE Key update support module.
 * @details Set of functions used to compute required material to update keys.
 *
 *
 * @addtogroup CSE_personalization_support
 * @{
 */

#define CR27_ENABLED

#include "cse_typedefs.h"
#include "crypto_al.h"
#include "err_codes.h"

#include "serialprintf.h"


#undef DEBUG_PRINT

/*
 * @note Module can be mapped either on sw crypto lib or CSE HW engine. It is also usable on little or big endian platforms
 *
 */

/* BYTE arrays --> endianness independent */
const uint8_t KEY_UPDATE_ENC_C[16] =
    { 0x01, 0x01, 0x53, 0x48, 0x45, 0x00, 0x80, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb0};
const uint8_t KEY_UPDATE_MAC_C[16] =
    { 0x01, 0x02, 0x53, 0x48, 0x45, 0x00, 0x80, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb0};
const uint8_t ASYMKEY_UPDATE_ENC_C[16] =
    { 0x01, 0x41, 0x53, 0x48, 0x45, 0x00, 0x80, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb0};

const uint8_t KEY_UPDATE_ENC_C_EXT[16] =
    { 0x01, 0x81, 0x53, 0x48, 0x45, 0x00, 0x80, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb0};
const uint8_t KEY_UPDATE_MAC_C_EXT[16] =
    { 0x01, 0x82, 0x53, 0x48, 0x45, 0x00, 0x80, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb0};

const uint8_t IV0_Key[16] =
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

vuint32_t K1[4];
vuint32_t m1[4];
vuint32_t m2[8];
vuint32_t m3[4];
vuint32_t m4[8];
vuint32_t m5[4];
char K2[16];
char K3[16];
char K4[16];

/*================================================================================================*/
/**
 * @brief          Key Derivation Function
 * @details        Loads a key value to be used when selecting index 0xE (RAM_KEY)
 *
 * @param[in]      UserKey		pointer to value to derive from
 * @param[in]      Key_Constant	pointer to constant to use for derivation
 * @param[out]     KeyOut 		pointer to derived Key
 *
 * @return         Error code
 * @retval 0  	  When key was loaded properly
 * @retval xx      In case of error
 *
 * @note           The function can use either the CSE HW or the sw crypto library to perform the AES operations
 * 				  hidden thanks to an abstraction function AES_ECB_Encrypt
 */
int KDF(uint8_t* UserKey, uint8_t* Key_Constant, uint8_t* KeyOut) {
  uint8_t out1[16];
  uint8_t out2[16];

  int j = 0;
  uint32_t ret;

  /* 1st KDF step:
   * Encryption of Key to derive in ECB with key equal to zero (also called IV0)
   * XOR result with Key to derive and key (IV0) */
  ret = AES_ECB_Encrypt(UserKey, 16, (uint8_t *)IV0_Key, 16, out1);
  if (ret == AES_SUCCESS) {
    /* Compute XOR of previous result with "Key to derive" and previous result (key = OUT(i-1)) */
    for (j = 0; j < 16; j++) {
      out1[j] = out1[j] ^ UserKey[j] ^ IV0_Key[j];
    }

    /* 2nd KDF step:
     * Encryption of Constant in ECB with key being result of previous step (out1)
     * XOR result with Constant and result of previous step ( key = out1) */
    ret = AES_ECB_Encrypt(Key_Constant, 16, (uint8_t *)out1, 16, out2);
    if (ret == AES_SUCCESS) {
      /* Compute XOR of previous result with "Key to derive" and previous result (key = OUT(i-1)) */
      for (j = 0; j < 16; j++) {
        KeyOut[j] = out2[j] ^ Key_Constant[j] ^ out1[j];
      }
    }
  }
  return ret;
}

/*================================================================================================*/
/**
 * @brief          Generate the messages required to load a Key in the NVM using CSE_LOAD_KEY
 * @details        Computes the formatted messages and associated integrity and authenticity MACs
 *
 * @param[in]      new_key	pointer to cleartext key value (128bits)
 * @param[in]      k_authid	pointer to cleartext authorization key value (128bits)
 * @param[in]      uid		pointer to chip unique ID (120bits)
 * @param[in]      id     	Key index where the key will have to be stored in NVM
 * @param[in]      authid 	Authentication Key index to be used for authentication check when loading
 * @param[in]      cid 		Counter value
 * @param[in]      fid     	Security flags to store with the Key
 *
 * @param[in]      ext_key_set	1 to use extended key set, 0 for normal key set
 *
 * @param[out]     m1		pointer to message1
 * @param[out]     m2		pointer to message2
 * @param[out]     m3		pointer to message3
 * @param[out]     m4		pointer to message4
 * @param[out]     m5		pointer to message5
 *
 * @return         Error code
 * @retval 0  	  When key was loaded properly
 * @retval 1..21   In case of error - the error code values are the CSE returned ones
 *
 * @note           This function is not intended to be used on-board, but rather in an offline program
 * 				  It requires the knowledge of the authentication key
 * 				  M4 and M5 are not required to load a key, but they can be used as reference to check key_load succeeded
 */
uint32_t generate_M1_to_M5(uint32_t* new_key, uint32_t* k_authid, uint32_t* uid,
                           uint8_t id, uint8_t authid,
                           uint32_t cid, uint8_t fid,
						   uint32_t ext_key_set,
						   uint32_t* m1, uint32_t* m2, uint32_t* m3, 
                           uint32_t* m4, uint32_t* m5) {
  uint32_t initial_value_cbc[4] = {0, 0, 0, 0};
  uint32_t m2_input[8];
  uint32_t m3_input[12];
  uint32_t m4_star_input[4];
  uint32_t m4_star[4];

  uint8_t* m1_byte_array;
  uint8_t* m2_input_byte_array;
  uint8_t* uid_byte_array;
  int ret;

  m1_byte_array = (uint8_t*)m1;
  m2_input_byte_array = (uint8_t*)m2_input;
  uid_byte_array = (uint8_t*)uid;

  /* Derive K1 and K2 keys */
  if(ext_key_set)
  {
  	  KDF((uint8_t *)k_authid, (uint8_t *)KEY_UPDATE_ENC_C_EXT, (uint8_t *)K1);
  	  KDF((uint8_t *)k_authid, (uint8_t *)KEY_UPDATE_MAC_C_EXT, (uint8_t *)K2);
  }
  else
  {
	  KDF((uint8_t *)k_authid, (uint8_t *)KEY_UPDATE_ENC_C, (uint8_t *)K1);
	  KDF((uint8_t *)k_authid, (uint8_t *)KEY_UPDATE_MAC_C, (uint8_t *)K2);
  }


#ifdef DEBUG_PRINT
  display_buf( "k_authid : ", (uint8_t*)k_authid, 16);
  display_buf( "KUPDENCC : ", (uint8_t*)KEY_UPDATE_ENC_C, 16);
  display_buf( "K1 :       ", (uint8_t*)K1, 16);
  display_buf( "K2 :       ", (uint8_t*)K2, 16);
#endif

  /* uid is 120 bits */
  /* Counter (cid) is 28 bits */
  /* id is 4 bits */
  /* authid is 4 bits */

  /* Fill M1 */
  m1[0] = uid[0];
  m1[1] = uid[1];
  m1[2] = uid[2];

  m1_byte_array[12] = uid_byte_array[12];
  m1_byte_array[13] = uid_byte_array[13];
  m1_byte_array[14] = uid_byte_array[14];
  m1_byte_array[15] = ((id & 0x0F) << 4) + (authid & 0x0F);


  /* Fill M2 cleartext */
  m2_input_byte_array[0] = (cid & 0x0FF00000) >> 20;
  m2_input_byte_array[1] = (cid & 0x000FF000) >> 12;
  m2_input_byte_array[2] = (cid & 0x00000FF0) >> 4;
  m2_input_byte_array[3] = ((cid & 0x0000000F) << 4) + ((fid & 0xF0) >> 4);

  m2_input[1] = 0;
  m2_input_byte_array[4] = (fid & 0x0F) << 4;


  m2_input[2] = 0;
  m2_input[3] = 0;
  m2_input[4] = new_key[0];
  m2_input[5] = new_key[1];
  m2_input[6] = new_key[2];
  m2_input[7] = new_key[3];

#ifdef DEBUG_PRINT
  display_buf( "m2_inp : ", (uint8_t*)m2_input, 32);
#endif

  /* Encrypt M2, AES CBC mode, Key = K1, IV = 0 */
  ret = AES_CBC_Encrypt((uint8_t *)m2_input,
                        32,
                        (const uint8_t *)K1,
                        16,
                        (const uint8_t *)initial_value_cbc,
                        (uint8_t *)m2);

#ifdef DEBUG_PRINT
  display_buf( "m2 res : ", (uint8_t*)m2, 32);
#endif

  /* Fill M3 CMAC payload */
  m3_input[0] = m1[0];
  m3_input[1] = m1[1];
  m3_input[2] = m1[2];
  m3_input[3] = m1[3];
  m3_input[4] = m2[0];
  m3_input[5] = m2[1];
  m3_input[6] = m2[2];
  m3_input[7] = m2[3];
  m3_input[8] = m2[4];
  m3_input[9] = m2[5];
  m3_input[10] = m2[6];
  m3_input[11] = m2[7];

#ifdef DEBUG_PRINT
  display_buf("m3_input: ", (uint8_t*)m3_input, 48);
#endif

  /* Compute M3 = CMAC K2( M1 | M2) */
  ret = AES_CMAC_Encrypt((uint8_t *)m3_input, 48, (uint8_t *)K2, 16, 16,
                         (uint8_t *)m3);

  /* Derive K3 and K4 */
  if(ext_key_set)
  {
	  KDF((uint8_t *)new_key, (uint8_t *)KEY_UPDATE_ENC_C_EXT, (uint8_t *)K3);
	  KDF((uint8_t *)new_key, (uint8_t *)KEY_UPDATE_MAC_C_EXT, (uint8_t *)K4);
  }
  else
  {
	  KDF((uint8_t *)new_key, (uint8_t *)KEY_UPDATE_ENC_C, (uint8_t *)K3);
	  KDF((uint8_t *)new_key, (uint8_t *)KEY_UPDATE_MAC_C, (uint8_t *)K4);
  }

#ifdef DEBUG_PRINT
  display_buf( "K3 :       ", (uint8_t*)K3, 16);
  display_buf( "K4 :       ", (uint8_t*)K4, 16);
#endif

  /* Generate M4 */
#if CRL_ENDIANNESS==2
  m4_star_input[0] = (cid << 4) + 8;
#else
  m4_star_input[0] = ((cid << 4) + 8) << 24;
#endif
  m4_star_input[1] = 0;
  m4_star_input[2] = 0;
  m4_star_input[3] = 0;

#ifdef DEBUG_PRINT
  display_buf("m4_star_input: ", (uint8_t*)m4_star_input, 16);
#endif

  ret = AES_ECB_Encrypt((uint8_t *)m4_star_input,
                        16,
                        (const uint8_t *)K3,
                        16,
                        (uint8_t *)m4_star);
  m4[0]=uid[0];
  m4[1]=uid[1];
  m4[2]=uid[2];
#if CRL_ENDIANNESS==2
  //m4[3]=uid[3] + (id <<4)  + authid;
  m4[3] = (uid[3] & 0xFFFFFF00) + ((id & 0x0F) << 4) + authid;
#else
  //p = (uint8_t*)(&uid[3]);
  //m4[3] = (p[2] << 16) + (p[1] << 8) + (p[0] ) + (((id <<4)  + authid)<<24);

  m4[3] = (uid[3] & 0x00FFFFFF) + ( ( ((id & 0x0F)<<4) + authid) <<24);
#endif

  m4[4] = m4_star[0];
  m4[5] = m4_star[1];
  m4[6] = m4_star[2];
  m4[7] = m4_star[3];

#ifdef DEBUG_PRINT
  display_buf("m4   : ", (uint8_t*)m4, 32);
#endif

  /* Compute M5 = CMAC K4(M4) */
  ret = AES_CMAC_Encrypt((uint8_t *)m4, 32, (uint8_t *)K4, 16, 16,
                         (uint8_t *)m5);

  return(ret);
}

/** @} */
