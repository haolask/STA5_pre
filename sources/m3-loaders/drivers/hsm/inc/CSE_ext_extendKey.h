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
 * @file    CSE_ext_asymKey.c
 * @brief   Header file for Asymmetric Crypto
 * @details Definition common to RSA and ECC Asymmetric Key Cryptography
 *	    functions
 *
 *
 * @addtogroup SHE-ext_driver
 * @{
 */
#ifndef _CSE_ext_ASYM_KEY_INCLUDED_
#define _CSE_ext_ASYM_KEY_INCLUDED_

#include "cse_types.h"

/** @brief	Description and usage restriction flags dedicated for
 *		asymmetric keys
 *		(i.e. FID = WRITE_PROTECTION|BOOT_PROTECTION|
 *		DEBUGGER_PROTECTION|KEY_USAGE|WILDCARD|VERIFY_ONLY|00b)
 */

/* Little endian version (ARM cores) */
union EXTENDED_KEY_flags {
	uint32_t R;
	struct {
		/** @brief padding bits */
		uint32_t reserved:2;
		/**
		 * @brief Wild Card enabled: if clear the key can be updated
		 * with M1, .., M3 messages
		 * if set key update messages must be generated with the target
		 * chip UID
		 */
		uint32_t WC:1;
		/**
		 * @brief Debug Protected: if set, key can not be used for
		 * operationwhen a debugger has been attached to the platform
		 */
		uint32_t DP:1;
		/**
		 * @brief Boot Protected: if set, key cannot be used when a
		 * secure boot was not successfully performed
		 */
		uint32_t BP:1;
		/**
		 * @brief Write Protected: if clear, key can be updated; if set,
		 * prevent any further update of this key entry.
		 * It also prevent erasure of all keys
		 */
		uint32_t WP:1;
		/**
		 * @brief Key supports derivation, i.e. other keys can be
		 * derived from this key
		 */
		uint32_t derive:1;
		/** @brief Key is extractable and can be wrapped */
		uint32_t extractable:1;
		/**
		 * @brief Key supports unwrapping, i.e. can be used to unwrap
		 * other keys
		 */
		uint32_t unwrap:1;
		/**
		 * @brief Key supports wrapping, i.e. can be used to wrap other
		 * keys
		 */
		uint32_t wrap:1;
		/**
		 * @brief Key supports ONLY MAC verification for AES256 base
		 * CMAC, CCM and GCM
		 */
		uint32_t macVerifyOnly:1;
		/**
		 * @brief Key supports MAC generation and verification for
		 * AES256 base CMAC, CCM and GCM
		 */
		uint32_t mac:1;
		/** @brief Key supports decryption */
		uint32_t decrypt:1;
		/** @brief Key supports encryption */
		uint32_t encrypt:1;
		/** @brief Key supports signature verification */
		uint32_t verify:1;
		/** @brief Key supports signature generation */
		uint32_t sign:1;
		uint32_t unused:16;
	} B;
};

#endif  /* _CSE_ext_ASYM_KEY_INCLUDED_*/

/**
 * @}
 */
