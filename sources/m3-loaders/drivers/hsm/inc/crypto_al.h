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

#define AES_CBC_Encrypt     AES_CBC_Encrypt_HW
#define AES_CBC_Decrypt     AES_CBC_Decrypt_HW
#define AES_ECB_Encrypt     AES_ECB_Encrypt_HW
#define AES_ECB_Decrypt     AES_ECB_Decrypt_HW
#define AES_CMAC_Encrypt    AES_CMAC_Encrypt_HW

/**
 * @}
 */
