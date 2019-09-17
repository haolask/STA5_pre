/*
    SPC5-CRYPTO - Copyright (C) 2015 STMicroelectronics

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
 * @file    CSE_ext_RSA_PKCS_test.h
 * @brief   pkcs signature tests - header file
 * @details
 *
 *
 * @addtogroup CSE_driver_test
 * @{
 */
#include "cse_types.h"

extern uint32_t M3[4];

extern uint32_t M4[8];

extern uint32_t M5[4];

extern uint32_t resM4[8];

extern uint32_t resM5[4];

extern uint32_t pkcs_signature_test(uint32_t verbose, uint32_t reduced_tv_nb );

extern uint32_t pkcs_encryption_test(uint32_t verbose);

extern uint32_t pkcs_signature_notSupportedHash_test (uint32_t P_verbose);

extern uint32_t pkcs_signatureWithProtectedKeys_test(uint32_t P_verbose, uint32_t reduced_tv_nb );

extern uint32_t pkcs_encryptionWithProtectedKeys_test(uint32_t P_verbose, uint32_t reduced_test_vector_nb );

extern uint32_t pkcs_signatureAllSha_test( uint32_t verbose, uint32_t reduced_tv_nb );

extern uint32_t pkcs_allSha_signatureWithProtectedKeys_test( uint32_t P_verbose, uint32_t reduced_tv_nb );

extern uint32_t pkcs_allSha_signatureVerifyWithProtectedKeys_test( uint32_t P_verbose, uint32_t reduced_tv_nb );

extern uint32_t pkcs_allSha_signatureVerifyRamKeys_test (uint32_t P_verbose, uint32_t reduced_tv_nb );

/**
 * @}
 */
