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

/**
 * @file    CSE_Constants.h
 * @brief   CSE-HSM driver constants values
 * @details Definition of all constants used in the CSE driver
 *          (Commands and Key indexes, error codes)
 *
 *
 * @addtogroup CSE_driver
 * @{
 */

#ifndef _CSE_CONSTANTS_H_
#define _CSE_CONSTANTS_H_

/**
 * @addtogroup CSE_command_indexes Command indexes
 * @{
 */
/**
 * @name    CSE Commands indexes
 * @{
 * @brief   Definition of all CSE HW supported commands identifiers
 * @note    These are the values to provide in the CMD register to invoke a
 *          particular operation
 */
#define CSE_ENC_ECB           0x01
#define CSE_ENC_CBC           0x02
#define CSE_DEC_ECB           0x03
#define CSE_DEC_CBC           0x04
#define CSE_GENERATE_MAC      0x05
#define CSE_VERIFY_MAC        0x06
#define CSE_LOAD_KEY          0x07

#define CSE_ENC_ECB_EXT       0x81
#define CSE_ENC_CBC_EXT       0x82
#define CSE_DEC_ECB_EXT       0x83
#define CSE_DEC_CBC_EXT       0x84
#define CSE_GENERATE_MAC_EXT  0x85
#define CSE_VERIFY_MAC_EXT    0x86
#define CSE_LOAD_KEY_EXT      0x87

#define CSE_LOAD_PLAIN_KEY    0x08
#define CSE_EXPORT_RAM_KEY    0x09
#define CSE_INIT_RNG          0x0A
#define CSE_EXTEND_SEED       0x0B
#define CSE_RND               0x0C
#define CSE_SECURE_BOOT       0x0D
#define CSE_BOOT_FAILURE      0x0E
#define CSE_BOOT_OK           0x0F
#define CSE_GET_ID            0x10
#define CSE_CANCEL            0x11
#define CSE_DEBUG_CHAL        0x12
#define CSE_DEBUG_AUTH        0x13
#define CSE_TRNG_RND          0x14
#define CSE_INIT_CSE          0x15

#define CSE_GET_FWID          0x16
#define CSE_TRNG_ONLINETST    0x17

/* RSA Command Identifiers */
#define CSE_RSA_LOAD_RAM_KEY			((uint32_t)0x40)
#define CSE_RSA_LOAD_KEY			((uint32_t)0x41)
#define CSE_RSA_GENERATE_LOAD_KEY_PAIR		((uint32_t)0x42)
#define CSE_RSA_EXPORT_PUBLIC_KEY		((uint32_t)0x43)
#define CSE_RSA_EXPORT_PRIVATE_KEY		((uint32_t)0x44)
#define CSE_RSA_PKCS1_15_SIGN			((uint32_t)0x45)
#define CSE_RSA_PKCS1_15_VERIFY			((uint32_t)0x46)
#define CSE_RSA_PKCS1_15_ENCRYPT		((uint32_t)0x47)
#define CSE_RSA_PKCS1_15_DECRYPT		((uint32_t)0x48)

/* HASH Command Identifiers */
#define CSE_HASH				((uint32_t)0x50)

/* TLS command Indentifiers */
#define CSE_TLSV12_PRF				((uint32_t)0x55)

/* ECC Command Identifiers */
#define CSE_ECC_LOAD_RAM_KEY			((uint32_t)0x60)
#define CSE_ECC_LOAD_KEY			((uint32_t)0x61)
#define CSE_ECC_GENERATE_LOAD_KEY_PAIR		((uint32_t)0x62)
#define CSE_ECC_EXPORT_PUBLIC_KEY		((uint32_t)0x63)
#define CSE_ECC_EXPORT_PRIVATE_KEY		((uint32_t)0x64)
#define CSE_ECC_ECDSA_SIGN			((uint32_t)0x65)
#define CSE_ECC_ECDSA_VERIFY			((uint32_t)0x66)
#define CSE_ECC_ECIES_ENCRYPT			((uint32_t)0x67)
#define CSE_ECC_ECIES_DECRYPT			((uint32_t)0x68)
#define CSE_ECC_ECDH_KEY_AGREEMENT		((uint32_t)0x69)
#define CSE_ECC_ECDHfixed_KEY_AGREEMENT		((uint32_t)0x6A)
#define CSE_ECC_ECDHfixed_SERVER_KEY_AGREEMENT	((uint32_t)0x6B)
#define CSE_ECC_CHANGE_EC_CURVE			((uint32_t)0x6E)
#define CSE_ECC_GET_CURRENT_EC_CURVE		((uint32_t)0x6F)

/* AES256 Command Identifiers */
#define CSE_AES256_LOAD_RAM_KEY			((uint32_t)0x90)
#define CSE_AES256_EXPORT_RAM_KEY		((uint32_t)0x91)
#define CSE_AES256_LOAD_KEY			((uint32_t)0x92)
#define CSE_AES256_ECB_ENCRYPT			((uint32_t)0x93)
#define CSE_AES256_ECB_DECRYPT			((uint32_t)0x94)
#define CSE_AES256_CBC_ENCRYPT			((uint32_t)0x95)
#define CSE_AES256_CBC_DECRYPT			((uint32_t)0x96)
#define CSE_AES256_CCM_ENCRYPT			((uint32_t)0x97)
#define CSE_AES256_CCM_DECRYPT			((uint32_t)0x98)
#define CSE_AES256_CMAC_GENERATE		((uint32_t)0x99)
#define CSE_AES256_CMAC_VERIFY			((uint32_t)0x9A)
#define CSE_AES256_GCM_ENCRYPT			((uint32_t)0x9B)
#define CSE_AES256_GCM_DECRYPT			((uint32_t)0x9C)

/* HMAC Command Identifiers */
#define CSE_HMAC_LOAD_RAM_KEY			((uint32_t)0xC0)
#define CSE_HMAC_EXPORT_RAM_KEY			((uint32_t)0xC1)
#define CSE_HMAC_LOAD_KEY			((uint32_t)0xC2)
#define CSE_HMAC_GENERATE			((uint32_t)0xC3)
#define CSE_HMAC_VERIFY				((uint32_t)0xC4)

/* CSE internal tests */
#define CSE_RNG_CONF4_INTERNAL_TESTS    	((uint32_t)0x70)

/*
 * Telemaco3p specific commands
 */
#define CSE_OTP_READ_PUBLIC                      ((uint32_t)0xA0)
#define CSE_OTP_WRITE_PUBLIC                     ((uint32_t)0xA1)
#define CSE_OTP_GET_CONFIG                       ((uint32_t)0xA2)
#define CSE_OTP_SET_CONFIG                       ((uint32_t)0xA3)
#define CSE_GET_RAM_MONOTONIC                    ((uint32_t)0xA4)
#define CSE_SET_RAM_MONOTONIC                    ((uint32_t)0xA5)
#define CSE_OTP_WRITE_SEC_BOOT_KEY               ((uint32_t)0xA6)
#define CSE_EXT_IMPORT_KEY_IMAGE                 ((uint32_t)0xA7)
#define CSE_EXT_SET_VALID_EXT_MEM_RANGE          ((uint32_t)0xA8)
#define CSE_EXT_LOCK_EXT_MEM_RANGE               ((uint32_t)0xA9)
#define CSE_EXT_UPDATE_SERVICE_SET               ((uint32_t)0xAA)
#define CSE_EXT_SET_M3_SHE_REGISTERS_MEM_CONFIG  ((uint32_t)0xAC)
#define CSE_EXT_LOCK_M3_SHE_REGISTERS_MEM_CONFIG ((uint32_t)0xAD)
#define CSE_EXT_SET_A7_SHE_REGISTERS_MEM_CONFIG  ((uint32_t)0xAE)
#define CSE_EXT_LOCK_A7_SHE_REGISTERS_MEM_CONFIG ((uint32_t)0xAF)

/** @} */
/** @} */

/**
 * @addtogroup CSE_Key_indexes Key indexes
 * @{
 */
/**
 * @name    CSE Key indexes
 * @{
 */
#define CSE_SECRET_KEY     0x0
#define CSE_MASTER_ECU_KEY 0x1
#define CSE_BOOT_MAC_KEY   0x2
#define CSE_BOOT_MAC       0x3
#define CSE_KEY_1          0x4
#define CSE_KEY_2          0x5
#define CSE_KEY_3          0x6
#define CSE_KEY_4          0x7
#define CSE_KEY_5          0x8
#define CSE_KEY_6          0x9
#define CSE_KEY_7          0xA
#define CSE_KEY_8          0xB
#define CSE_KEY_9          0xC
#define CSE_KEY_10         0xD
#define CSE_RAM_KEY        0xE

#define CSE_KEY_11         0x4
#define CSE_KEY_12         0x5
#define CSE_KEY_13         0x6
#define CSE_KEY_14         0x7
#define CSE_KEY_15         0x8
#define CSE_KEY_16         0x9
#define CSE_KEY_17         0xA
#define CSE_KEY_18         0xB
#define CSE_KEY_19         0xC
#define CSE_KEY_20         0xD
/** @} */

/**
 * @name    CSE RSA Key indexes
 * @{
 */

#define RSA_RAM_KEY         ((uint32_t)0x0U)
#define RSA_KEY_1           ((uint32_t)0x1U)
#define RSA_KEY_2           ((uint32_t)0x2U)
#define RSA_KEY_3           ((uint32_t)0x3U)
#define RSA_KEY_4           ((uint32_t)0x4U)
#define RSA_KEY_MAX_IDX     RSA_KEY_4
/** @} */

/**
 * @name    CSE ECC Key indexes
 * @{
 */

/* 4 ECC NVM keys and one ECC RAM key */
#define NB_OF_CSE_ECC_KEYS ((uint32_t)(5))

#define ECC_RAM_KEY_IDX         ((uint32_t)0x0U)
#define ECC_KEY_1_IDX           ((uint32_t)0x1U)
#define ECC_KEY_2_IDX           ((uint32_t)0x2U)
#define ECC_KEY_3_IDX           ((uint32_t)0x3U)
#define ECC_KEY_4_IDX           ((uint32_t)0x4U)
#define ECC_KEY_MAX_IDX         ECC_KEY_4_IDX
/** @} */

/**
 * @name    CSE AES256 Key indexes
 * @{
 */

/* 4 AES256 NVM keys and one AES256 RAM key */
#define NB_OF_CSE_AES256_KEYS	((uint32_t)(5))

#define AES256_RAM_KEY_IDX	((uint32_t)0x0U)
#define AES256_KEY_1_IDX	((uint32_t)0x1U)
#define AES256_KEY_2_IDX	((uint32_t)0x2U)
#define AES256_KEY_3_IDX	((uint32_t)0x3U)
#define AES256_KEY_4_IDX	((uint32_t)0x4U)
#define AES256_KEY_MAX_IDX	AES256_KEY_4_IDX
/** @} */

/* 4 HMAC NVM keys and one HMAC RAM key */
#define NB_OF_CSE_AES256_KEYS	((uint32_t)(5))
#define HMAC_RAM_KEY_IDX	((uint32_t)0x0U)
#define HMAC_KEY_1_IDX		((uint32_t)0x1U)
#define HMAC_KEY_2_IDX		((uint32_t)0x2U)
#define HMAC_KEY_3_IDX		((uint32_t)0x3U)
#define HMAC_KEY_4_IDX		((uint32_t)0x4U)
#define HMAC_KEY_LAST_IDX	HMAC_KEY_4_IDX

/**
 * @addtogroup CSE_Error_codes Error codes
 * @{
 */
/**
 * @name    CSE Error codes values
 * @{
 */
#define CSE_NO_ERR                0x00
#define CSE_CMD_SEQ_ERR           0x02
#define CSE_KEY_NOT_AVAIL         0x03
#define CSE_INV_KEY               0x04
#define CSE_EMPTY_KEY             0x05
#define CSE_NO_SECURE_BOOT        0x06
#define CSE_KEY_WRITE_PROTECTED   0x07
#define CSE_KEY_UPDATE_ERR        0x08
#define CSE_RND_NUM_SEED_NOT_INIT 0x09
#define CSE_INT_DEBUG_NOT_ALLOWED 0x0A
#define CSE_CMD_ISSUED_WHILE_BUSY 0x0B
#define CSE_SYSTEM_MEMORY_ERR     0x0C
#define CSE_GENERAL_ERR           0x0D
#define CSE_INT_MEMORY_ERR        0x10
#define CSE_INVALID_CMD           0x11
#define CSE_INVALID_PARAMETER     0x12
#define CSE_ERR_READ_CSE_FL_BLOCK 0x13
#define CSE_INT_CMD_PROC_ERR      0x14
#define CSE_LENGTH_ERR            0x15

/* CSE-SHE extension error code */
/** @brief Returned upon attempt to load a key in a non empty slot */
#define ERC_KEY_NOT_EMPTY       ((uint32_t)0x20)
/** @brief Returned if hash type is not a supported value */
#define ERC_INVALID_HASH_ALGO   ((uint32_t)0x21)
/**
 * @brief Returned when Tags comparison in ECIES Decryption or CCM decryption
 * do not match
 */
#define ERC_ENCRYPTION_INVALID  ((uint32_t)0x25)
/** @brief Returned when an asymmetric key is not in correct range */
#define ERC_ASYM_KEY_INVALID    ((uint32_t)0x26)

/** @} */
/** @} */

#endif //_CSE_CONSTANTS_H_
/**
 * @}
 */
