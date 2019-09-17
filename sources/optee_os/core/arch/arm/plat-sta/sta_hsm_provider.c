// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2017, STMicroelectronics International N.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <kernel/cache_helpers.h>
#include <mm/core_memprot.h>
#include <mm/tee_mm.h>
#include <tee_api_types.h>
#include <crypto/crypto.h>
#include <trace.h>
#include <platform_config.h>
#include <sta_hsm_provider.h>
#if !defined(CFG_CRYPTO_HASH_FROM_CRYPTOLIB) || \
	!defined(CFG_CRYPTO_ECC_FROM_CRYPTOLIB)
#include "CSE_ext_hash.h"
#endif
#if !defined(CFG_CRYPTO_ECC_FROM_CRYPTOLIB)
#include "CSE_ext_ECC.h"
#include "CSE_ext_ECC_ECDH.h"
#endif
#if !defined(CFG_CRYPTO_RNG_FROM_CRYPTOLIB)
#include "CSE_RNG.h"
#endif

/******************************************************************************
 * Message digest functions
 ******************************************************************************/
#if defined(_CFG_CRYPTO_WITH_HASH) && !defined(CFG_CRYPTO_HASH_FROM_CRYPTOLIB)

struct tee_hash_ctx {
	const uint8_t *data;
	size_t len;
	uint32_t nr_entries;
};

/**
 * @brief - Allocate hsm hash context memory.
 * @param Context pointer output
 * @param algo unused
 * @return TEE_SUCCESS or TEE_ERROR_OUT_OF_MEMORY
 */
TEE_Result crypto_hash_alloc_ctx(void **ctx_ret, uint32_t algo __unused)
{
	void *ctx;

	ctx = calloc(1, sizeof(struct tee_hash_ctx));
	if (!ctx)
		return TEE_ERROR_OUT_OF_MEMORY;

	*ctx_ret = ctx;
	return TEE_SUCCESS;
}

void crypto_hash_free_ctx(void *ctx, uint32_t algo __unused)
{
	free(ctx);
}

void crypto_hash_copy_state(void *dst_ctx, void *src_ctx,
			    uint32_t algo __unused)
{
	memcpy(dst_ctx, src_ctx, sizeof(struct tee_hash_ctx));
}

TEE_Result crypto_hash_init(void *ctx __unused, uint32_t algo __unused)
{
	return TEE_SUCCESS;
}

/**
 * @brief - Save inputs parameters.
 * @param Context
 * @param Hash algorithm
 * @param Data input
 * @param Data length input
 * @return TEE_SUCCESS if no issue
 *         TEE_ERROR_OUT_OF_MEMORY if data is NULL
 */
TEE_Result crypto_hash_update(void *ctx, uint32_t algo __unused,
			      const uint8_t *data, size_t len)
{
	struct tee_hash_ctx *hash_cs = (struct tee_hash_ctx *)ctx;

	if (!data)
		return TEE_ERROR_BAD_PARAMETERS;

	hash_cs->data = data;
	hash_cs->len = len;
	hash_cs->nr_entries++;

	return TEE_SUCCESS;
}

/**
 * @brief - Using inputs parameters, Digest is computed with eHSM driver.
 * @param Context
 * @param Hash algorithm
 * @param Digest output
 * @param Digest length output
 * @return TEE_SUCCESS if no issue
 *         TEE_ERROR_NOT_SUPPORTED if algo is not supported
 */
TEE_Result crypto_hash_final(void *ctx, uint32_t algo, uint8_t *digest,
			     size_t len)
{
	TEE_Result ret = TEE_SUCCESS;
	struct tee_hash_ctx *hash_cs = (struct tee_hash_ctx *)ctx;
	paddr_t pa_data = ALIAS_TO_PHYS(virt_to_phys((void *)hash_cs->data));
	paddr_t pa_digest = ALIAS_TO_PHYS(virt_to_phys((void *)digest));
	size_t digest_size = 0;
	uint32_t res = 0;

	/* Flush input data buffer before notifying HSM */
	dcache_clean_range((void *)hash_cs->data, len);

	/* Invalidate output data buffer to get answer from HSM */
	dcache_inv_range((void *)digest, CRL_SHA512_SIZE);

	switch (algo) {
	case TEE_ALG_SHA1:
		digest_size = CRL_SHA1_SIZE;
		res = cse_sha1((uint8_t *)pa_data, (uint32_t)hash_cs->len,
			       (uint8_t *)pa_digest, &digest_size);
		break;
	case TEE_ALG_SHA224:
		digest_size = CRL_SHA224_SIZE;
		res = cse_sha224((uint8_t *)pa_data, (uint32_t)hash_cs->len,
				 (uint8_t *)pa_digest, &digest_size);
		break;
	case TEE_ALG_SHA256:
		digest_size = CRL_SHA256_SIZE;
		res = cse_sha256((uint8_t *)pa_data, (uint32_t)hash_cs->len,
				 (uint8_t *)pa_digest, &digest_size);
		break;
	case TEE_ALG_SHA384:
		digest_size = CRL_SHA384_SIZE;
		res = cse_sha384((uint8_t *)pa_data, (uint32_t)hash_cs->len,
				 (uint8_t *)pa_digest, &digest_size);
		break;
	case TEE_ALG_SHA512:
		digest_size = CRL_SHA512_SIZE;
		res = cse_sha512((uint8_t *)pa_data, (uint32_t)hash_cs->len,
				 (uint8_t *)pa_digest, &digest_size);
		break;
	default:
		ret = TEE_ERROR_NOT_SUPPORTED;
	}

	if (hash_cs->nr_entries > 1)
		EMSG("Muti entries not supported\n");
	hash_cs->nr_entries = 0;

	if (res) {
		EMSG("Crypto HASH error: %x\n", res);
		ret = TEE_ERROR_GENERIC;
	}
	return ret;
}
#endif /* _CFG_CRYPTO_WITH_HASH && !CFG_CRYPTO_HASH_FROM_CRYPTOLIB */

/******************************************************************************
 * Asymmetric algorithms
 ******************************************************************************/
#if defined(CFG_CRYPTO_ECC) && !defined(CFG_CRYPTO_ECC_FROM_CRYPTOLIB)

#define ECC_STATE_KEY_FREE		0
#define ECC_STATE_KEY_PUB_LOADED	1
#define ECC_STATE_KEY_PAIR_LOADED	2
#define ECC_KEY_NOT_VALID		((uint32_t)0xFFFFFFFF)

/* List of supported NIST Curves by eHSM */
#define C_NIST_P_256			(0x50323536)
#define C_NIST_P_384			(0x50333834)
#define C_NIST_P_521			(0x50353231)
#define C_BRAINPOOL_P256R1		(0x42323536)
#define C_BRAINPOOL_P384R1		(0x42333834)

#define ECC_KEY_DUMMY_SIZE		4

static uint8_t ecc_key_dummy[] = {
	0xDD, 0xDD, 0xDD, 0x00
};

struct tee_ecc_hsm_key {
	uint32_t state;
	struct bignum *pub_key_x;
	struct bignum *pub_key_y;
};

static struct tee_ecc_hsm_key ecc_hsm_key[NB_OF_CSE_ECC_KEYS];
static bool ecc_hsm_key_storage_init_done;

static uint32_t ecc_get_hsm_curve(uint32_t curve)
{
	switch (curve) {
	case TEE_ECC_CURVE_NIST_P256:
		return C_NIST_P_256;

	case TEE_ECC_CURVE_NIST_P384:
		return C_NIST_P_384;

	case TEE_ECC_CURVE_NIST_P521:
		return C_NIST_P_521;

	case TEE_ECC_CURVE_NIST_P192:
	case TEE_ECC_CURVE_NIST_P224:
	default:
		/* Not supported by eHSM */
		break;
	}
	return 0;
}

static uint32_t ecc_get_keysize_bytes(uint32_t curve)
{
	switch (curve) {
	case TEE_ECC_CURVE_NIST_P256:
		return 32;

	case TEE_ECC_CURVE_NIST_P384:
		return 48;

	case TEE_ECC_CURVE_NIST_P521:
		return 66;

	case TEE_ECC_CURVE_NIST_P192:
	case TEE_ECC_CURVE_NIST_P224:
	default:
		/* Not supported by eHSM */
		break;
	}
	return 0;
}

static TEE_Result ecc_get_free_hsm_key_id(uint32_t *key_id)
{
	int i;

	for (i = ECC_KEY_1_IDX; i < ECC_KEY_MAX_IDX; i++) {
		struct tee_ecc_hsm_key *key = &ecc_hsm_key[i];

		if (key->state == ECC_STATE_KEY_FREE) {
			*key_id = i;
			return TEE_SUCCESS;
		}
	}
	/* No more ECC NVM key slot available in eHSM */
	*key_id = ECC_KEY_NOT_VALID;

	return TEE_ERROR_STORAGE_NO_SPACE;
}

static TEE_Result ecc_set_hsm_key_id(uint32_t key_id,
				     struct bignum *pub_key_x,
				     struct bignum *pub_key_y,
				     struct bignum *priv_key_d)
{
	struct tee_ecc_hsm_key *hsm_key = &ecc_hsm_key[key_id];

	/*
	 * An ECC key pair has been generated and stored in the eHSM.
	 * Save an image of the eHSM key storage in TEE to be able to handle
	 * key index according to the requested public key for the next ECC
	 * operation.
	 * eHSM can store up to 4 ECC NVM keys and one ECC RAM key
	 */
	if (hsm_key->state == ECC_STATE_KEY_FREE) {
		size_t sz_bit = C_MAX_PUB_KEY_SIZE * 8;

		/* Allocate public key if index never loaded */
		hsm_key->pub_key_x = crypto_bignum_allocate(sz_bit);
		if (!hsm_key->pub_key_x)
			goto err;
		hsm_key->pub_key_y = crypto_bignum_allocate(sz_bit);
		if (!hsm_key->pub_key_y)
			goto err;
	}
	crypto_bignum_copy(hsm_key->pub_key_x, pub_key_x);
	crypto_bignum_copy(hsm_key->pub_key_y, pub_key_y);

	if (priv_key_d)
		hsm_key->state = ECC_STATE_KEY_PAIR_LOADED;
	else
		hsm_key->state = ECC_STATE_KEY_PUB_LOADED;

	return TEE_SUCCESS;

err:
	crypto_bignum_free(hsm_key->pub_key_x);
	crypto_bignum_free(hsm_key->pub_key_y);
	return TEE_ERROR_OUT_OF_MEMORY;
}

static TEE_Result ecc_export_hsm_pub_key(uint32_t key_id,
					 struct ecc_keypair *keypair,
					 struct ecc_pub_key_stt *keypub)
{
	TEE_Result ret = TEE_SUCCESS;
	uint32_t hsm_res = 0;

	struct ecc_pub_key_stt *pub_key = NULL;
	uint8_t *pub_key_x = NULL;
	uint8_t *pub_key_y = NULL;
	paddr_t pa_pub_key = 0;
	paddr_t pa_pub_key_x = 0;
	paddr_t pa_pub_key_y = 0;
	uint32_t pub_key_size_x = C_MAX_PUB_KEY_SIZE;
	uint32_t pub_key_size_y = C_MAX_PUB_KEY_SIZE;

	if (keypub) {
		pub_key = keypub;
		pub_key_x = (uint8_t *)keypub->pub_key_coordX;
		pub_key_size_x = keypub->pub_key_coordX_size;
		pub_key_y = (uint8_t *)keypub->pub_key_coordY;
		pub_key_size_y = keypub->pub_key_coordY_size;
	} else {
		pub_key = calloc(1, sizeof(struct ecc_pub_key_stt));
		pub_key_x = calloc(1, pub_key_size_x);
		pub_key_y = calloc(1, pub_key_size_y);
	}
	if (!pub_key || !pub_key_x || !pub_key_y) {
		ret = TEE_ERROR_OUT_OF_MEMORY;
		goto err;
	}
	pa_pub_key = ALIAS_TO_PHYS(virt_to_phys((void *)pub_key));
	pa_pub_key_x = ALIAS_TO_PHYS(virt_to_phys((void *)pub_key_x));
	pa_pub_key_y = ALIAS_TO_PHYS(virt_to_phys((void *)pub_key_y));

	/* Exports the public key part of the ECC key pair generated */
	pub_key->pub_key_coordX = (uint8_t *)pa_pub_key_x;
	pub_key->pub_key_coordX_size = pub_key_size_x;
	pub_key->pub_key_coordY = (uint8_t *)pa_pub_key_y;
	pub_key->pub_key_coordY_size = pub_key_size_y;

	/* Flush input/output data buffer before notifying HSM */
	dcache_clean_range((void *)pub_key, sizeof(struct ecc_pub_key_stt));
	dcache_clean_range((void *)pub_key_x, pub_key_size_x);
	dcache_clean_range((void *)pub_key_y, pub_key_size_y);

	hsm_res = cse_ecc_export_publickey(key_id,
					(struct ecc_pub_key_stt *)pa_pub_key);

	/* Invalidate output data buffer to get answer from HSM */
	dcache_inv_range((void *)pub_key, sizeof(struct ecc_pub_key_stt));
	dcache_inv_range((void *)pub_key_x, pub_key_size_x);
	dcache_inv_range((void *)pub_key_y, pub_key_size_y);

	if (hsm_res) {
		if (hsm_res == CSE_EMPTY_KEY)
			ret = TEE_ERROR_SIGNATURE_INVALID;
		else
			ret = TEE_ERROR_GENERIC;
		goto err;
	}

	/*
	 * If ECC key pair have only private key then X and Y key sizes are
	 * set to 0. So in this case the X and Y coordinates are filled to
	 * a dummy key + key index where the key is stored in eHSM.
	 */
	if (!pub_key->pub_key_coordX_size && !pub_key->pub_key_coordY_size) {
		ecc_key_dummy[3] = (uint8_t)key_id;

		memcpy(pub_key_x, ecc_key_dummy, ECC_KEY_DUMMY_SIZE);
		pub_key->pub_key_coordX_size = ECC_KEY_DUMMY_SIZE;
		memcpy(pub_key_y, ecc_key_dummy, ECC_KEY_DUMMY_SIZE);
		pub_key->pub_key_coordY_size = ECC_KEY_DUMMY_SIZE;
	}

	/* Copy the public key only */
	crypto_bignum_clear(keypair->d);
	crypto_bignum_bin2bn(pub_key_x,
			     pub_key->pub_key_coordX_size,
			     keypair->x);
	crypto_bignum_bin2bn(pub_key_y,
			     pub_key->pub_key_coordY_size,
			     keypair->y);

	if (keypub) {
		keypub->pub_key_coordX_size = pub_key->pub_key_coordX_size;
		keypub->pub_key_coordY_size = pub_key->pub_key_coordY_size;
	}

	/* Update local ECC key array */
	ret = ecc_set_hsm_key_id(key_id, keypair->x, keypair->y, keypair->d);

err:
	if (!keypub) {
		free(pub_key_x);
		free(pub_key_y);
		free(pub_key);
	}

	return ret;
}

int ecc_hsm_init(void)
{
	struct tee_ecc_hsm_key *key = ecc_hsm_key;

	memset(key, 0, sizeof(struct tee_ecc_hsm_key) * NB_OF_CSE_ECC_KEYS);
	ecc_hsm_key_storage_init_done = false;

	return 0;
}

TEE_Result ecc_get_hsm_key_id(void *kpair, void *kpub, uint32_t *key_id)
{
	TEE_Result ret = TEE_SUCCESS;
	struct ecc_keypair *keypair = (struct ecc_keypair *)kpair;
	struct ecc_public_key *keypub = (struct ecc_public_key *)kpub;
	struct bignum *pub_x = NULL;
	struct bignum *pub_y = NULL;
	struct bignum *priv_d = NULL;
	uint32_t key_size = 0;
	uint32_t state = ECC_STATE_KEY_FREE;
	int i;

	/* Get eHSM key index according to the requested public key */
	if (keypair) {
		pub_x = keypair->x;
		pub_y = keypair->y;
		priv_d = keypair->d;
		key_size = ecc_get_keysize_bytes(keypair->curve);
		state = ECC_STATE_KEY_PAIR_LOADED;
	} else if (keypub) {
		pub_x = keypub->x;
		pub_y = keypub->y;
		key_size = ecc_get_keysize_bytes(keypub->curve);
		state = ECC_STATE_KEY_PUB_LOADED;
	} else {
		/* If no key requested, return next ECC NVM key */
		if (ecc_hsm_key_storage_init_done)
			/*
			 * Successful Key storage initialization
			 * so get 1st free ECC NVM key index
			 */
			ret = ecc_get_free_hsm_key_id(key_id);
		else
			/*
			 * Key storage not supported or not initialized
			 * so only RAM key index can be handled
			 */
			*key_id = ECC_RAM_KEY_IDX;

		goto end;
	}

	/*
	 * One public key is requested then check if this key is already
	 * loaded in the eHSM.
	 */
	for (i = 0; i < ECC_KEY_MAX_IDX; i++) {
		struct tee_ecc_hsm_key *key = &ecc_hsm_key[i];

		if (state <= key->state) {
			if (!crypto_bignum_compare(pub_x, key->pub_key_x) &&
			    !crypto_bignum_compare(pub_y, key->pub_key_y)) {
				*key_id = i;
				goto end;
			}
		}
	}
	/* Public key not found so load it in the RAM_KEY index */
	ret = ecc_load_hsm_ram_key(pub_x, pub_y, priv_d, key_size);
	if (ret == TEE_SUCCESS)
		*key_id = ECC_RAM_KEY_IDX;
	else
		*key_id = ECC_KEY_NOT_VALID;

end:
	return ret;
}

TEE_Result ecc_handle_hsm_curve(uint32_t curve)
{
	uint32_t hsm_res = 0;
	uint32_t ecc_curve_id;

	/* Keep current HSM curve */
	if (!curve)
		return TEE_SUCCESS;

	/*
	 * Get current curse selected in eHSM not needed
	 * Change current curve order only applies change if needed
	 */
	ecc_curve_id = ecc_get_hsm_curve(curve);
	if (!ecc_curve_id)
		return TEE_ERROR_BAD_PARAMETERS;

	/* Change curve setting in eHSM */
	hsm_res = cse_ecc_change_curve(ecc_curve_id);
	if (hsm_res)
		return TEE_ERROR_GENERIC;

	return TEE_SUCCESS;
}

TEE_Result ecc_load_hsm_ram_key(void *pubk_x, void *pubk_y, void *privk_d,
				uint32_t key_size)
{
	TEE_Result ret = TEE_SUCCESS;
	uint32_t hsm_res = 0;
	int offset = 0;
	struct bignum *key_x = (struct bignum *)pubk_x;
	struct bignum *key_y = (struct bignum *)pubk_y;
	struct bignum *key_d = (struct bignum *)privk_d;
	uint32_t key_x_size = 0;
	uint32_t key_y_size = 0;
	uint32_t key_d_size = 0;

	struct ecc_pub_key_stt *pub_key = NULL;
	struct ecc_priv_key_stt *priv_key = NULL;
	paddr_t pa_pub_key = 0;
	paddr_t pa_priv_key = 0;

	uint8_t *pub_key_x = NULL;
	uint8_t *pub_key_y = NULL;
	uint8_t *priv_key_d = NULL;
	paddr_t pa_pub_key_x = 0;
	paddr_t pa_pub_key_y = 0;
	paddr_t pa_priv_key_d = 0;

	if (!key_x || !key_y || !key_size)
		return TEE_ERROR_BAD_PARAMETERS;

	pub_key = calloc(1, sizeof(struct ecc_pub_key_stt));
	priv_key = calloc(1, sizeof(struct ecc_priv_key_stt));
	if (!pub_key || !priv_key) {
		ret = TEE_ERROR_OUT_OF_MEMORY;
		goto err;
	}
	pa_pub_key = ALIAS_TO_PHYS(virt_to_phys((void *)pub_key));
	pa_priv_key = ALIAS_TO_PHYS(virt_to_phys((void *)priv_key));

	pub_key_x = calloc(1, key_size);
	pub_key_y = calloc(1, key_size);
	if (!pub_key_x || !pub_key_y) {
		ret = TEE_ERROR_OUT_OF_MEMORY;
		goto err;
	}
	pa_pub_key_x = ALIAS_TO_PHYS(virt_to_phys((void *)pub_key_x));
	pa_pub_key_y = ALIAS_TO_PHYS(virt_to_phys((void *)pub_key_y));

	/* Copy public key. Add start padding if needed */
	key_x_size = crypto_bignum_num_bytes(key_x);
	if (key_x_size) {
		offset = key_size - key_x_size;
		if (offset < 0) {
			ret = TEE_ERROR_EXCESS_DATA;
			goto err;
		}
		crypto_bignum_bn2bin(key_x, pub_key_x + offset);
		key_x_size = key_size;
	}

	key_y_size = crypto_bignum_num_bytes(key_y);
	if (key_y_size) {
		offset = key_size - key_y_size;
		if (offset < 0) {
			ret = TEE_ERROR_EXCESS_DATA;
			goto err;
		}
		crypto_bignum_bn2bin(key_y, pub_key_y + offset);
		key_y_size = key_size;
	}

	/* Fill public key input parameter for eHSM */
	pub_key->pub_key_coordX = (uint8_t *)pa_pub_key_x;
	pub_key->pub_key_coordX_size = key_x_size;
	pub_key->pub_key_coordY = (uint8_t *)pa_pub_key_y;
	pub_key->pub_key_coordY_size = key_y_size;

	if (key_d) {
		priv_key_d = calloc(1, key_size);
		if (!priv_key_d) {
			ret = TEE_ERROR_OUT_OF_MEMORY;
			goto err;
		}
		pa_priv_key_d = ALIAS_TO_PHYS(
					virt_to_phys((void *)priv_key_d));

		/* Copy private key. Add start padding if needed */
		key_d_size = crypto_bignum_num_bytes(key_d);
		if (key_d_size) {
			offset = key_size - key_d_size;
			if (offset < 0) {
				ret = TEE_ERROR_EXCESS_DATA;
				goto err;
			}
			crypto_bignum_bn2bin(key_d, priv_key_d + offset);
			key_d_size = key_size;
		}

		/* Fill private key input parameter for eHSM */
		priv_key->priv_key = (uint8_t *)pa_priv_key_d;
		priv_key->priv_key_size = key_d_size;
	}

	/* Flush input/output data buffer before notifying HSM */
	dcache_clean_range((void *)pub_key, sizeof(struct ecc_pub_key_stt));
	dcache_clean_range((void *)priv_key, sizeof(struct ecc_priv_key_stt));
	dcache_clean_range((void *)pub_key_x, key_size);
	dcache_clean_range((void *)pub_key_y, key_size);
	if (priv_key_d)
		dcache_clean_range((void *)priv_key_d, key_size);

	/* Load ECC RAM key in eHSM */
	hsm_res = cse_ecc_load_ramkey((struct ecc_pub_key_stt *)pa_pub_key,
				      (struct ecc_priv_key_stt *)pa_priv_key);
	if (hsm_res) {
		ret = TEE_ERROR_GENERIC;
		goto err;
	}

	/* Update local ECC RAM key index */
	ret = ecc_set_hsm_key_id(ECC_RAM_KEY_IDX, key_x, key_y, key_d);

err:
	free(pub_key_x);
	free(pub_key_y);
	free(priv_key_d);
	free(pub_key);
	free(priv_key);

	return ret;
}

TEE_Result ecc_key_mapping_init(void)
{
	TEE_Result ret = TEE_SUCCESS;
	uint32_t key_id;

	/* Successful key storage init in eHSM */
	ecc_hsm_key_storage_init_done = true;

	/* Get ECC NVM key from HSM and update internal key array accordingly */
	for (key_id = ECC_KEY_1_IDX; key_id <= ECC_KEY_MAX_IDX; key_id++) {
		ret = ecc_get_pub_key(key_id, NULL);
		if (ret != TEE_SUCCESS)
			goto err;
	}

err:
	return ret;
}

TEE_Result ecc_get_pub_key(uint32_t key_id, void *pub_key)
{
	TEE_Result ret = TEE_SUCCESS;
	struct ecc_pub_key_stt *keypub = (struct ecc_pub_key_stt *)pub_key;
	struct ecc_keypair *keypair = calloc(1, sizeof(struct ecc_keypair));

	if (!keypair) {
		ret = TEE_ERROR_OUT_OF_MEMORY;
		goto err;
	}

	/* Allocate ecc keypair area required by key export function */
	keypair->d = crypto_bignum_allocate(C_MAX_PUB_KEY_SIZE * 8);
	keypair->x = crypto_bignum_allocate(C_MAX_PUB_KEY_SIZE * 8);
	keypair->y = crypto_bignum_allocate(C_MAX_PUB_KEY_SIZE * 8);
	if (!keypair->d || !keypair->x || !keypair->y) {
		ret = TEE_ERROR_OUT_OF_MEMORY;
		goto err;
	}

	/* Export ECC NVM key and update internal key array accordingly */
	ret = ecc_export_hsm_pub_key(key_id, keypair, keypub);
	if (ret == TEE_ERROR_SIGNATURE_INVALID) {
		ecc_hsm_key[key_id].state = ECC_STATE_KEY_FREE;
		ret = TEE_SUCCESS;
	}

err:
	crypto_bignum_free(keypair->d);
	crypto_bignum_free(keypair->x);
	crypto_bignum_free(keypair->y);
	free(keypair);
	return ret;
}

TEE_Result crypto_acipher_gen_ecc_key(struct ecc_keypair *key)
{
	TEE_Result ret = TEE_SUCCESS;
	uint32_t hsm_res = 0;
	uint32_t key_id = 0U;
	union extended_key_flags ec_flags;

	/* Change ECC curve setting in eHSM if needed */
	ret = ecc_handle_hsm_curve(key->curve);
	if (ret != TEE_SUCCESS)
		goto err;

	/* Generate new ECC key according to the selected curve */
	/* 1st, get the next free NVM key index */
	ret = ecc_get_hsm_key_id(NULL, NULL, &key_id);
	if (ret != TEE_SUCCESS)
		goto err;

	ec_flags.R = 0;
	ec_flags.B.sign = 1;      /* Key allowed for signature generation */
	ec_flags.B.verify = 1;    /* Key allowed for signature verification */

	hsm_res = cse_ecc_generate_load_keypair(key_id, ec_flags.R);
	if (hsm_res) {
		ret = TEE_ERROR_GENERIC;
		goto err;
	}

	/* Exports the public key part of the ECC key pair generated */
	ret = ecc_export_hsm_pub_key(key_id, key, NULL);

err:
	return ret;
}

TEE_Result crypto_acipher_ecc_sign(uint32_t algo, struct ecc_keypair *key,
				   const uint8_t *msg, size_t msg_len,
				   uint8_t *sig, size_t *sig_len)
{
	TEE_Result ret = TEE_SUCCESS;
	uint32_t hsm_res = 0;
	uint32_t key_id = 0;
	paddr_t pa_msg = 0;

	struct ecdsa_signature_stt *sign = calloc(1,
				sizeof(struct ecdsa_signature_stt));
	uint8_t *sign_r = calloc(1, C_MAX_SIG_SIZE);
	uint8_t *sign_s = calloc(1, C_MAX_SIG_SIZE);
	paddr_t pa_sign = 0;
	paddr_t pa_sign_r = 0;
	paddr_t pa_sign_s = 0;

	if (!sign || !sign_r || !sign_s) {
		ret = TEE_ERROR_OUT_OF_MEMORY;
		goto err;
	}
	pa_sign = ALIAS_TO_PHYS(virt_to_phys((void *)sign));
	pa_sign_r = ALIAS_TO_PHYS(virt_to_phys((void *)sign_r));
	pa_sign_s = ALIAS_TO_PHYS(virt_to_phys((void *)sign_s));
	pa_msg = ALIAS_TO_PHYS(virt_to_phys((void *)msg));

	if (algo == 0) {
		ret = TEE_ERROR_BAD_PARAMETERS;
		goto err;
	}

	/* Change ECC curve setting in eHSM if needed */
	ret = ecc_handle_hsm_curve(key->curve);
	if (ret != TEE_SUCCESS)
		goto err;

	/* Get key id from internal hsm location */
	ret = ecc_get_hsm_key_id(key, NULL, &key_id);
	if (ret != TEE_SUCCESS)
		goto err;

	sign->sigR = (vuint8_t *)pa_sign_r;
	sign->sigS = (vuint8_t *)pa_sign_s;

	/*
	 * Generate ECDSA digest signature
	 */
	/* Flush input/output data buffer before notifying HSM */
	dcache_clean_range((void *)msg, msg_len);
	dcache_clean_range((void *)sign, sizeof(struct ecdsa_signature_stt));
	dcache_clean_range((void *)sign_r, C_MAX_SIG_SIZE);
	dcache_clean_range((void *)sign_s, C_MAX_SIG_SIZE);

	hsm_res = cse_ecc_ecdsa_digest_sign(key_id, (vuint8_t *)pa_msg, msg_len,
					(struct ecdsa_signature_stt *)pa_sign);

	/* Invalidate output data buffer to get answer from HSM */
	dcache_inv_range((void *)sign_r, C_MAX_SIG_SIZE);
	dcache_inv_range((void *)sign_s, C_MAX_SIG_SIZE);
	dcache_inv_range((void *)sign, sizeof(struct ecdsa_signature_stt));

	/* Concatenate sig.R and sig.S to *sig */
	if (!hsm_res) {
		*sig_len = 2 * ecc_get_keysize_bytes(key->curve);
		memset(sig, 0, *sig_len);
		memcpy((uint8_t *)sig + *sig_len / 2 - sign->sigR_size,
		       (void *)sign_r, sign->sigR_size);
		memcpy((uint8_t *)sig + *sig_len - sign->sigS_size,
		       (void *)sign_s, sign->sigS_size);

		ret = TEE_SUCCESS;
	} else {
		ret = TEE_ERROR_GENERIC;
	}

err:
	free(sign_r);
	free(sign_s);
	free(sign);

	return ret;
}

TEE_Result crypto_acipher_ecc_verify(uint32_t algo, struct ecc_public_key *key,
				     const uint8_t *msg, size_t msg_len,
				     const uint8_t *sig, size_t sig_len)
{
	TEE_Result ret = TEE_SUCCESS;
	uint32_t hsm_res = 0;
	uint32_t key_id = 0;
	paddr_t pa_msg;

	uint32_t sign_verif = 0;

	struct ecdsa_signature_stt *sign = calloc(1,
				sizeof(struct ecdsa_signature_stt));
	uint8_t *sign_r = calloc(1, C_MAX_SIG_SIZE);
	uint8_t *sign_s = calloc(1, C_MAX_SIG_SIZE);
	paddr_t pa_sign = 0;
	paddr_t pa_sign_r = 0;
	paddr_t pa_sign_s = 0;

	if (!sign || !sign_r || !sign_s) {
		ret = TEE_ERROR_OUT_OF_MEMORY;
		goto err;
	}
	pa_sign = ALIAS_TO_PHYS(virt_to_phys((void *)sign));
	pa_sign_r = ALIAS_TO_PHYS(virt_to_phys((void *)sign_r));
	pa_sign_s = ALIAS_TO_PHYS(virt_to_phys((void *)sign_s));
	pa_msg = ALIAS_TO_PHYS(virt_to_phys((void *)msg));

	if (algo == 0) {
		ret = TEE_ERROR_BAD_PARAMETERS;
		goto err;
	}

	/* Change ECC curve setting in eHSM if needed */
	ret = ecc_handle_hsm_curve(key->curve);
	if (ret != TEE_SUCCESS)
		goto err;

	/* Extract sign.R and sign.S from *sig */
	memcpy((void *)sign_r, (uint8_t *)sig, sig_len / 2);
	memcpy((void *)sign_s, (uint8_t *)sig + sig_len / 2, sig_len / 2);

	/* Get key id from internal hsm location */
	ret = ecc_get_hsm_key_id(NULL, key, &key_id);
	if (ret != TEE_SUCCESS)
		goto err;

	sign->sigR = (vuint8_t *)pa_sign_r;
	sign->sigR_size = sig_len / 2;
	sign->sigS = (vuint8_t *)pa_sign_s;
	sign->sigS_size = sig_len / 2;

	/*
	 * Verify ECDSA digest signature
	 */
	/* Flush input/output data buffer before notifying HSM */
	dcache_clean_range((void *)msg, msg_len);
	dcache_clean_range((void *)sign, sizeof(struct ecdsa_signature_stt));
	dcache_clean_range((void *)sign_r, sig_len / 2);
	dcache_clean_range((void *)sign_s, sig_len / 2);

	hsm_res = cse_ecc_ecdsa_digest_verify(key_id, (vuint8_t *)pa_msg,
					msg_len,
					(struct ecdsa_signature_stt *)pa_sign,
					(uint32_t *)&sign_verif);
	/*
	 * "sign_verif" is directly extracted from eHSM return parameter
	 * so no need to invalidate dcache to get answer
	 */
	if (!hsm_res && !sign_verif)
		ret = TEE_SUCCESS;
	else
		ret = TEE_ERROR_GENERIC;

err:
	free(sign_r);
	free(sign_s);
	free(sign);

	return ret;
}

TEE_Result crypto_acipher_ecc_shared_secret(struct ecc_keypair *private_key,
					    struct ecc_public_key *public_key,
					    void *secret,
					    unsigned long *secret_len)
{
	TEE_Result ret = TEE_SUCCESS;
	uint32_t hsm_res = 0;
	uint32_t key_id = 0;
	int offset = 0;

	struct ecdh_premaster_secret *pm_secret = calloc(1,
				sizeof(struct ecdh_premaster_secret));
	paddr_t pa_pm_secret = 0;
	paddr_t pa_secret = 0;

	struct ecc_pub_key_stt *pub_key = NULL;
	uint8_t *pub_key_x = NULL;
	uint8_t *pub_key_y = NULL;
	int32_t key_size = 0;
	paddr_t pa_pub_key = 0;
	paddr_t pa_pub_key_x = 0;
	paddr_t pa_pub_key_y = 0;

	if (!pm_secret) {
		ret = TEE_ERROR_OUT_OF_MEMORY;
		goto err;
	}

	/* Check the curves are the same */
	if (private_key->curve != public_key->curve)
		return TEE_ERROR_BAD_PARAMETERS;

	/* Change ECC curve setting in eHSM if needed */
	ret = ecc_handle_hsm_curve(public_key->curve);
	if (ret != TEE_SUCCESS)
		goto err;

	/* Prepare Pre-Master secret descriptor buffer */
	pa_pm_secret = ALIAS_TO_PHYS(virt_to_phys((void *)pm_secret));
	pa_secret = ALIAS_TO_PHYS(virt_to_phys((void *)secret));
	pm_secret->address = (uint8_t *)pa_secret;
	pm_secret->byteSize = *secret_len;
	*(uint32_t *)secret = *secret_len;

	/* 3 different eHSM cases according to the input private key */
	/*
	 * Case 1: cse_ecc_ecdh_key_agreement
	 *     No private key provided (d, x & y set to NULL)
	 *     Public key to be loaded in RAM or NVM slot
	 *     -> Public key IDX to be provided to eHSM
	 *
	 * Case 2: cse_ecc_ecdhfixed_key_agreement
	 *     Private key must be found in NVM slot
	 *     -> Private key IDX to be provided to eHSM
	 *     Public key to be loaded in RAM slot
	 *
	 * Case 3: cse_ecc_ecdhfixed_server_key_agreement
	 *     Private key must NOT be found in NVM slot
	 *     -> Private key to be loaded in RAM slot
	 *     Public key provided to eHSM as (x, y) coordinates
	 */
	if (!private_key ||
	    (!private_key->d && !private_key->x && !private_key->y)) {
		/*
		 * Case 1: cse_ecc_ecdh_key_agreement
		 */
		ret = ecc_get_hsm_key_id(NULL, public_key, &key_id);
		if (ret != TEE_SUCCESS)
			goto err;

		/* Flush input/output data buffer before notifying HSM */
		dcache_clean_range((void *)pm_secret,
				   sizeof(struct ecdh_premaster_secret));
		dcache_clean_range((void *)secret, pm_secret->byteSize);

		hsm_res = cse_ecc_ecdh_key_agreement(key_id,
				(struct ecdh_premaster_secret *)pa_pm_secret);
		goto end;
	}

	ret = ecc_get_hsm_key_id(private_key, NULL, &key_id);
	if (ret != TEE_SUCCESS)
		goto err;
	key_size = ecc_get_keysize_bytes(private_key->curve);
	if (!key_size) {
		ret = TEE_ERROR_BAD_PARAMETERS;
		goto err;
	}

	if (key_id >= ECC_KEY_1_IDX && key_id <= ECC_KEY_MAX_IDX) {
		/*
		 * Case 2: cse_ecc_ecdhfixed_key_agreement
		 */
		/* Load public key in ECC RAM slot */
		ret = ecc_load_hsm_ram_key(public_key->x, public_key->y,
					   NULL, key_size);
		if (ret != TEE_SUCCESS)
			goto err;

		/* Flush input/output data buffer before notifying HSM */
		dcache_clean_range((void *)pm_secret,
				   sizeof(struct ecdh_premaster_secret));
		dcache_clean_range((void *)secret, pm_secret->byteSize);

		hsm_res = cse_ecc_ecdhfixed_key_agreement(key_id,
				(struct ecdh_premaster_secret *)pa_pm_secret);
		goto end;

	} else if (key_id == ECC_RAM_KEY_IDX) {
		/*
		 * Case 3: cse_ecc_ecdhfixed_server_key_agreement
		 */
		pub_key_x = calloc(1, key_size);
		pub_key_y = calloc(1, key_size);
		pub_key = calloc(1, sizeof(struct ecc_pub_key_stt));
		if (!pub_key || !pub_key_x || !pub_key_y) {
			ret = TEE_ERROR_OUT_OF_MEMORY;
			goto err;
		}
		pa_pub_key = ALIAS_TO_PHYS(virt_to_phys((void *)pub_key));
		pa_pub_key_x = ALIAS_TO_PHYS(virt_to_phys((void *)pub_key_x));
		pa_pub_key_y = ALIAS_TO_PHYS(virt_to_phys((void *)pub_key_y));

		/* Copy public key. Add start padding if needed */
		offset = key_size - crypto_bignum_num_bytes(public_key->x);
		if (offset < 0) {
			ret = TEE_ERROR_EXCESS_DATA;
			goto err;
		}
		crypto_bignum_bn2bin(public_key->x, pub_key_x + offset);

		offset = key_size - crypto_bignum_num_bytes(public_key->y);
		if (offset < 0) {
			ret = TEE_ERROR_EXCESS_DATA;
			goto err;
		}
		crypto_bignum_bn2bin(public_key->y, pub_key_y + offset);

		/* Exports the public key part of the ECC key pair generated */
		pub_key->pub_key_coordX = (uint8_t *)pa_pub_key_x;
		pub_key->pub_key_coordX_size = key_size;
		pub_key->pub_key_coordY = (uint8_t *)pa_pub_key_y;
		pub_key->pub_key_coordY_size = key_size;

		/* Flush input/output data buffer before notifying HSM */
		dcache_clean_range((void *)pub_key,
				   sizeof(struct ecc_pub_key_stt));
		dcache_clean_range((void *)pub_key_x, key_size);
		dcache_clean_range((void *)pub_key_y, key_size);
		dcache_clean_range((void *)pm_secret,
				   sizeof(struct ecdh_premaster_secret));
		dcache_clean_range((void *)secret, pm_secret->byteSize);

		hsm_res = cse_ecc_ecdhfixed_server_key_agreement(
				(struct ecc_pub_key_stt *)pa_pub_key,
				(struct ecdh_premaster_secret *)pa_pm_secret);
		goto end;
	}

end:
	/* Invalidate output data buffer to get answer from HSM */
	dcache_inv_range((void *)pm_secret,
			 sizeof(struct ecdh_premaster_secret));
	dcache_inv_range((void *)secret, pm_secret->byteSize);

	if (hsm_res) {
		ret = TEE_ERROR_GENERIC;
		goto err;
	}
	/* get the size of the shared secret */
	*secret_len = pm_secret->byteSize;

err:
	free(pm_secret);
	free(pub_key);
	free(pub_key_x);
	free(pub_key_y);

	return ret;
}
#endif /* CFG_CRYPTO_ECC  && !CFG_CRYPTO_ECC_FROM_CRYPTOLIB*/

/******************************************************************************
 * Pseudo Random Number Generator
 ******************************************************************************/
#if !defined(CFG_CRYPTO_RNG_FROM_CRYPTOLIB)

#define RAND_BYTES_LEN	16
TEE_Result crypto_rng_read(void *buf, size_t blen)
{
	TEE_Result ret = TEE_SUCCESS;
	uint32_t hsm_res = 0;
	uint8_t *rand = calloc(1, RAND_BYTES_LEN);
	paddr_t pa_rand = 0;
	size_t randlen = 0;

	if (!rand) {
		ret = TEE_ERROR_OUT_OF_MEMORY;
		goto err;
	}
	pa_rand = ALIAS_TO_PHYS(virt_to_phys((void *)rand));

	while (randlen < blen) {
		size_t sz = blen - randlen;

		/* Flush input/output data buffer before notifying HSM */
		dcache_clean_range((void *)rand, RAND_BYTES_LEN);

		/* Generates a 128 bits random value */
		hsm_res = cse_prng_get_rand((uint8_t *)pa_rand);

		/* Invalidate output data buffer to get answer from HSM */
		dcache_inv_range((void *)rand, RAND_BYTES_LEN);

		if (hsm_res) {
			ret = TEE_ERROR_GENERIC;
			goto err;
		}
		/* Copy expected random bytes */
		if (sz > RAND_BYTES_LEN) {
			memcpy((uint8_t *)buf + randlen, rand, RAND_BYTES_LEN);
			randlen += RAND_BYTES_LEN;
		} else {
			memcpy((uint8_t *)buf + randlen, rand, sz);
			randlen = blen;
		}
	}

err:
	free(rand);

	return ret;
}

#endif /* !CFG_CRYPTO_RNG_FROM_CRYPTOLIB*/

