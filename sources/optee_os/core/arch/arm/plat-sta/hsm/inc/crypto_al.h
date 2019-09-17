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

#include "CSE_AES_API_sc.h"

/*
 * Abstraction layer points to AES HW crypto
 */

#define aes_cbc_encrypt     aes_cbc_encrypt_hw
#define aes_cbc_decrypt     aes_cbc_decrypt_hw
#define aes_ecb_encrypt     aes_ecb_encrypt_hw
#define aes_ecb_decrypt     aes_ecb_decrypt_hw
#define aes_cmac_encrypt    aes_cmac_encrypt_hw

/**
 * @}
 */
