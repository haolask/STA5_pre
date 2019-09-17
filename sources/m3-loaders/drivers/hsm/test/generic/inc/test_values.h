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
 * @file    test_values.h
 * @brief   CSE tests reference values - header
 * @details
 *
 *
 * @addtogroup CSE_driver_test
 * @{
 */

#ifndef TEST_VALUES_INCLUDED_H
#define TEST_VALUES_INCLUDED_H

#include "cse_typedefs.h"

typedef struct
{
    uint8_t* key;
    uint32_t fid;
}key_desc_stt;


extern uint32_t empty_key[4];

extern uint32_t iv[4];
extern uint32_t mac[4];
extern uint32_t exp_mac[4];

extern uint32_t challenge[4];
extern uint32_t master_key[4];
extern vuint32_t ID[4];


/* !!!! If we keep the Host/CSE emulated registers shared area in the middle of system RAM, we can not test with 64KB payload  */
extern uint8_t src_large[32768];


extern uint8_t master_key_byte[16];
extern uint8_t bootmac_key[16];

extern uint8_t Key_1[16];
extern uint8_t Key_2[16];
extern uint8_t Key_3[16];
extern uint8_t Key_4[16];
extern uint8_t Key_5[16];
extern uint8_t Key_6[16];
extern uint8_t Key_7[16];
extern uint8_t Key_8[16];
extern uint8_t Key_9[16];
extern uint8_t Key_10[16];

/* To avoid confusion and detect any normal/extended key set mix,
 * set key11..20 in reverse order compared to 1..10
 */
extern uint8_t Key_11[16];
extern uint8_t Key_12[16];
extern uint8_t Key_13[16];
extern uint8_t Key_14[16];
extern uint8_t Key_15[16];
extern uint8_t Key_16[16];
extern uint8_t Key_17[16];
extern uint8_t Key_18[16];
extern uint8_t Key_19[16];
extern uint8_t Key_20[16];

/* Flags is on 8 bits, with the 2 lsb bits set to 0
 * WP|BP|DP|KU|WC|VO|0|0
 */
extern key_desc_stt key_array[20];

/*
 *  Secure Boot test values
 */
#define BL_WORD_SZ  ((uint32_t)32)
extern uint32_t ref_boot_loader[BL_WORD_SZ];

extern uint32_t ref_boot_loader_size;
extern uint32_t* corrupted_boot_loader;
extern uint32_t  corrupted_boot_loader_size;

extern uint32_t ref_boot_mac[4];

extern void display_CSE_status(void);
extern void display_pass_fail(int pass);
#endif
/**
 * @}
 */
