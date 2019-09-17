/*
 *  Copyright (C) 2018 STMicroelectronics
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
 * @file    lspayload.h
 * @brief   Load/store unit payload definition header file.
 *
 * @addtogroup LSPAYLOAD
 * @{
 */

#ifndef _LSPAYLOAD_H_
#define _LSPAYLOAD_H_

#include <stddef.h>

#include "rpmx_common.h"
//#include "crypto_proxy.h" // FIXME

/**
 * @brief   Size of a signature.
 */
#define CRYPTO_SIGNATURE_SIZE   32 // FIXME: use crypto_proxy.h instead

/*
 * Number of type 1 keys in storage.
 */
#define NUM_KEY_TYPE1     23U

/*
 * Number of type 2 keys in storage.
 */
#define NUM_KEY_TYPE2     4U

/*
 * Number of type 3 keys in storage.
 */
#define NUM_KEY_TYPE3     4U

/*
 * Number of type 4 keys in storage.
 */
#define NUM_KEY_TYPE4     4U

/**
 * @brief   Number of type 5 keys in storage.
 */
#define NUM_KEY_TYPE5     1U

/*
 * Space used by a AES128 key.
 */
#define KEY_SIZE_TYPE1    (16 + 4 + 4)

/*
 * Space used by a AES256 key.
 */
#define KEY_SIZE_TYPE2    (32 + 4 + 4)

/*
 * @brief   Space used by a ECC key.
 *          Size & offset for public (X & Y) & private key fields
 *              --> 3 x (16 + 16 bits): 12
 *          Buffer for public (X & Y) & private key values (521bit max each
 *              --> 3 x 68 bytes): 204
 *          1 word for rollback counter: 4
 *          1 word for key attributes (flags) : 4
 */
#define KEY_SIZE_TYPE3    (12 + 204 + 4 + 4)

/*
 * @brief   Space used by a RSA key. (size + offset for pub exp,
 *                                    priv exp & modulus,
 *                                    priv exp & modulus (384 each),
 *                                    pub exp max size (32 bytes),
 *                                    counter, flag)
 *
 */
#define KEY_SIZE_TYPE4    (12 + 768 + 32 + 4 + 4)

/**
 * @brief   Space used by a HMAC  key. (128 bytes max per key, counter, flag)
 *
 */
#define KEY_SIZE_TYPE5    (8 + 128 + 4 + 4)

/*
 * @brief Type of a type 1 key container.
 */
struct payload_type1_key_t {
	/* Key attributes. */
	uint32_t type;
	/* Key size, zero if key is not used. */
	size_t size;
	/* Key buffer. */
	uint8_t key[KEY_SIZE_TYPE1];
};

/*
 * @brief Type of a type 2 key container.
 */
struct payload_type2_key_t {
	/* Key attributes. */
	uint32_t type;
	/* Key size, zero if key is not used. */
	size_t size;
	/* Key buffer. */
	uint8_t key[KEY_SIZE_TYPE2];
};

/*
 * @brief Type of a type 3 key container.
 */
struct payload_type3_key_t {
	/* Key attributes. */
	uint32_t type;
	/* Key size, zero if key is not used. */
	size_t size;
	/* Key buffer. */
	uint8_t key[KEY_SIZE_TYPE3];
};

/*
 * @brief Type of a type 4 key container.
 */
struct payload_type4_key_t {
	/* Key attributes. */
	uint32_t type;
	/* Key size, zero if key is not used. */
	size_t size;
	/* Key buffer. */
	uint8_t key[KEY_SIZE_TYPE4];
};

/*
 * @brief Type of a type 5 key container.
 */
struct payload_type5_key_t {
	/* Key attributes. */
	uint32_t type;
	/* Key size, zero if key is not used. */
	size_t size;
	/* Key buffer. */
	uint8_t key[KEY_SIZE_TYPE5];
};

/*
 * @brief Type of a flash block payload.
 */
struct lspayload_t {
	struct {
		/* Monotonic counter stamp for this payload. */
		mcvalue_t cnt;
		/* Required to have the key image size multiple of 16 */
		uint32_t padding[3];
		/* Array of the type 1 keys. */
		struct payload_type1_key_t type1[NUM_KEY_TYPE1];
		/* Array of the type 2 keys. */
		struct payload_type2_key_t type2[NUM_KEY_TYPE2];
		/* Array of the type 3 keys. */
		struct payload_type3_key_t type3[NUM_KEY_TYPE3];
		/* Array of the type 4 keys. */
		struct payload_type4_key_t type4[NUM_KEY_TYPE4];
		/* Array of the type 5 keys. */
		struct payload_type5_key_t type5[NUM_KEY_TYPE5];
		/* HMAC padding */
		uint32_t hmac_key_padding[2];
	} keys;
	/* Payload signature. */
	uint8_t signature[CRYPTO_SIGNATURE_SIZE];
};

#endif /* _LSPAYLOAD_H_ */

/** @} */
