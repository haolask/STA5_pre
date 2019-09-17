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

#include <compiler.h>
#include <kernel/pseudo_ta.h>
#include <kernel/panic.h>
#include <mm/core_memprot.h>
#include <mm/tee_mm.h>
#include <string.h>
#include <tee/cache.h>
#include <kernel/cache_helpers.h>
#include <tee_api_defines.h>
#include <tee_api_types.h>
#include <crypto/crypto.h>
#include <trace.h>
#include <types_ext.h>
#include <platform_config.h>
#include <sta_hsm_provider.h>
#include <pta_sta_hsm.h>
#include "CSE_Manager.h"
#include "CSE_ext_manager.h"
#include "CSE_ext_ECC.h"
#include "CSE_ext_TLSv12_PRF.h"

#define TA_NAME		"sta_hsm.pta"

/*
 * Allocate memory in secure ddr area
 * [in]  buffer size to allocate
 * [out] virtual address of the allocated buffer
 * [out] physical address of the allocated buffer
 */
static tee_mm_entry_t *mm_sec_alloc(size_t size, vaddr_t *va, paddr_t *pa)
{
	tee_mm_entry_t *mm = NULL;

	mm = tee_mm_alloc(&tee_mm_sec_ddr, size);
	if (!mm)
		goto err;
	*pa = tee_mm_get_smem(mm);
	*va = (vaddr_t)phys_to_virt(*pa, MEM_AREA_TA_RAM);
	if (!va)
		goto err;

	return mm;

err:
	tee_mm_free(mm);
	return NULL;
}

static void mm_sec_free(tee_mm_entry_t *p)
{
	tee_mm_free(p);
}

static bool check_memref_param(uint32_t type, TEE_Param p[TEE_NUM_PARAMS])
{
	int i;

	for (i = 0; i < TEE_NUM_PARAMS; i++) {
		void *src = p[i].memref.buffer;
		size_t sz = p[i].memref.size;

		switch (TEE_PARAM_TYPE_GET(type, i)) {
		case TEE_PARAM_TYPE_MEMREF_INPUT:
		case TEE_PARAM_TYPE_MEMREF_OUTPUT:
		case TEE_PARAM_TYPE_MEMREF_INOUT:
			if (core_vbuf_is(CORE_MEM_NSEC_SHM, src, sz) ||
			    core_vbuf_is(CORE_MEM_TEE_RAM, src, sz) ||
			    core_vbuf_is(CORE_MEM_TA_RAM, src, sz))
				continue;
			else
				return false;
		default:
			break;
		}
	}
	return true;
}

static bool check_output_buf(uint8_t *in, size_t sz, tee_mm_entry_t **mm,
			     uint8_t **out, paddr_t *pa_out)
{
	/* Check if output buffer is located in not secure memory area */
	if (core_vbuf_is(CORE_MEM_NSEC_SHM, in, sz)) {
		/* output buffer need to be reallocated is secure DDR */
		*mm = mm_sec_alloc(sz, (vaddr_t *)out, (paddr_t *)pa_out);
		if (!(*mm))
			return false;

		memcpy((void *)(*out), (void *)in, sz);
	} else {
		*out = in;
		*pa_out = ALIAS_TO_PHYS(virt_to_phys((void *)(*out)));
	}
	return true;
}

/*
 * Get eHSM Firmware version ID
 */
static TEE_Result hsm_get_fw_id(uint32_t type, TEE_Param p[TEE_NUM_PARAMS])
{
	TEE_Result ret = TEE_SUCCESS;
	uint32_t hsm_res = 0;
	uint32_t fw_id = 0U;
	uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_OUTPUT,
					  TEE_PARAM_TYPE_NONE,
					  TEE_PARAM_TYPE_NONE,
					  TEE_PARAM_TYPE_NONE);

	if (exp_pt != type) {
		DMSG("bad parameter types");
		ret = TEE_ERROR_BAD_PARAMETERS;
		goto end;
	}

	/* Get FW ID from eHSM */
	hsm_res = cse_get_fw_id(&fw_id);
	if (hsm_res) {
		ret = TEE_ERROR_GENERIC;
		goto end;
	}
	p[0].value.a = fw_id;

	DMSG("get eHSM FW version ID: Ox%x", fw_id);

end:
	return ret;
}

/*
 * Derive a secret according to the TLS protocol used
 */
static TEE_Result hsm_tls_prf(uint32_t type, TEE_Param p[TEE_NUM_PARAMS])
{
	TEE_Result ret = TEE_SUCCESS;
	uint32_t hsm_res = 0;
	tee_mm_entry_t *mm = NULL;
	uint8_t *buf_out = NULL;
	paddr_t pa_buf_out = 0;

	struct tls_prf_secret *secret = calloc(1, sizeof(*secret));
	struct tls_prf_label *label = calloc(1, sizeof(*label));
	struct tls_prf_seed *seed = calloc(1, sizeof(*seed));
	struct tls_prf_output *tls_out = calloc(1, sizeof(*tls_out));
	paddr_t pa_secret;
	paddr_t pa_label;
	paddr_t pa_seed;
	paddr_t pa_tls_out;
	uint32_t hash_algo;

	uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
					  TEE_PARAM_TYPE_MEMREF_INPUT,
					  TEE_PARAM_TYPE_MEMREF_INPUT,
					  TEE_PARAM_TYPE_MEMREF_INOUT);

	if (exp_pt != type) {
		DMSG("bad parameter types");
		ret = TEE_ERROR_BAD_PARAMETERS;
		goto end;
	}
	if (!check_memref_param(type, p)) {
		DMSG("bad memref memory area");
		ret = TEE_ERROR_BAD_PARAMETERS;
		goto end;
	}
	if (!secret || !label || !seed || !tls_out) {
		ret = TEE_ERROR_OUT_OF_MEMORY;
		goto end;
	}
	/*
	 * HSM is configured to only write output data in secure memory area
	 * so reallocate output buffer accordingly if needed
	 */
	if (!check_output_buf(p[3].memref.buffer, p[3].memref.size, &mm,
			      &buf_out, &pa_buf_out)) {
		ret = TEE_ERROR_OUT_OF_MEMORY;
		goto end;
	}

	/* Convert to Physical address before passing to eHSM */
	pa_secret = ALIAS_TO_PHYS(virt_to_phys((void *)secret));
	pa_label = ALIAS_TO_PHYS(virt_to_phys((void *)label));
	pa_seed = ALIAS_TO_PHYS(virt_to_phys((void *)seed));
	pa_tls_out = ALIAS_TO_PHYS(virt_to_phys((void *)tls_out));

	secret->paddress = (uint8_t *)ALIAS_TO_PHYS(virt_to_phys(
						(void *)p[0].memref.buffer));
	secret->size = p[0].memref.size;
	label->paddress = (uint8_t *)ALIAS_TO_PHYS(virt_to_phys(
						(void *)p[1].memref.buffer));
	label->size = p[1].memref.size;
	seed->paddress = (uint8_t *)ALIAS_TO_PHYS(virt_to_phys(
						(void *)p[2].memref.buffer));
	seed->size = p[2].memref.size;
	tls_out->paddress = (uint8_t *)pa_buf_out;
	tls_out->size = p[3].memref.size;

	/* hash_algo set in the 2nd word of the tls output buffer */
	hash_algo = *((uint32_t *)p[3].memref.buffer + 1);

	/* Flush input/output data buffer before notifying HSM */
	dcache_clean_range((void *)secret, sizeof(*secret));
	dcache_clean_range((void *)label, sizeof(*label));
	dcache_clean_range((void *)seed, sizeof(*seed));
	dcache_clean_range((void *)tls_out, sizeof(*tls_out));
	dcache_clean_range((void *)p[0].memref.buffer, p[0].memref.size);
	dcache_clean_range((void *)p[1].memref.buffer, p[1].memref.size);
	dcache_clean_range((void *)p[2].memref.buffer, p[2].memref.size);
	dcache_clean_range((void *)buf_out, p[3].memref.size);

	/* Derive TLS secret */
	hsm_res = cse_tlsv12_prf((struct tls_prf_secret *)pa_secret,
				 (struct tls_prf_label *)pa_label,
				 (struct tls_prf_seed *)pa_seed,
				 hash_algo,
				 (struct tls_prf_output *)pa_tls_out);

	/* Invalidate output data buffer to get answer from HSM */
	dcache_inv_range((void *)buf_out, p[3].memref.size);
	dcache_inv_range((void *)tls_out, sizeof(*tls_out));

	if (hsm_res) {
		ret = TEE_ERROR_GENERIC;
		goto end;
	}

	/* Copy the TLS output descriptor */
	if (buf_out != p[3].memref.buffer)
		memcpy(p[3].memref.buffer, buf_out, tls_out->size);

	DMSG("TLS secret derived");

end:
	free(secret);
	free(label);
	free(seed);
	free(tls_out);
	mm_sec_free(mm);

	return ret;
}

#if defined(CFG_CRYPTO_ECC) && !defined(CFG_CRYPTO_ECC_FROM_CRYPTOLIB)
/*
 * Regenerate a new ECC key pair with security control
 */
static TEE_Result hsm_ecc_key_regen(uint32_t type, TEE_Param p[TEE_NUM_PARAMS])
{
	TEE_Result ret = TEE_SUCCESS;
	uint32_t hsm_res = 0;
	uint8_t *pa_m1 = (uint8_t *)ALIAS_TO_PHYS(virt_to_phys(
						(void *)p[0].memref.buffer));
	uint8_t *pa_m2 = (uint8_t *)ALIAS_TO_PHYS(virt_to_phys(
						(void *)p[1].memref.buffer));
	uint8_t *pa_m3 = (uint8_t *)ALIAS_TO_PHYS(virt_to_phys(
						(void *)p[2].memref.buffer));
	uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
					  TEE_PARAM_TYPE_MEMREF_INPUT,
					  TEE_PARAM_TYPE_MEMREF_INPUT,
					  TEE_PARAM_TYPE_VALUE_INPUT);

	if (exp_pt != type) {
		DMSG("bad parameter types");
		ret = TEE_ERROR_BAD_PARAMETERS;
		goto end;
	}
	if (!check_memref_param(type, p)) {
		DMSG("bad memref memory area");
		ret = TEE_ERROR_BAD_PARAMETERS;
		goto end;
	}

	/* Change ECC curve setting in eHSM if needed */
	ret = ecc_handle_hsm_curve(p[3].value.a);
	if (ret != TEE_SUCCESS)
		goto end;

	/* Flush input/output data buffer before notifying HSM */
	dcache_clean_range((void *)p[0].memref.buffer, p[0].memref.size);
	dcache_clean_range((void *)p[1].memref.buffer, p[1].memref.size);
	dcache_clean_range((void *)p[2].memref.buffer, p[2].memref.size);

	/* Regenerate a new ECC key pair */
	hsm_res = cse_ecc_regenerate_nvm_keypair(pa_m1, pa_m2, pa_m3);
	if (hsm_res) {
		ret = TEE_ERROR_GENERIC;
		goto end;
	}

	DMSG("New ECC key pair regenerated");

end:
	return ret;
}

/*
 * Key storage initialization
 */
static TEE_Result hsm_key_storage_init(uint32_t type,
				       TEE_Param p[TEE_NUM_PARAMS])
{
	TEE_Result ret = TEE_SUCCESS;
	uint32_t hsm_res = 0;
	uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
					  TEE_PARAM_TYPE_NONE,
					  TEE_PARAM_TYPE_NONE,
					  TEE_PARAM_TYPE_NONE);

	if (exp_pt != type) {
		DMSG("bad parameter types");
		ret = TEE_ERROR_BAD_PARAMETERS;
		goto end;
	}

	/* Trigger the external key image loading by the eHSM */
	hsm_res = cse_ext_set_ext_remote_key_import(p[0].value.a);
	if (hsm_res) {
		/* Key storage not supported or key image loading error */
		ret = TEE_ERROR_GENERIC;
		goto end;
	}

	/*
	 * Successful key image loading in eHSM so update the TEE internal
	 * array of the eHSM ECC key storage accordingly.
	 */
	ret = ecc_key_mapping_init();

	DMSG("Key storage initialized");

end:
	return ret;
}

/*
 * Export of ECC Public key
 */
static TEE_Result hsm_key_pub_export(uint32_t type, TEE_Param p[TEE_NUM_PARAMS])
{
	TEE_Result ret = TEE_SUCCESS;
	tee_mm_entry_t *mm_x = NULL;
	tee_mm_entry_t *mm_y = NULL;
	uint8_t *key_x = NULL;
	uint8_t *key_y = NULL;
	paddr_t pa_key_x = 0;
	paddr_t pa_key_y = 0;

	struct ecc_pub_key_stt *keypub = calloc(1,
				sizeof(struct ecc_pub_key_stt));

	uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
					  TEE_PARAM_TYPE_MEMREF_OUTPUT,
					  TEE_PARAM_TYPE_MEMREF_OUTPUT,
					  TEE_PARAM_TYPE_NONE);

	if (exp_pt != type) {
		DMSG("bad parameter types");
		ret = TEE_ERROR_BAD_PARAMETERS;
		goto end;
	}
	if (!check_memref_param(type, p)) {
		DMSG("bad memref memory area");
		ret = TEE_ERROR_BAD_PARAMETERS;
		goto end;
	}
	if (!keypub) {
		ret = TEE_ERROR_OUT_OF_MEMORY;
		goto end;
	}
	/*
	 * HSM is configured to only write output data in secure memory area
	 * so reallocate output buffer accordingly if needed
	 */
	if (!check_output_buf(p[1].memref.buffer, p[1].memref.size, &mm_x,
			      &key_x, &pa_key_x) ||
	    !check_output_buf(p[2].memref.buffer, p[2].memref.size, &mm_y,
			      &key_y, &pa_key_y)) {
		ret = TEE_ERROR_OUT_OF_MEMORY;
		goto end;
	}

	/* Get ECC NVM key from HSM and update internal key array accordingly */
	keypub->pub_key_coordX = key_x;
	keypub->pub_key_coordX_size = p[1].memref.size;
	keypub->pub_key_coordY = key_y;
	keypub->pub_key_coordY_size = p[2].memref.size;

	ret = ecc_get_pub_key(p[0].value.a, (void *)keypub);
	if (ret != TEE_SUCCESS)
		goto end;

	/* Copy the public key clear text */
	if (key_x != p[1].memref.buffer) {
		memcpy(p[1].memref.buffer, key_x, keypub->pub_key_coordX_size);
		p[1].memref.size = keypub->pub_key_coordX_size;
	}
	if (key_y != p[2].memref.buffer) {
		memcpy(p[2].memref.buffer, key_y, keypub->pub_key_coordY_size);
		p[2].memref.size = keypub->pub_key_coordY_size;
	}

	DMSG("ECC Public key exported");

end:
	free(keypub);
	mm_sec_free(mm_x);
	mm_sec_free(mm_y);

	return ret;
}

/*
 * Load ECC RAM key pair
 */
static TEE_Result hsm_load_ecc_ram_key(uint32_t type,
				       TEE_Param p[TEE_NUM_PARAMS])
{
	TEE_Result ret = TEE_SUCCESS;
	struct bignum *key_x = crypto_bignum_allocate(C_MAX_PUB_KEY_SIZE * 8);
	struct bignum *key_y = crypto_bignum_allocate(C_MAX_PUB_KEY_SIZE * 8);
	struct bignum *key_d = crypto_bignum_allocate(C_MAX_PUB_KEY_SIZE * 8);

	uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
					  TEE_PARAM_TYPE_MEMREF_INPUT,
					  TEE_PARAM_TYPE_MEMREF_INPUT,
					  TEE_PARAM_TYPE_VALUE_INPUT);

	if (exp_pt != type) {
		DMSG("bad parameter types");
		ret = TEE_ERROR_BAD_PARAMETERS;
		goto end;
	}
	if (!check_memref_param(type, p)) {
		DMSG("bad memref memory area");
		ret = TEE_ERROR_BAD_PARAMETERS;
		goto end;
	}
	if (!key_x || !key_y || !key_d) {
		ret = TEE_ERROR_OUT_OF_MEMORY;
		goto end;
	}

	/* Change ECC curve setting in eHSM if needed */
	ret = ecc_handle_hsm_curve(p[3].value.a);
	if (ret != TEE_SUCCESS)
		goto end;

	/* Convert and copy key pair to bignum */
	crypto_bignum_bin2bn(p[0].memref.buffer,
			     p[0].memref.size,
			     key_x);
	crypto_bignum_bin2bn(p[1].memref.buffer,
			     p[1].memref.size,
			     key_y);
	crypto_bignum_bin2bn(p[2].memref.buffer,
			     p[2].memref.size,
			     key_d);

	/* Load the ECC key pair */
	ret = ecc_load_hsm_ram_key(key_x, key_y, key_d, p[0].memref.size);
	if (ret != TEE_SUCCESS)
		goto end;

	DMSG("ECC RAM key pair loaded");

end:
	crypto_bignum_free(key_x);
	crypto_bignum_free(key_y);
	crypto_bignum_free(key_d);

	return ret;
}

/*
 * ECC Scalar Multiplication and Addition
 */
static TEE_Result hsm_ecc_scalar_mul_add(uint32_t type,
					 TEE_Param p[TEE_NUM_PARAMS])
{
	TEE_Result ret = TEE_SUCCESS;
	uint32_t hsm_res = 0;
	struct bignum *key_x = crypto_bignum_allocate(C_MAX_PUB_KEY_SIZE * 8);
	struct bignum *key_y = crypto_bignum_allocate(C_MAX_PUB_KEY_SIZE * 8);
	struct bignum *key_d = crypto_bignum_allocate(C_MAX_PUB_KEY_SIZE * 8);
	struct ecc_keypair *keypair = calloc(1, sizeof(*keypair));
	struct ecc_scalar_stt *mult = calloc(1, sizeof(*mult));
	struct ecc_scalar_stt *addend = calloc(1, sizeof(*addend));
	paddr_t pa_mult = 0;
	paddr_t pa_addend = 0;
	uint32_t key_id = 0;
	uint32_t key_size = (p[0].memref.size - sizeof(uint32_t)) / 3;
	uint32_t curve = 0;
	uint8_t *keys = NULL;

	uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
					  TEE_PARAM_TYPE_MEMREF_INPUT,
					  TEE_PARAM_TYPE_MEMREF_INPUT,
					  TEE_PARAM_TYPE_VALUE_OUTPUT);

	if (exp_pt != type) {
		DMSG("bad parameter types");
		ret = TEE_ERROR_BAD_PARAMETERS;
		goto end;
	}
	if (!check_memref_param(type, p)) {
		DMSG("bad memref memory area");
		ret = TEE_ERROR_BAD_PARAMETERS;
		goto end;
	}
	if (!keypair || !key_x || !key_y || !key_d || !mult || !addend) {
		ret = TEE_ERROR_OUT_OF_MEMORY;
		goto end;
	}

	/* Change ECC curve setting in eHSM if needed */
	curve = *((uint32_t *)p[0].memref.buffer);
	ret = ecc_handle_hsm_curve(curve);
	if (ret != TEE_SUCCESS)
		goto end;

	/* Convert and copy key pair to bignum */
	keys = p[0].memref.buffer + sizeof(uint32_t);
	crypto_bignum_bin2bn(keys, key_size, key_x);
	crypto_bignum_bin2bn(keys + key_size, key_size, key_y);
	crypto_bignum_bin2bn(keys + (2 * key_size), key_size, key_d);
	keypair->x = key_x;
	keypair->y = key_y;
	keypair->d = key_d;
	keypair->curve = curve;

	/* Get key ID related to the request key pair */
	ret = ecc_get_hsm_key_id(keypair, NULL, &key_id);
	if (ret != TEE_SUCCESS)
		goto end;

	/* Convert to Physical address before passing to eHSM */
	pa_mult = ALIAS_TO_PHYS(virt_to_phys((void *)mult));
	pa_addend = ALIAS_TO_PHYS(virt_to_phys((void *)addend));

	mult->address = (uint8_t *)ALIAS_TO_PHYS(virt_to_phys(
						 (void *)p[1].memref.buffer));
	mult->byteSize = p[1].memref.size;
	addend->address = (uint8_t *)ALIAS_TO_PHYS(virt_to_phys(
						   (void *)p[2].memref.buffer));
	addend->byteSize = p[2].memref.size;

	/* Calculated key always stored in RAM key slot */
	/* TODO: Get free slot for cut2_BH */
	p[3].value.a = ECC_RAM_KEY_IDX;

	/* Flush input/output data buffer before notifying HSM */
	dcache_clean_range((void *)mult, sizeof(*mult));
	dcache_clean_range((void *)addend, sizeof(*addend));
	dcache_clean_range((void *)p[1].memref.buffer, mult->byteSize);
	dcache_clean_range((void *)p[2].memref.buffer, addend->byteSize);

	/* Generate temporary private key */
	hsm_res = cse_ecc_scalar_mult_add(key_id,
					 (struct ecc_scalar_stt *)pa_mult,
					 (struct ecc_scalar_stt *)pa_addend,
					 p[3].value.a);
	if (hsm_res) {
		ret = TEE_ERROR_GENERIC;
		goto end;
	}

	DMSG("ECC Scalar Multiplication and Addition done");

end:
	crypto_bignum_free(key_x);
	crypto_bignum_free(key_y);
	crypto_bignum_free(key_d);
	free(keypair);
	free(mult);
	free(addend);

	return ret;
}

/*
 * Export of ECC key pair in protected form
 */
#define ECC_KEY_M4_SIZE		32
#define ECC_KEY_M5_SIZE		16
static TEE_Result hsm_export_ecc_key_protect(uint32_t type,
					     TEE_Param p[TEE_NUM_PARAMS])
{
	TEE_Result ret = TEE_SUCCESS;
	uint32_t hsm_res = 0;
	tee_mm_entry_t *mm_m1 = NULL;
	tee_mm_entry_t *mm_m2 = NULL;
	tee_mm_entry_t *mm_m3 = NULL;
	tee_mm_entry_t *mm_m45 = NULL;
	uint8_t *m1 = NULL;
	uint8_t *m2 = NULL;
	uint8_t *m3 = NULL;
	uint8_t *m45 = NULL;
	paddr_t pa_m1 = 0;
	paddr_t pa_m2 = 0;
	paddr_t pa_m3 = 0;
	paddr_t pa_m45 = 0;

	uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INOUT,
					  TEE_PARAM_TYPE_MEMREF_INOUT,
					  TEE_PARAM_TYPE_MEMREF_OUTPUT,
					  TEE_PARAM_TYPE_MEMREF_OUTPUT);

	if (exp_pt != type) {
		DMSG("bad parameter types");
		ret = TEE_ERROR_BAD_PARAMETERS;
		goto end;
	}
	if (!check_memref_param(type, p)) {
		DMSG("bad memref memory area");
		ret = TEE_ERROR_BAD_PARAMETERS;
		goto end;
	}
	if (p[3].memref.size < (ECC_KEY_M4_SIZE + ECC_KEY_M5_SIZE)) {
		ret = TEE_ERROR_BAD_PARAMETERS;
		goto end;
	}
	/*
	 * HSM is configured to only write output data in secure memory area
	 * so reallocate output buffer accordingly if needed
	 */
	if (!check_output_buf(p[0].memref.buffer, p[0].memref.size, &mm_m1,
			      &m1, &pa_m1) ||
	    !check_output_buf(p[1].memref.buffer, p[1].memref.size, &mm_m2,
			      &m2, &pa_m2) ||
	    !check_output_buf(p[2].memref.buffer, p[2].memref.size, &mm_m3,
			      &m3, &pa_m3) ||
	    !check_output_buf(p[3].memref.buffer, p[3].memref.size, &mm_m45,
			      &m45, &pa_m45)) {
		ret = TEE_ERROR_OUT_OF_MEMORY;
		goto end;
	}

	/* Flush input/output data buffer before notifying HSM */
	dcache_clean_range((void *)m1, p[0].memref.size);
	dcache_clean_range((void *)m2, p[1].memref.size);
	dcache_clean_range((void *)m3, p[2].memref.size);
	dcache_clean_range((void *)m45, p[3].memref.size);

	/* Get ECC NVM key from HSM and update internal key array accordingly */
	/* m4 and m5 (m45) are concatenated in the same output buffer */
	hsm_res = cse_ecc_export_privatekey((uint8_t *)pa_m1, (uint8_t *)pa_m2,
					    (uint8_t *)pa_m3, (uint8_t *)pa_m45,
					    (uint8_t *)pa_m45 + ECC_KEY_M4_SIZE);
	if (hsm_res) {
		ret = TEE_ERROR_GENERIC;
		goto end;
	}

	/* Convert size of M2 for the RAM Key Export patch for TC3p Cut2BF */
	hsm_res = cse_ecc_convert_M2_size_ram_key_export((uint8_t *)pa_m1,
							 (uint8_t *)pa_m2,
							 (uint8_t *)pa_m3);
	if (hsm_res) {
		ret = TEE_ERROR_GENERIC;
		goto end;
	}

	/* Invalidate output data buffer to get answer from HSM */
	dcache_inv_range((void *)m1, p[0].memref.size);
	dcache_inv_range((void *)m2, p[1].memref.size);
	dcache_inv_range((void *)m3, p[2].memref.size);
	dcache_inv_range((void *)m45, p[3].memref.size);

	/* Copy the output buffers if they have been reallocated */
	if (m1 != p[0].memref.buffer)
		memcpy(p[0].memref.buffer, m1, p[0].memref.size);
	if (m2 != p[1].memref.buffer)
		memcpy(p[1].memref.buffer, m2, p[1].memref.size);
	if (m3 != p[2].memref.buffer)
		memcpy(p[2].memref.buffer, m3, p[2].memref.size);
	if (m45 != p[3].memref.buffer)
		memcpy(p[3].memref.buffer, m45, p[3].memref.size);

	DMSG("ECC key pair exported in protected form");

end:
	mm_sec_free(mm_m1);
	mm_sec_free(mm_m2);
	mm_sec_free(mm_m3);
	mm_sec_free(mm_m45);

	return ret;
}

/*
 * Load of ECC key pair in protected form
 */
#define ECC_M1_KEY_ID_OFFSET	15
static TEE_Result hsm_load_ecc_key_protect(uint32_t type,
					   TEE_Param p[TEE_NUM_PARAMS])
{
	TEE_Result ret = TEE_SUCCESS;
	uint32_t hsm_res = 0;
	struct ecc_pub_key_stt *keypub = calloc(1,
				sizeof(struct ecc_pub_key_stt));
	paddr_t pa_m1 = ALIAS_TO_PHYS(virt_to_phys((void *)p[0].memref.buffer));
	paddr_t pa_m2 = ALIAS_TO_PHYS(virt_to_phys((void *)p[1].memref.buffer));
	paddr_t pa_m3 = ALIAS_TO_PHYS(virt_to_phys((void *)p[2].memref.buffer));
	uint8_t *m45 = calloc(1, ECC_KEY_M4_SIZE + ECC_KEY_M5_SIZE);
	paddr_t pa_m45 = 0;
	tee_mm_entry_t *mm_key_xy = NULL;
	uint8_t *key_xy = NULL;
	paddr_t pa_key_xy = 0;
	uint32_t key_id = 0;
	uint32_t key_size = p[3].memref.size / 2;

	uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
					  TEE_PARAM_TYPE_MEMREF_INPUT,
					  TEE_PARAM_TYPE_MEMREF_INPUT,
					  TEE_PARAM_TYPE_MEMREF_INOUT);

	if (exp_pt != type) {
		DMSG("bad parameter types");
		ret = TEE_ERROR_BAD_PARAMETERS;
		goto end;
	}
	if (!check_memref_param(type, p)) {
		DMSG("bad memref memory area");
		ret = TEE_ERROR_BAD_PARAMETERS;
		goto end;
	}
	if (!keypub || !m45) {
		ret = TEE_ERROR_OUT_OF_MEMORY;
		goto end;
	}
	/*
	 * HSM is configured to only write output data in secure memory area
	 * so reallocate output buffer accordingly if needed
	 */
	if (!check_output_buf(p[3].memref.buffer, p[3].memref.size, &mm_key_xy,
			      &key_xy, &pa_key_xy)) {
		ret = TEE_ERROR_OUT_OF_MEMORY;
		goto end;
	}
	pa_m45 = ALIAS_TO_PHYS(virt_to_phys((void *)m45));

	/* Extract from M1 the index of the key to load */
	key_id = *((uint8_t *)p[0].memref.buffer + ECC_M1_KEY_ID_OFFSET) >> 4;

	/* Flush input/output data buffer before notifying HSM */
	dcache_clean_range((void *)p[0].memref.buffer, p[0].memref.size);
	dcache_clean_range((void *)p[1].memref.buffer, p[1].memref.size);
	dcache_clean_range((void *)p[2].memref.buffer, p[2].memref.size);
	dcache_clean_range((void *)m45, ECC_KEY_M4_SIZE + ECC_KEY_M5_SIZE);

	/* Get ECC NVM key from HSM and update internal key array accordingly */
	/* m4 and m5 (m45) are concatenated in the same output buffer */
	hsm_res = cse_ecc_load_key((uint8_t *)pa_m1, (uint8_t *)pa_m2,
				   (uint8_t *)pa_m3, (uint8_t *)pa_m45,
				   (uint8_t *)pa_m45 + ECC_KEY_M4_SIZE);
	if (hsm_res) {
		ret = TEE_ERROR_GENERIC;
		goto end;
	}

	/* Invalidate output data buffer to get answer from HSM */
	dcache_inv_range((void *)m45, ECC_KEY_M4_SIZE + ECC_KEY_M5_SIZE);

	/* TODO: Not supported in cut2_BF */
	/* Check loading attestation by comparing the M4 and M5 messages
	 * returned with the ones sent in input (stored in key_xy buffer) */
#if 0
	if (memcmp(key_xy, m45, ECC_KEY_M4_SIZE + ECC_KEY_M5_SIZE)) {
		ret = TEE_ERROR_GENERIC;
		goto end;
	}
#endif
	/*
	 * Exports the public key part of the ECC key pair loaded
	 */
	memset(key_xy, 0, p[3].memref.size);
	keypub->pub_key_coordX = key_xy;
	keypub->pub_key_coordX_size = key_size;
	keypub->pub_key_coordY = key_xy + key_size;
	keypub->pub_key_coordY_size = key_size;

	ret = ecc_get_pub_key(key_id, (void *)keypub);
	if (ret != TEE_SUCCESS)
		goto end;

	/* Copy the output buffers if they have been reallocated */
	if (key_xy != p[3].memref.buffer)
		memcpy(p[3].memref.buffer, key_xy, p[3].memref.size);

	DMSG("ECC key pair loaded in protected form");

end:
	free(keypub);
	free(m45);
	mm_sec_free(mm_key_xy);

	return ret;
}

#endif /* CFG_CRYPTO_ECC && !CFG_CRYPTO_ECC_FROM_CRYPTOLIB */

/*
 * Trusted Application Entry Points
 */

static TEE_Result create_ta(void)
{
	DMSG("create entry point for pseudo TA \"%s\"", TA_NAME);
	return TEE_SUCCESS;
}

static void destroy_ta(void)
{
	DMSG("destroy entry point for pseudo ta \"%s\"", TA_NAME);
}

static TEE_Result open_session(uint32_t nParamTypes __unused,
			       TEE_Param pParams[TEE_NUM_PARAMS] __unused,
			       void **ppSessionContext __unused)
{
	DMSG("open entry point for pseudo ta \"%s\"", TA_NAME);
	return TEE_SUCCESS;
}

static void close_session(void *pSessionContext __unused)
{
	DMSG("close entry point for pseudo ta \"%s\"", TA_NAME);
}

static TEE_Result invoke_command(void *pSessionContext __unused,
				 uint32_t nCommandID, uint32_t nParamTypes,
				 TEE_Param pParams[TEE_NUM_PARAMS])
{
	FMSG("command entry point for pseudo ta \"%s\"", TA_NAME);

	switch (nCommandID) {
	case PTA_HSM_CMD_GET_FW_ID:
		return hsm_get_fw_id(nParamTypes, pParams);
	case PTA_HSM_CMD_TLSV12_PRF:
		return hsm_tls_prf(nParamTypes, pParams);
#if defined(CFG_CRYPTO_ECC) && !defined(CFG_CRYPTO_ECC_FROM_CRYPTOLIB)
	case PTA_HSM_CMD_ECC_KEY_REGEN:
		return hsm_ecc_key_regen(nParamTypes, pParams);
	case PTA_HSM_CMD_KS_INIT:
		return hsm_key_storage_init(nParamTypes, pParams);
	case PTA_HSM_CMD_PUB_KEY_EXP:
		return hsm_key_pub_export(nParamTypes, pParams);
	case PTA_HSM_CMD_LOAD_ECC_RAM_KEY:
		return hsm_load_ecc_ram_key(nParamTypes, pParams);
	case PTA_HSM_CMD_ECC_SCALAR_MUL_ADD:
		return hsm_ecc_scalar_mul_add(nParamTypes, pParams);
	case PTA_HSM_CMD_EXP_ECC_KEY_PROT:
		return hsm_export_ecc_key_protect(nParamTypes, pParams);
	case PTA_HSM_CMD_LOAD_ECC_KEY_PROT:
		return hsm_load_ecc_key_protect(nParamTypes, pParams);
#endif /* CFG_CRYPTO_ECC && !CFG_CRYPTO_ECC_FROM_CRYPTOLIB */
	default:
		break;
	}
	return TEE_ERROR_BAD_PARAMETERS;
}

pseudo_ta_register(.uuid = PTA_HSM_UUID, .name = TA_NAME,
		   .flags = PTA_DEFAULT_FLAGS,
		   .create_entry_point = create_ta,
		   .destroy_entry_point = destroy_ta,
		   .open_session_entry_point = open_session,
		   .close_session_entry_point = close_session,
		   .invoke_command_entry_point = invoke_command);
