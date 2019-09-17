/**
 * @file sta_ccc_test.c
 * @brief CCC tests file
 *
 * Copyright (C) ST-Microelectronics SA 2016
 * @author: APG-MID team
 */

#include <string.h>
#include <errno.h>

/*#define DEBUG*/
#ifdef DEBUG
#ifdef TRACE_LEVEL
#undef TRACE_LEVEL
#define TRACE_LEVEL TRACE_LEVEL_INFO
#endif
#endif

#include "sta_common.h"
#include "FreeRTOS.h"
#include "task.h"
#include "trace.h"
#include "utils.h"
#include "sta_type.h"

#include "sta_ccc_if.h"

#if defined(DEBUG) & 0
#define MOVE_LOOP_ON_FAILURE() { while (1); }
#define HASH_LOOP_ON_FAILURE() { while (1); }
#define AES_LOOP_ON_FAILURE() { while (1); }
#define RSA_LOOP_ON_FAILURE() { while (1); }
#define RNG_LOOP_ON_FAILURE() { while (1); }
#else
#define MOVE_LOOP_ON_FAILURE()
#define HASH_LOOP_ON_FAILURE()
#define AES_LOOP_ON_FAILURE()
#define RSA_LOOP_ON_FAILURE()
#define RNG_LOOP_ON_FAILURE()
#endif

#define DIV_ROUND_UP(n, d)   (((n) + (d) - 1) / (d))
#define SIZE_IN_BYTES(size_in_bits) DIV_ROUND_UP(size_in_bits, 8)
#define SIZE_IN_WORDS(size_in_bits) DIV_ROUND_UP(SIZE_IN_BYTES(size_in_bits), \
						 sizeof(unsigned int))

static unsigned char *in = (unsigned char *)ESRAM_A7_BASE + 0x28000;
static unsigned char *out = (unsigned char *)ESRAM_A7_BASE + 0x68000;
static unsigned char *digest = (unsigned char *)ESRAM_A7_BASE + 0x78000;
static unsigned char aes_key[32] = {0,};
static unsigned char aes_ae_key[32] = {0,};
#define NR_SPECIAL_KEY_SLOTS 2
static unsigned int aes_biggest_requested_size[NR_SPECIAL_KEY_SLOTS] = {0,};
#define AES_BLOCK_SIZE 16
#define AES_IV_SIZE AES_BLOCK_SIZE
static unsigned char iv[AES_IV_SIZE] = {0,};
#define AES_GCM_TAG_SIZE (128 / 8)
static unsigned char tag[AES_GCM_TAG_SIZE] = {0,};
static unsigned char mod[2048 / 8] = {0,};
static unsigned char exp[2048 / 8] = {0,};
static unsigned char mod_ecc[MAX_ECC_MODULUS_SIZE] = {0,};
static unsigned char a_ec[MAX_ECC_MODULUS_SIZE] = {0,};
static unsigned char k_ecc[MAX_ECC_EXPONENT_SIZE] = {0,};
static unsigned char n_ecdsa[MAX_ECC_ORDER_SIZE] = {0,};
static unsigned char d_ecdsa[MAX_ECC_ORDER_SIZE] = {0,};
static unsigned char k_ecdsa[MAX_ECC_ORDER_SIZE] = {0,};
static unsigned char e_ecdsa[MAX_ECC_ORDER_SIZE] = {0,};
static unsigned char q_ecdsa[2 * MAX_ECC_MODULUS_SIZE] = {0,};
static unsigned char r_ecdsa[MAX_ECC_ORDER_SIZE] = {0,};
static unsigned char s_ecdsa[MAX_ECC_ORDER_SIZE] = {0,};
static unsigned char arith_alpha[MAX_ECC_MODULUS_SIZE] = {0,};
static unsigned char arith_beta[MAX_ECC_MODULUS_SIZE] = {0,};
static unsigned char arith_gamma[MAX_ECC_MODULUS_SIZE] = {0,};
static unsigned int update;

#define DEFAULT_STRING "Too many secrets"
#define AES_KEY "seTec astronomy 0123456701234567"
#define PALINDROME "3333333333333333"
#define DATA_SIZE 100

#if defined(DEBUG) & 0
void dump_addr(__maybe_unused struct ccc_scatter *scatter)
{
	unsigned int i;

	for (i = 0; i < scatter->nr_entries; i++) {
#if TRACE_LEVEL >= TRACE_LEVEL_INFO
		struct entry *entry = &scatter->entries[i];
#endif

		TRACE_INFO("%s: req entry[%d]->src %08x\n", __func__, i,
			   entry->src);
		TRACE_INFO("%s: req entry[%d]->dst %08x\n", __func__, i,
			   entry->dst);
		TRACE_INFO("%s: req entry[%d]->size %d\n", __func__, i,
			   entry->size);
	}
}

#define DUMP_ADDR(scatter) dump_addr(scatter)
#else
#define DUMP_ADDR(_)
#endif

void hexdump(unsigned char *p, unsigned int size)
{
	unsigned int i;

	TRACE_ERR("");
	for (i = 0; i < size; i++)
		trace_printf("%02x", *(p++));
	trace_printf("\n");
}

#ifdef DEBUG
#define HEXDUMP(p, size) hexdump(p, size)
#else
#define HEXDUMP(_1, _2)
#endif

void perform_move_test(struct ccc_crypto_req *req, const char *title,
		       const char *expected, unsigned int size)
{
	void *ctx;
	int err;
	struct entry *entry;

	DUMP_ADDR(&req->scatter);

	ctx = ccc_crypto_init(req);
	if (!ctx) {
		TRACE_ERR("%s: ccc_crypto_init fails\n", __func__);
		return;
	}

	entry = &req->scatter.entries[0];	/* Consider 1st entry only */
	err = ccc_crypto_run(ctx);
	if (err)
		TRACE_INFO("%s: ccc_crypto_run returns %d\n", __func__, err);

	if (0 == memcmp(entry->dst, expected, size))
		TRACE_ERR("%s [passed]\n", title);
	else {
		TRACE_ERR("%s [failed]\n", title);
		HEXDUMP(entry->dst, entry->size);
		MOVE_LOOP_ON_FAILURE();
	}
}

void perform_move_tests(struct ccc_crypto_req *req)
{
#define MOVE "MOVE sum"
	unsigned int size;

	memset((unsigned char *)in, '\0', DATA_SIZE);
	strncpy((char *)in, DEFAULT_STRING,
		strlen(DEFAULT_STRING) + sizeof('\0'));
	size = strlen((const char *)in);
	memset((unsigned char *)out, '\0', DATA_SIZE);
	strncpy((char *)out, DEFAULT_STRING,
		strlen(DEFAULT_STRING) + sizeof('\0'));

	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, in, out, size);

	perform_move_test(req, MOVE, (char *)in, size);
}

void perform_hash_test(struct ccc_crypto_req *req, const char *title,
		       const char *expected, unsigned int size)
{
	void *ctx;
	int err;

	DUMP_ADDR(&req->scatter);

	ctx = ccc_crypto_init(req);
	if (!ctx) {
		TRACE_ERR("%s: ccc_crypto_init fails\n", __func__);
		return;
	}

	err = ccc_crypto_run(ctx);
	if (err)
		TRACE_INFO("%s: ccc_crypto_run returns %d\n", __func__, err);

	if (0 == memcmp(req->hash.digest, expected, size))
		TRACE_ERR("%s [passed]\n", title);
	else {
		TRACE_ERR("%s [failed]\n", title);
		HEXDUMP(req->hash.digest, size);
		HASH_LOOP_ON_FAILURE();
	}
}

void *perform_hash_test_mb_init(struct ccc_crypto_req *req, const char *title)
{
	void *ctx;

	update = 0;
	ctx = ccc_crypto_init(req);
	if (!ctx) {
		TRACE_INFO("%s: ccc_crypto_init fails\n", __func__);
		TRACE_ERR("%s [failed]\n", title);
		HASH_LOOP_ON_FAILURE();
	}
	return ctx;
}

int perform_hash_test_mb_update(void *ctx, struct ccc_scatter *scatter,
				const char *title)
{
	int err;

	update++;
	err = ccc_crypto_update(ctx, scatter);
	if (err) {
		TRACE_INFO("%s: ccc_crypto_update #%d fails\n", __func__,
			   update);
		TRACE_ERR("%s [failed]\n", title);
		HASH_LOOP_ON_FAILURE();
	}
	return err;
}

void perform_hash_test_mb_finish(struct ccc_crypto_req *req, void *ctx,
				 const char *title, const char *expected,
				 unsigned int size)
{
	int err;

	err = ccc_crypto_finish(ctx);
	if (err)
		TRACE_INFO("%s: ccc_crypto_finish fails\n", __func__);

	if (0 == memcmp(req->hash.digest, expected, size)) {
		TRACE_ERR("%s [passed]\n", title);
	} else {
		TRACE_ERR("%s [failed]\n", title);
		HEXDUMP(req->hash.digest, size);
		HASH_LOOP_ON_FAILURE();
	}
}

void perform_hash_big_data_tests(struct ccc_crypto_req *req, unsigned char *src,
				 unsigned char *dst)
{
#define SHA256_SIZE 32

#define SHA256_64K_DIGEST "\x7d\xac\xa2\x09\x5d\x04\x38\x26\x0f\xa8\x49\x18\x3d\xfc\x67\xfa\xa4\x59\xfd\xf4\x93\x6e\x1b\xc9\x1e\xec\x6b\x28\x1b\x27\xe4\xc2"
#define SHA256_64K "SHA256 64K sum"

	unsigned char *p, c = '\0';
	unsigned int i;
	void *ctx;

	ccc_scatter_init(&req->scatter);
	/* 64KB > MAX_CHUNK_SIZE */
	p = src;
	ccc_scatter_append(&req->scatter, p, dst, 64 * 1024);
	for (i = 0; i < (64 * 1024); i++)
		*(p++) = c++;

	req->hash.alg = HASH_SHA_256_ALG;
	perform_hash_test(req, SHA256_64K,
			  SHA256_64K_DIGEST, SHA256_SIZE);

#define SHA256_63K_DIGEST "\x11\xad\x2c\x34\xc2\x4f\x2c\xf4\x15\x3b\x5e\xaf\x5f\xc4\xa3\xad\x89\x9b\x09\x0b\x22\x17\x48\xe9\x74\x94\x3d\x03\x13\x73\x8c\x25"
#define SHA256_63K_2_PERSISTENT_CHUNKS "SHA256 sum of 2 persistent scattered chunks for a total of 63K"
#define _21K (21 * 1024)
#define _42K (42 * 1024)

	c = '\0';
	ccc_scatter_init(&req->scatter);
	/* First chunk of 21KB */
	p = src;
	ccc_scatter_append(&req->scatter, p, dst, _21K);
	for (i = 0; i < _21K; i++)
		*(p++) = c++;
	/* Skip 1KB */
	p += 1024;
	/* Skip one more byte to induce a misalignment */
	p++;
	/* Second chunk of 42KB */
	ccc_scatter_append(&req->scatter, p, dst + (p - src), _42K);
	for (i = 0; i < _42K; i++)
		*(p++) = c++;

	req->hash.alg = HASH_SHA_256_ALG;
	perform_hash_test(req, SHA256_63K_2_PERSISTENT_CHUNKS,
			  SHA256_63K_DIGEST, SHA256_SIZE);

#define SHA256_63K_DIGEST "\x11\xad\x2c\x34\xc2\x4f\x2c\xf4\x15\x3b\x5e\xaf\x5f\xc4\xa3\xad\x89\x9b\x09\x0b\x22\x17\x48\xe9\x74\x94\x3d\x03\x13\x73\x8c\x25"
#define SHA256_63K_2_TRANSIENT_CHUNKS "SHA256 sum of 2 transient scattered chunks for a total of 63K"
#define _21K (21 * 1024)
#define _42K (42 * 1024)

	req->hash.alg = HASH_SHA_256_ALG;
	c = '\0';
	ccc_scatter_init(&req->scatter);
	ctx = perform_hash_test_mb_init(req, SHA256_63K_2_TRANSIENT_CHUNKS);
	if (!ctx)
		goto skip_S62TC;
	/* First chunk of 21KB */
	p = src;
	ccc_scatter_append(&req->scatter, p, dst, _21K);
	for (i = 0; i < _21K; i++)
		*(p++) = c++;

	if (perform_hash_test_mb_update(ctx, &req->scatter,
					SHA256_63K_2_TRANSIENT_CHUNKS))
		goto skip_S62TC;
	/* Scrub the first buffer to make it transient */
	memset(src, 0xaa, _21K);
	/* Skip 1KB */
	p += 1024;
	/* Skip one more byte to induce a misalignment */
	p++;
	ccc_scatter_deinit(&req->scatter);
	/* Second chunk of 42KB */
	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, p, dst + (p - src), _42K);
	for (i = 0; i < _42K; i++)
		*(p++) = c++;

	if (perform_hash_test_mb_update(ctx, &req->scatter,
					SHA256_63K_2_TRANSIENT_CHUNKS))
		goto skip_S62TC;
	perform_hash_test_mb_finish(req, ctx, SHA256_63K_2_TRANSIENT_CHUNKS,
				    SHA256_63K_DIGEST, SHA256_SIZE);
skip_S62TC:
	ccc_scatter_deinit(&req->scatter);

#define SHA1_SIZE 20
#define SHA1_PBKDF2_DIGEST "\xd7\xa5\xe1\x4f\x25\x25\x77\xcd\x67\xbf\xae\x5f\x82\x5f\x86\xa4\xb2\x9b\xd9\x02"
#define PBKDF2_CHUNK_1 "\x46\x57\x45\x45\x72\x77\x62\x77\x54\x06\x06\x77\x74\x01\x6f\x4e\x72\x62\x62\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36"
#define PBKDF2_CHUNK_1_SIZE 64
#define PBKDF2_CHUNK_2 "\x73\x61\x6c\x74\x4b\x45\x59\x62\x63\x54\x63\x58\x48\x43\x42\x78\x74\x6a\x44"
#define PBKDF2_CHUNK_2_SIZE 19
#define PBKDF2_CHUNK_3 "\x00\x00\x00\x01"
#define PBKDF2_CHUNK_3_SIZE 4
#define SHA1_PBKDF2_TRANSIENT_CHUNKS "SHA1 sum of 3 transient chunks used for PBKDF2"

	req->hash.alg = HASH_SHA_1_ALG;
	c = '\0';
	TRACE_INFO("init\n");
	ccc_scatter_init(&req->scatter);
	ctx = perform_hash_test_mb_init(req, SHA1_PBKDF2_TRANSIENT_CHUNKS);
	if (!ctx)
		goto skip_SPD;
	/* First chunk of 64B */
	p = src;
	TRACE_INFO("First chunk of 64B\n");
	ccc_scatter_append(&req->scatter, p, dst, PBKDF2_CHUNK_1_SIZE);
	memcpy(p, PBKDF2_CHUNK_1, PBKDF2_CHUNK_1_SIZE);
	if (perform_hash_test_mb_update(ctx, &req->scatter,
					SHA1_PBKDF2_TRANSIENT_CHUNKS))
		goto skip_SPD;
	/* Scrub the first buffer to make it transient */
	memset(p, 0xaa, PBKDF2_CHUNK_1_SIZE);
	p += PBKDF2_CHUNK_1_SIZE;
	/* Second chunk of 19B */
	ccc_scatter_init(&req->scatter);
	TRACE_INFO("chunk of 19B\n");
	ccc_scatter_append(&req->scatter, p, dst + (p - src),
			   PBKDF2_CHUNK_2_SIZE);
	memcpy(p, PBKDF2_CHUNK_2, PBKDF2_CHUNK_2_SIZE);
	if (perform_hash_test_mb_update(ctx, &req->scatter,
					SHA1_PBKDF2_TRANSIENT_CHUNKS))
		goto skip_SPD;
	/* Scrub the second buffer to make it transient */
	memset(p, 0xaa, PBKDF2_CHUNK_2_SIZE);
	p += PBKDF2_CHUNK_2_SIZE;
	p++;
	/* Third chunk of 4B */
	ccc_scatter_init(&req->scatter);
	TRACE_INFO("chunk of 4B\n");
	ccc_scatter_append(&req->scatter, p, dst + (p - src),
			   PBKDF2_CHUNK_3_SIZE);
	memcpy(p, PBKDF2_CHUNK_3, PBKDF2_CHUNK_3_SIZE);
	if (perform_hash_test_mb_update(ctx, &req->scatter,
					SHA1_PBKDF2_TRANSIENT_CHUNKS))
		goto skip_SPD;
	perform_hash_test_mb_finish(req, ctx, SHA1_PBKDF2_TRANSIENT_CHUNKS,
				    SHA1_PBKDF2_DIGEST, SHA1_SIZE);
skip_SPD:
	ccc_scatter_deinit(&req->scatter);

#define SHA1_PBKDF2_DIGEST "\xd7\xa5\xe1\x4f\x25\x25\x77\xcd\x67\xbf\xae\x5f\x82\x5f\x86\xa4\xb2\x9b\xd9\x02"
#define PBKDF2_DATA "\x46\x57\x45\x45\x72\x77\x62\x77\x54\x06\x06\x77\x74\x01\x6f\x4e\x72\x62\x62\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x73\x61\x6c\x74\x4b\x45\x59\x62\x63\x54\x63\x58\x48\x43\x42\x78\x74\x6a\x44\x00\x00\x00\x01"
#define PBKDF2_DATA_SIZE 87
#define SHA1_PBKDF2 "SHA1 sum of 1 chunk used for PBKDF2"

	req->hash.alg = HASH_SHA_1_ALG;
	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, src, dst, PBKDF2_DATA_SIZE);
	memcpy(src, PBKDF2_DATA, PBKDF2_DATA_SIZE);
	perform_hash_test(req, SHA1_PBKDF2, SHA1_PBKDF2_DIGEST, SHA1_SIZE);
	ccc_scatter_deinit(&req->scatter);
}

void perform_hash_intermix_tests(struct ccc_crypto_req *req, unsigned char *src,
				 unsigned char *dst, unsigned char *src_2,
				 unsigned char *dst_2)
{
	unsigned char *p;
	void *ctx;

#define SHA1_SIZE 20
#define SHA1_PBKDF2_DIGEST "\xd7\xa5\xe1\x4f\x25\x25\x77\xcd\x67\xbf\xae\x5f\x82\x5f\x86\xa4\xb2\x9b\xd9\x02"
#define PBKDF2_CHUNK_1 "\x46\x57\x45\x45\x72\x77\x62\x77\x54\x06\x06\x77\x74\x01\x6f\x4e\x72\x62\x62\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36"
#define PBKDF2_CHUNK_1_SIZE 64
#define PBKDF2_CHUNK_2 "\x73\x61\x6c\x74\x4b\x45\x59\x62\x63\x54\x63\x58\x48\x43\x42\x78\x74\x6a\x44"
#define PBKDF2_CHUNK_2_SIZE 19
#define SHA256_SIZE 32
#define SHA256_DIGEST "\x49\xd4\xeb\x24\x31\xb6\x50\x14\xa2\x89\x48\xcd\xc2\xfb\xc3\x74\x7d\xf7\x99\x36\xbd\x28\x69\xf4\xc9\x70\x1b\xf6\x72\x20\xa5\x15"
#define SHA256_NESTED "SHA256 sum nested"
#define PBKDF2_CHUNK_3 "\x00\x00\x00\x01"
#define PBKDF2_CHUNK_3_SIZE 4
#define SHA1_SHA256_INTERMIXED "SHA1 intermixed with SHA256"

	req->hash.alg = HASH_SHA_1_ALG;
	TRACE_INFO("init\n");
	ccc_scatter_init(&req->scatter);
	ctx = perform_hash_test_mb_init(req, SHA1_SHA256_INTERMIXED);
	if (!ctx)
		goto skip_SSI;
	/* First chunk of 64B */
	p = src;
	TRACE_INFO("First chunk of 64B\n");
	ccc_scatter_append(&req->scatter, p, dst, PBKDF2_CHUNK_1_SIZE);
	memcpy(p, PBKDF2_CHUNK_1, PBKDF2_CHUNK_1_SIZE);
	if (perform_hash_test_mb_update(ctx, &req->scatter,
					SHA1_SHA256_INTERMIXED))
		goto skip_SSI;
	/* Scrub the first buffer to make it transient */
	memset(p, 0xaa, PBKDF2_CHUNK_1_SIZE);
	p += PBKDF2_CHUNK_1_SIZE;
	/* Second chunk of 19B */
	ccc_scatter_init(&req->scatter);
	TRACE_INFO("chunk of 19B\n");
	ccc_scatter_append(&req->scatter, p, dst + (p - src),
			   PBKDF2_CHUNK_2_SIZE);
	memcpy(p, PBKDF2_CHUNK_2, PBKDF2_CHUNK_2_SIZE);
	if (perform_hash_test_mb_update(ctx, &req->scatter,
					SHA1_SHA256_INTERMIXED))
		goto skip_SSI;
	/* Scrub the second buffer to make it transient */
	memset(p, 0xaa, PBKDF2_CHUNK_2_SIZE);
	p += PBKDF2_CHUNK_2_SIZE;
	p++;
	/* Insert a single block SHA256 computation in the middle */
	memset(src_2, '\0', strlen(DEFAULT_STRING) + sizeof('\0'));
	strncpy((char *)src_2, DEFAULT_STRING,
		strlen(DEFAULT_STRING) + sizeof('\0'));
	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, src_2, dst_2, strlen((const char *)src_2));
	req->hash.alg = HASH_SHA_256_ALG;
	perform_hash_test(req, SHA256_NESTED, SHA256_DIGEST, SHA256_SIZE);
	/* Third chunk of 4B */
	ccc_scatter_init(&req->scatter);
	TRACE_INFO("chunk of 4B\n");
	ccc_scatter_append(&req->scatter, p, dst + (p - src),
			   PBKDF2_CHUNK_3_SIZE);
	memcpy(p, PBKDF2_CHUNK_3, PBKDF2_CHUNK_3_SIZE);
	if (perform_hash_test_mb_update(ctx, &req->scatter,
					SHA1_SHA256_INTERMIXED))
		goto skip_SSI;
	perform_hash_test_mb_finish(req, ctx, SHA1_SHA256_INTERMIXED,
				    SHA1_PBKDF2_DIGEST, SHA1_SIZE);
skip_SSI:
	ccc_scatter_deinit(&req->scatter);
}

void perform_hash_tests(struct ccc_crypto_req *req, unsigned char *src,
			unsigned char *dst)
{
#define MD5_SIZE 16
#define MD5_DIGEST "\x87\xc5\xa3\xdb\x8a\x28\x48\x2f\x2b\x54\xce\x53\x77\x09\x33\xb7"
#define MD5 "MD5 sum"
#define SHA1_SIZE 20
#define SHA1_DIGEST "\xd5\xc0\x0e\x79\xf8\x27\x4a\x8c\xae\x8e\x88\xd5\x57\xc9\xee\x7a\xed\x6d\xae\xf7"
#define SHA1 "SHA1 sum"
#define SHA224_SIZE 28
#define SHA224_DIGEST "\xd0\x9e\x58\xac\x83\xa2\xc9\x1d\x6d\x0d\x50\xa5\xd4\x26\x26\x00\x5a\x27\x16\x55\x46\x92\xba\x45\xcf\x69\x8b\xde"
#define SHA224 "SHA224 sum"
#define SHA256_SIZE 32
#define SHA256_DIGEST "\x49\xd4\xeb\x24\x31\xb6\x50\x14\xa2\x89\x48\xcd\xc2\xfb\xc3\x74\x7d\xf7\x99\x36\xbd\x28\x69\xf4\xc9\x70\x1b\xf6\x72\x20\xa5\x15"
#define SHA256 "SHA256 sum"

	memset((unsigned char *)in, '\0', DATA_SIZE);
	strncpy((char *)in, DEFAULT_STRING,
		strlen(DEFAULT_STRING) + sizeof('\0'));
	memset((unsigned char *)out, '\0', DATA_SIZE);
	strncpy((char *)out, DEFAULT_STRING,
		strlen(DEFAULT_STRING) + sizeof('\0'));

	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, src, dst, strlen((const char *)in));

	req->hash.alg = HASH_MD5_ALG;
	perform_hash_test(req, MD5, MD5_DIGEST, MD5_SIZE);

	req->hash.alg = HASH_SHA_1_ALG;
	perform_hash_test(req, SHA1, SHA1_DIGEST, SHA1_SIZE);

	req->hash.alg = HASH_SHA_224_ALG;
	perform_hash_test(req, SHA224, SHA224_DIGEST, SHA224_SIZE);

	req->hash.alg = HASH_SHA_256_ALG;
	perform_hash_test(req, SHA256, SHA256_DIGEST, SHA256_SIZE);
}

static bool is_sp_key_slot(unsigned int slot)
{
	return slot < NR_SPECIAL_KEY_SLOTS;
}

void perform_aes_test(struct ccc_crypto_req *req, const char *title,
		      const char *expected)
{
	void *ctx;
	int err;
	struct entry *entry;

	DUMP_ADDR(&req->scatter);

	if (is_sp_key_slot(req->sym.key_slot)) {
		unsigned int biggest_size =
			aes_biggest_requested_size[req->sym.key_slot];

		if (req->sym.key_size == AES_KEYSIZE_192) {
			TRACE_ERR("%s [skipped] (192b not supported on sp slot "
				  "#%d)\n", title, req->sym.key_slot);
			return;
		}
		if (req->sym.key_size < biggest_size) {
			TRACE_ERR("%s [skipped] (sp slot #%d formerly "
				  "provisionned with a biggest key of %dbits)\n",
				  title, req->sym.key_slot, 8 * biggest_size);
			return;
		}
	}

	ctx = ccc_crypto_init(req);
	if (!ctx) {
		TRACE_ERR("%s: ccc_crypto_init fails\n", __func__);
		return;
	}

	entry = &req->scatter.entries[0];	/* Consider 1st entry only */
	err = ccc_crypto_run(ctx);
	if (err)
		TRACE_INFO("%s: ccc_crypto_run returns %d\n", __func__, err);

	if (0 == memcmp(entry->dst, expected, entry->size))
		TRACE_ERR("%s [passed]\n", title);
	else {
		TRACE_ERR("%s [failed]\n", title);
		HEXDUMP(entry->dst, entry->size);
		AES_LOOP_ON_FAILURE();
	}
}

void perform_aes_ccm_test(struct ccc_crypto_req *req, const char *title,
			  unsigned char *output, const char *expected,
			  unsigned int size, const char *tag_expected,
			  unsigned int tag_size)
{
	void *ctx;
	int err;

	DUMP_ADDR(&req->scatter);

	if (is_sp_key_slot(req->sym.key_slot)) {
		unsigned int biggest_size =
			aes_biggest_requested_size[req->sym.key_slot];

		if (req->sym.key_size == AES_KEYSIZE_192) {
			TRACE_ERR("%s [skipped] (192b not supported on sp slot "
				  "#%d)\n", title, req->sym.key_slot);
			return;
		}
		if (req->sym.key_size < biggest_size) {
			TRACE_ERR("%s [skipped] (sp slot #%d formerly "
				  "provisionned with a biggest key of %dbits)\n",
				  title, req->sym.key_slot, 8 * biggest_size);
			return;
		}
	}

	ctx = ccc_crypto_init(req);
	if (!ctx) {
		TRACE_ERR("%s: ccc_crypto_init fails\n", __func__);
		return;
	}

	err = ccc_crypto_run(ctx);
	if (err)
		TRACE_INFO("%s: ccc_crypto_run returns %d\n", __func__, err);

	if (0 == memcmp(req->sym.tag, tag_expected, tag_size))
		TRACE_ERR("%s authentication [passed]\n", title);
	else {
		TRACE_ERR("%s authentication [failed]\n", title);
		HEXDUMP(req->sym.tag, size);
		AES_LOOP_ON_FAILURE();
	}

	if (!memcmp(output, expected, size))
		TRACE_ERR("%s [passed]\n", title);
	else {
		TRACE_ERR("%s [failed]\n", title);
		HEXDUMP(output, size);
		AES_LOOP_ON_FAILURE();
	}
}

void perform_aes_ae_test(struct ccc_crypto_req *req, const char *title,
			 const char *expected, unsigned int size)
{
	void *ctx;
	int err;

	DUMP_ADDR(&req->scatter);

	if (is_sp_key_slot(req->sym.key_slot)) {
		unsigned int biggest_size =
			aes_biggest_requested_size[req->sym.key_slot];

		if (req->sym.key_size == AES_KEYSIZE_192) {
			TRACE_ERR("%s [skipped] (192b not supported on sp slot "
				  "#%d)\n", title, req->sym.key_slot);
			return;
		}
		if (req->sym.key_size < biggest_size) {
			TRACE_ERR("%s [skipped] (sp slot #%d formerly "
				  "provisionned with a biggest key of %dbits)\n",
				  title, req->sym.key_slot, 8 * biggest_size);
			return;
		}
	}

	ctx = ccc_crypto_init(req);
	if (!ctx) {
		TRACE_ERR("%s: ccc_crypto_init fails\n", __func__);
		return;
	}

	err = ccc_crypto_run(ctx);
	if (err)
		TRACE_INFO("%s: ccc_crypto_run returns %d\n", __func__, err);

	if (0 == memcmp(req->sym.tag, expected, size))
		TRACE_ERR("%s [passed]\n", title);
	else {
		TRACE_ERR("%s [failed]\n", title);
		HEXDUMP(req->sym.tag, size);
		AES_LOOP_ON_FAILURE();
	}
}

void set_key_size(struct symmetric_crypto *sym, unsigned int size)
{
	sym->key_size = size;
	if (is_sp_key_slot(sym->key_slot))
		if (size > aes_biggest_requested_size[sym->key_slot])
			aes_biggest_requested_size[sym->key_slot] = size;
}

void perform_aes_tests(struct ccc_crypto_req *req, unsigned char *src,
		       unsigned char *dst)
{
#define AES_128_ECB_ENCRYPT_PALINDROME_CIPHER "\x2E\x3A\x70\x07\xE0\x69\x6F\xED\x74\xCD\xF0\xE5\xD2\xE4\x65\x1D"
#define AES_128_ECB_ENCRYPT_PALINDROME "AES-128-ECB palindrome encryption"
#define AES_128_ECB_DECRYPT_PALINDROME_CLEAR "\xDA\x0D\x9F\x65\xE8\x00\x84\xCD\xC5\x5D\x4E\xCA\x5B\x6A\x8A\x3A"
#define AES_128_ECB_DECRYPT_PALINDROME "AES-128-ECB palindrome decryption"
#define AES_128_ECB_ENCRYPT_CIPHER "\xe9\xea\x38\x84\xa1\x4a\x82\x5e\x02\x71\x2a\xba\x45\xea\xc0\x79"
#define AES_128_ECB_ENCRYPT "AES-128-ECB encryption"
#define AES_128_ECB_DECRYPT_CLEAR "\x7f\xcd\x5d\x98\x90\x2d\x43\x3c\xd3\x3c\x0d\x47\x92\xff\x86\x95"
#define AES_128_ECB_DECRYPT "AES-128-ECB decryption"
#define AES_192_ECB_ENCRYPT_CIPHER "\x98\xa9\xb9\x8f\x4a\xe5\xd0\x3e\x6d\x92\xe7\xe8\xc8\x69\x42\x6a"
#define AES_192_ECB_ENCRYPT "AES-192-ECB encryption"
#define AES_256_ECB_ENCRYPT_CIPHER "\xe1\x16\x90\xfc\xec\xab\x72\xf5\xaa\xf1\x9a\x7a\x25\xc7\xb8\xfe"
#define AES_256_ECB_ENCRYPT "AES-256-ECB encryption"
#define AES_128_CBC_ENCRYPT_CIPHER "\x6a\xb5\xf9\x6e\xdd\xaf\x30\x01\x67\xf8\xcf\xd2\x8e\xd6\xd2\xbc"
#define AES_128_CBC_ENCRYPT "AES-128-CBC encryption"

#define AES_128_ECB_ENCRYPT_CIPHER_KP "\x5e\xa5\x58\x4b\x88\xb4\x6e\xc2\xce\x21\x47\x21\x31\x61\xdb\xb3"
#define AES_128_ECB_ENCRYPT_KP "AES-128-ECB encryption with key provisioning"
#define AES_192_ECB_ENCRYPT_CIPHER_KP "\x6b\x46\xea\xa2\x4d\xbb\xef\x94\xe2\xcc\x94\xff\x3c\xa7\xc1\x54"
#define AES_192_ECB_ENCRYPT_KP "AES-192-ECB encryption with key provisioning"
#define AES_256_ECB_ENCRYPT_CIPHER_KP "\xb0\xcf\xff\xfa\xd3\x2a\x5a\xc1\x6f\xc6\x69\x23\xdc\x0b\x02\xa8"
#define AES_256_ECB_ENCRYPT_KP "AES-256-ECB encryption with key provisioning"

	char *expected;
	set_key_size(&req->sym, AES_KEYSIZE_128);
	req->sym.dst = dst;

#ifndef NO_AES_ECB_TESTS
	req->sym.alg = AES_ECB_ALG;

#ifdef AES_128_ECB_ENCRYPT_PALINDROME
	strncpy((char *)in, PALINDROME,	strlen(PALINDROME) + sizeof('\0'));
	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, src, dst, strlen((const char *)in));
	req->sym.direction = AES_ENCRYPT;
	if (!req->sym.key)
		perform_aes_test(req,
				 AES_128_ECB_ENCRYPT_PALINDROME,
				 AES_128_ECB_ENCRYPT_PALINDROME_CIPHER);
#endif

#ifdef AES_128_ECB_DECRYPT_PALINDROME
	strncpy((char *)in, PALINDROME, strlen(PALINDROME) + sizeof('\0'));
	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, src, dst, strlen((const char *)in));
	req->sym.direction = AES_DECRYPT;
	if (!req->sym.key)
		perform_aes_test(req,
				 AES_128_ECB_DECRYPT_PALINDROME,
				 AES_128_ECB_DECRYPT_PALINDROME_CLEAR);
#endif

#ifdef AES_128_ECB_ENCRYPT
	strncpy((char *)in, DEFAULT_STRING,
		strlen(DEFAULT_STRING) + sizeof('\0'));
	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, src, dst, strlen((const char *)in));
	req->sym.direction = AES_ENCRYPT;
	if (!req->sym.key)
		perform_aes_test(req,
				 AES_128_ECB_ENCRYPT,
				 AES_128_ECB_ENCRYPT_CIPHER);
	else
		perform_aes_test(req,
				 AES_128_ECB_ENCRYPT,
				 AES_128_ECB_ENCRYPT_CIPHER_KP);
#endif

#ifdef AES_128_ECB_DECRYPT
	strncpy((char *)in, DEFAULT_STRING,
		strlen(DEFAULT_STRING) + sizeof('\0'));
	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, src, dst, strlen((const char *)in));
	req->sym.direction = AES_DECRYPT;
	if (!req->sym.key)
		perform_aes_test(req,
				 AES_128_ECB_DECRYPT,
				 AES_128_ECB_DECRYPT_CLEAR);
#endif

#ifdef AES_192_ECB_ENCRYPT
	strncpy((char *)in, DEFAULT_STRING,
		strlen(DEFAULT_STRING) + sizeof('\0'));
	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, src, dst, strlen((const char *)in));
	set_key_size(&req->sym, AES_KEYSIZE_192);
	req->sym.direction = AES_ENCRYPT;
	if (!req->sym.key)
		perform_aes_test(req,
				 AES_192_ECB_ENCRYPT,
				 AES_192_ECB_ENCRYPT_CIPHER);
	else
		perform_aes_test(req,
				 AES_192_ECB_ENCRYPT,
				 AES_192_ECB_ENCRYPT_CIPHER_KP);
#endif

#if 0
	strncpy((char *)in, DEFAULT_STRING,
		strlen(DEFAULT_STRING) + sizeof('\0'));
	set_key_size(&req->sym, AES_KEYSIZE_128);
	req->sym.direction = AES_ENCRYPT;
	if (!req->sym.key)
		perform_aes_test(req,
				 AES_128_ECB_ENCRYPT,
				 AES_128_ECB_ENCRYPT_CIPHER);
#endif

#ifdef AES_256_ECB_ENCRYPT
	strncpy((char *)in, DEFAULT_STRING,
		strlen(DEFAULT_STRING) + sizeof('\0'));
	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, src, dst, strlen((const char *)in));
	set_key_size(&req->sym, AES_KEYSIZE_256);
	if (req->sym.key)
		expected = AES_256_ECB_ENCRYPT_CIPHER_KP;
	else
		expected = is_sp_key_slot(req->sym.key_slot) ?
			AES_128_ECB_ENCRYPT_CIPHER : AES_256_ECB_ENCRYPT_CIPHER;
	perform_aes_test(req,
			 AES_256_ECB_ENCRYPT,
			 expected);
#endif
#endif /* !NO_AES_ECB_TESTS */

#ifndef NO_AES_CBC_TESTS
	req->sym.alg = AES_CBC_ALG;

#ifdef AES_128_CBC_ENCRYPT
	strncpy((char *)in, DEFAULT_STRING,
		strlen(DEFAULT_STRING) + sizeof('\0'));
	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, src, dst, strlen((const char *)in));
	set_key_size(&req->sym, AES_KEYSIZE_128);
	req->sym.direction = AES_ENCRYPT;
	memset((unsigned char *)iv, '\0', AES_IV_SIZE);
	iv[15] = 1;
	req->sym.iv = iv;
	if (!req->sym.key)
		perform_aes_test(req,
				 AES_128_CBC_ENCRYPT,
				 AES_128_CBC_ENCRYPT_CIPHER);
#endif
#endif /* !NO_AES_CBC_TESTS */

#ifndef NO_AES_GCM_TESTS
	req->sym.alg = AES_GCM_ALG;

#define AES_128_GCM_ENCRYPT_CIPHER "\x57\xe7\xb5\xee\x0d\xd7\xcd\xeb\xd3\x5b\xa7\xda\x03\xd7\x8a\x0b"
#define AES_128_GCM_ENCRYPT_SIZE 16
#define AES_128_GCM_ENCRYPT "AES-128-GCM encryption"

#ifdef AES_128_GCM_ENCRYPT
	strncpy((char *)in, DEFAULT_STRING,
		strlen(DEFAULT_STRING) + sizeof('\0'));
	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, src, dst, AES_128_GCM_ENCRYPT_SIZE);
	set_key_size(&req->sym, AES_KEYSIZE_128);
	req->sym.direction = AES_ENCRYPT;
	memset((unsigned char *)iv, '\0', AES_IV_SIZE);
	req->sym.iv = iv;
	memset((unsigned char *)tag, '\0', AES_GCM_TAG_SIZE);
	req->sym.tag = tag;
	req->sym.payload = in;
	req->sym.payload_size = AES_128_GCM_ENCRYPT_SIZE;
	req->sym.header = NULL;
	req->sym.header_size = 0;

	if (!req->sym.key)
		perform_aes_test(req,
				 AES_128_GCM_ENCRYPT,
				 AES_128_GCM_ENCRYPT_CIPHER);
#endif

#define AES_128_GCM_TAG "\x84\x45\xa7\x35\xcc\xc6\x75\xf8\xe9\x59\xe9\xcf\xc5\xd4\xae\x55"
#define AES_128_GCM_SIZE 16
#define AES_128_GCM "AES-128-GCM authenticated encryption"

#ifdef AES_128_GCM
	strncpy((char *)in, DEFAULT_STRING,
		strlen(DEFAULT_STRING) + sizeof('\0'));
	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, src, dst, AES_128_GCM_SIZE);
	set_key_size(&req->sym, AES_KEYSIZE_128);
	req->sym.direction = AES_ENCRYPT;
	memset((unsigned char *)iv, '\0', AES_IV_SIZE);
	req->sym.iv = iv;
	memset((unsigned char *)tag, '\0', AES_GCM_TAG_SIZE);
	req->sym.tag = tag;
#ifndef GET_A_CRYPTO_INIT_FAILURE
	req->sym.payload = in;
	req->sym.payload_size = AES_128_GCM_SIZE;
	req->sym.header = NULL;
	req->sym.header_size = 0;
#endif
	if (!req->sym.key)
		perform_aes_ae_test(req,
				    AES_128_GCM,
				    AES_128_GCM_TAG,
				    AES_GCM_TAG_SIZE);
#endif

#define AES_256_GCM_TAG "\x9a\xc8\x2f\x1d\x20\x01\x05\x17\x27\x3d\xa0\xb0\xc8\x96\xe9\x6b"
#define AES_256_GCM_TAG_KP "\x0b\xbb\x60\xef\x65\x49\xef\x21\xff\x1e\x15\x89\x99\x1b\x00\x83"
#define AES_256_GCM_SIZE 16
#define AES_256_GCM "AES-256-GCM authenticated encryption"

#ifdef AES_256_GCM
	strncpy((char *)in, DEFAULT_STRING,
		strlen(DEFAULT_STRING) + sizeof('\0'));
	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, src, dst, AES_128_GCM_SIZE);
	set_key_size(&req->sym, AES_KEYSIZE_256);
	req->sym.direction = AES_ENCRYPT;
	memset((unsigned char *)iv, '\0', AES_IV_SIZE);
	req->sym.iv = iv;
	memset((unsigned char *)tag, '\0', AES_GCM_TAG_SIZE);
	req->sym.tag = tag;
	req->sym.payload = in;
	req->sym.payload_size = AES_128_GCM_SIZE;
	req->sym.header = NULL;
	req->sym.header_size = 0;
	if (req->sym.key)
		expected = AES_256_GCM_TAG_KP;
	else
		expected = is_sp_key_slot(req->sym.key_slot) ?
			AES_128_GCM_TAG : AES_256_GCM_TAG;
	perform_aes_ae_test(req,
			    AES_256_GCM,
			    expected,
			    AES_GCM_TAG_SIZE);
#endif

#define AES_128_GCM_WITH_HEADER_KEY "\x00\xd6\x36\x07\x6e\x19\x68\x44\x75\x12\x9c\x22\x59\x6b\x23\x00"
#define AES_128_GCM_WITH_HEADER_KEY_SIZE 16
#define AES_128_GCM_WITH_HEADER_IV "\xdc\x8c\x6a\x6e\x63\x8a\x0e\xc6\x6e\xec\x71\x6b\x00\x00\x00\x01"
#define AES_128_GCM_WITH_HEADER_CLEAR "\xd6\x11\xb1\x05\x92\xd6\x1f\xa0\x63\xe3\xdd\xaa\xfd\xfd\xbf\xfd\xd3\xf6\x05\x42\x0f\x6e\x86\x85\x81\x22\xa7\xda\x8e\xca\xdb\x64"
#define AES_128_GCM_WITH_HEADER_CLEAR_SIZE 32
#define AES_128_GCM_WITH_HEADER_PAYLOAD_OFFSET 16
#define AES_128_GCM_WITH_HEADER_TAG "\xac\xc6\x77\x4b\x4d\x00\x75\x77\xe3\x2c\x8d\x83\x18\xe1\x4a\xd2"
#define AES_128_GCM_WITH_HEADER "AES-128-GCM authenticated encryption with header"

#ifdef AES_128_GCM_WITH_HEADER
	memcpy((char *)aes_ae_key, AES_128_GCM_WITH_HEADER_KEY,
	       AES_128_GCM_WITH_HEADER_KEY_SIZE);
	memcpy((char *)in, AES_128_GCM_WITH_HEADER_CLEAR,
	       AES_128_GCM_WITH_HEADER_CLEAR_SIZE);
	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, src, dst,
			   AES_128_GCM_WITH_HEADER_CLEAR_SIZE);

	set_key_size(&req->sym, AES_KEYSIZE_128);
	unsigned char *key_backup = req->sym.key;

	req->sym.key = aes_ae_key;
	req->sym.direction = AES_ENCRYPT;
	memcpy((unsigned char *)iv, AES_128_GCM_WITH_HEADER_IV, AES_IV_SIZE);
	req->sym.iv = iv;
	memset((unsigned char *)tag, '\0', AES_GCM_TAG_SIZE);
	req->sym.tag = tag;
	req->sym.payload = in + AES_128_GCM_WITH_HEADER_PAYLOAD_OFFSET;
	req->sym.payload_size = AES_128_GCM_WITH_HEADER_CLEAR_SIZE -
		AES_128_GCM_WITH_HEADER_PAYLOAD_OFFSET;
	req->sym.header = in;
	req->sym.header_size = AES_128_GCM_WITH_HEADER_PAYLOAD_OFFSET;
	perform_aes_ae_test(req,
			    AES_128_GCM_WITH_HEADER,
			    AES_128_GCM_WITH_HEADER_TAG,
			    AES_GCM_TAG_SIZE);
	req->sym.key = key_backup;
#endif

	/* Clean-up */
	req->sym.payload = NULL;
	req->sym.header = NULL;
	req->sym.tag = NULL;

#endif /* !NO_AES_GCM_TESTS */
}

void perform_aes_then_hash_tests(struct ccc_crypto_req *req, unsigned char *src,
				 unsigned char *dst)
{
#ifndef NO_AES_ECB_TESTS
#define AES_128_ECB_192KB_DATA_ENCRYPT_SHA256_DIGEST "\x54\x4c\x66\xc8\xa8\xdd\xb1\x12\xf3\x04\x66\x4b\x26\x23\xf3\x34\xb7\xd1\x2c\x72\xa8\x25\x1f\x84\x57\x1b\x54\x7c\x89\xd0\xc9\x64"
#define AES_128_ECB_192KB_DATA_ENCRYPT "AES-128-ECB 192KB data encryption"

	unsigned char *p, c = '\0';
	unsigned int i;

	req->sym.alg = AES_ECB_ALG;
	req->hash.alg = HASH_SHA_256_ALG;
	req->sym.direction = AES_ENCRYPT;
	req->sym.dst = dst;

#ifdef AES_128_ECB_192KB_DATA_ENCRYPT
	/* 192KB > 3 * MAX_CHUNK_SIZE where 3 < NR_CHUNKS */
	p = in;
	for (i = 0; i < (64 * 1024 * 3); i++)
		*(p++) = c++;

	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, src, dst, 64 * 1024 * 3);

	perform_hash_test(req,
			  AES_128_ECB_192KB_DATA_ENCRYPT,
			  AES_128_ECB_192KB_DATA_ENCRYPT_SHA256_DIGEST,
			  SHA256_SIZE);
#endif

#define AES_128_ECB_256KB_DATA_ENCRYPT_SHA256_DIGEST "\xfe\xc0\xb5\xd5\x1a\x56\xbd\x69\x37\xf5\xfb\xaa\x78\x09\x7b\x2a\x04\x1c\xea\x92\xd1\xf2\xe3\xc0\x48\x32\xb8\x12\x9d\x62\x5a\xc5"
#define AES_128_ECB_256KB_DATA_ENCRYPT "AES-128-ECB 256KB data encryption"

#ifdef AES_128_ECB_256KB_DATA_ENCRYPT
	/* 256KB > 4 * MAX_CHUNK_SIZE where 4 < NR_CHUNKS */
	p = in;
	for (i = 0; i < (64 * 1024 * 4); i++)
		*(p++) = c++;

	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, src, dst, 64 * 1024 * 4);

	perform_hash_test(req,
			  AES_128_ECB_256KB_DATA_ENCRYPT,
			  AES_128_ECB_256KB_DATA_ENCRYPT_SHA256_DIGEST,
			  SHA256_SIZE);
#endif
#endif /* !NO_AES_ECB_TESTS */
}

void perform_aes_ae_big_data_tests(struct ccc_crypto_req *req,
				   unsigned char *src, unsigned char *dst)
{
#ifndef NO_AES_GCM_TESTS
	req->sym.alg = AES_GCM_ALG;
	set_key_size(&req->sym, AES_KEYSIZE_128);
	req->sym.direction = AES_ENCRYPT;
	req->sym.dst = dst;

#define AES_128_GCM_256KB_DATA_ENCRYPT_TAG "\x4f\xea\xe6\x20\xbe\x0e\x58\xf6\x3a\x0f\xa6\x1f\x99\x06\x50\x3d"
#define AES_128_GCM_256KB_DATA_ENCRYPT "AES-128-GCM 256KB data authenticated encryption"

#ifdef AES_128_GCM_256KB_DATA_ENCRYPT
	memset((unsigned char *)iv, '\0', AES_IV_SIZE);
	req->sym.iv = iv;
	memset((unsigned char *)tag, '\0', AES_GCM_TAG_SIZE);
	req->sym.tag = tag;
	req->sym.payload = in;
	req->sym.payload_size = 64 * 1024 * 4;
	req->sym.header = NULL;
	req->sym.header_size = 0;

	unsigned char *p, c = '\0';
	unsigned int i;

	/* 256KB > 4 * MAX_CHUNK_SIZE where 4 < NR_CHUNKS */
	p = in;
	for (i = 0; i < (64 * 1024 * 4); i++)
		*(p++) = c++;

	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, src, dst, 64 * 1024 * 4);

	perform_aes_ae_test(req,
			    AES_128_GCM_256KB_DATA_ENCRYPT,
			    AES_128_GCM_256KB_DATA_ENCRYPT_TAG,
			    AES_GCM_TAG_SIZE);
#endif
	/* Clean-up */
	req->sym.payload = NULL;
	req->sym.header = NULL;
	req->sym.tag = NULL;

#endif /* !NO_AES_GCM_TESTS */
}

void perform_pka_test(struct ccc_crypto_req *req, const char *title,
		      const char *expected)
{
	void *ctx;
	int err;
	struct entry *entry;

	DUMP_ADDR(&req->scatter);

	ctx = ccc_crypto_init(req);
	if (!ctx) {
		TRACE_ERR("%s: ccc_crypto_init fails %x\n", __func__, ctx);
		return;
	}

	entry = &req->scatter.entries[0];	/* Consider 1st entry only */
	memset(entry->dst, '\0', entry->size);

	err = ccc_crypto_run(ctx);
	if (err)
		TRACE_INFO("%s: ccc_crypto_run returns %d\n", __func__, err);

	if (expected) {
		if (memcmp(entry->dst, expected, entry->size) == 0) {
			TRACE_ERR("%s [passed]\n", title);
		} else {
			TRACE_ERR("%s [failed]\n", title);
			HEXDUMP(entry->dst, entry->size);
			RSA_LOOP_ON_FAILURE();
		}
	} else {
		if (err)
			TRACE_ERR("%s [failed] with error %d\n", title, err);
		else
			TRACE_ERR("%s [passed]\n", title);
	}
}

void perform_rsa_tests(struct ccc_crypto_req *req)
{
	req->asym.alg = PK_RSA_ALG;

/* #define RSA_32_MODULAR_EXPONENTIATION "RSA 32 bits modular exponentiation" */
#define RSA_32_SIZE (32 / 8)
#if 1
#define RSA_32_BASE "\x0\x0\x0\x05"
#define RSA_32_MODULUS "\x0\x0\x0\x13"
#define RSA_32_EXPONENT "\x0\x0\x0\x0d"
#define RSA_32_MODULAR_EXPONENTIATION_RESULT "\x0\x0\x0\x11"
#else
#define RSA_32_BASE "\x80\x0\x0\x05"
#define RSA_32_MODULUS "\xf0\x0\x0\x05"
#define RSA_32_EXPONENT "\x80\x0\x0\x03"
#define RSA_32_MODULAR_EXPONENTIATION_RESULT "\x0\x0\x0\x03"
#endif

#ifdef RSA_32_MODULAR_EXPONENTIATION
	memcpy((char *)in, RSA_32_BASE, RSA_32_SIZE);
	memcpy((char *)mod, RSA_32_MODULUS, RSA_32_SIZE);
	req->asym.modulus_size = RSA_32_SIZE;
	req->asym.modulus = mod;
	memcpy((char *)exp, RSA_32_EXPONENT, RSA_32_SIZE);
	req->asym.exponent_size = RSA_32_SIZE;
	req->asym.exponent = exp;

	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, in, out, RSA_32_SIZE);

	perform_pka_test(req,
			 RSA_32_MODULAR_EXPONENTIATION,
			 RSA_32_MODULAR_EXPONENTIATION_RESULT);
#endif

#define RSA_64_MODULAR_EXPONENTIATION "RSA 64 bits modular exponentiation"
#define RSA_64_SIZE (64 / 8)
#if 1
#define RSA_64_BASE "\xff\xff\xff\x05\xff\xff\xff\x05"
#define RSA_64_MODULUS "\xff\xff\xff\x05\xff\xff\xff\x13"
#define RSA_64_EXPONENT "\x0\x0\x0\x3"
#define RSA_64_EXPONENT_SIZE 4
#define RSA_64_MODULAR_EXPONENTIATION_RESULT "\xff\xff\xff\x05\xff\xff\xf4\x5b"
#else
#define RSA_64_BASE "\x80\x0\x0\x0\x0\x0\x0\x05"
#define RSA_64_MODULUS "\x80\x0\x0\x0\x0\x0\x0\x13"
#define RSA_64_EXPONENT "\x80\x0\x0\x0\x0\x0\x0\x0d"
#define RSA_64_EXPONENT_SIZE 8
#define RSA_64_MODULAR_EXPONENTIATION_RESULT "\x34\xe6\xb6\x34\xcc\x34\x19\xb0"
#endif

#ifdef RSA_64_MODULAR_EXPONENTIATION
	memcpy((char *)in, RSA_64_BASE, RSA_64_SIZE);
	memcpy((char *)mod, RSA_64_MODULUS, RSA_64_SIZE);
	req->asym.modulus_size = RSA_64_SIZE;
	req->asym.modulus = mod;
	memcpy((char *)exp, RSA_64_EXPONENT, RSA_64_EXPONENT_SIZE);
	req->asym.exponent_size = RSA_64_EXPONENT_SIZE;
	req->asym.exponent = exp;

	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, in, out, RSA_64_SIZE);

	perform_pka_test(req,
			 RSA_64_MODULAR_EXPONENTIATION,
			 RSA_64_MODULAR_EXPONENTIATION_RESULT);
#endif

#define RSA_64_UNALIGNED_1_MODULAR_EXPONENTIATION "RSA 1 byte word-unaligned exponent 64 bits modular exponentiation"
#define RSA_64_SIZE (64 / 8)
#define RSA_64_BASE "\xff\xff\xff\x05\xff\xff\xff\x05"
#define RSA_64_MODULUS "\xff\xff\xff\x05\xff\xff\xff\x13"
#define RSA_64_UNALIGNED_1_EXPONENT "\x3"
#define RSA_64_UNALIGNED_1_EXPONENT_SIZE 1
#define RSA_64_MODULAR_EXPONENTIATION_RESULT "\xff\xff\xff\x05\xff\xff\xf4\x5b"

#ifdef RSA_64_MODULAR_EXPONENTIATION
	memcpy((char *)in, RSA_64_BASE, RSA_64_SIZE);
	memcpy((char *)mod, RSA_64_MODULUS, RSA_64_SIZE);
	req->asym.modulus_size = RSA_64_SIZE;
	req->asym.modulus = mod;
	memcpy((char *)exp, RSA_64_UNALIGNED_1_EXPONENT, RSA_64_UNALIGNED_1_EXPONENT_SIZE);
	req->asym.exponent_size = RSA_64_UNALIGNED_1_EXPONENT_SIZE;
	req->asym.exponent = exp;

	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, in, out, RSA_64_SIZE);

	perform_pka_test(req,
			 RSA_64_UNALIGNED_1_MODULAR_EXPONENTIATION,
			 RSA_64_MODULAR_EXPONENTIATION_RESULT);
#endif

#define RSA_64_UNALIGNED_2_MODULAR_EXPONENTIATION "RSA 2 bytes word-unaligned exponent 64 bits modular exponentiation"
#define RSA_64_SIZE (64 / 8)
#define RSA_64_BASE "\xff\xff\xff\x05\xff\xff\xff\x05"
#define RSA_64_MODULUS "\xff\xff\xff\x05\xff\xff\xff\x13"
#define RSA_64_UNALIGNED_2_EXPONENT "\x0\x3"
#define RSA_64_UNALIGNED_2_EXPONENT_SIZE 2
#define RSA_64_MODULAR_EXPONENTIATION_RESULT "\xff\xff\xff\x05\xff\xff\xf4\x5b"

#ifdef RSA_64_MODULAR_EXPONENTIATION
	memcpy((char *)in, RSA_64_BASE, RSA_64_SIZE);
	memcpy((char *)mod, RSA_64_MODULUS, RSA_64_SIZE);
	req->asym.modulus_size = RSA_64_SIZE;
	req->asym.modulus = mod;
	memcpy((char *)exp, RSA_64_UNALIGNED_2_EXPONENT, RSA_64_UNALIGNED_2_EXPONENT_SIZE);
	req->asym.exponent_size = RSA_64_UNALIGNED_2_EXPONENT_SIZE;
	req->asym.exponent = exp;

	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, in, out, RSA_64_SIZE);

	perform_pka_test(req,
			 RSA_64_UNALIGNED_2_MODULAR_EXPONENTIATION,
			 RSA_64_MODULAR_EXPONENTIATION_RESULT);
#endif

#define RSA_64_UNALIGNED_3_MODULAR_EXPONENTIATION "RSA 3 bytes word-unaligned exponent 64 bits modular exponentiation"
#define RSA_64_SIZE (64 / 8)
#define RSA_64_BASE "\xff\xff\xff\x05\xff\xff\xff\x05"
#define RSA_64_MODULUS "\xff\xff\xff\x05\xff\xff\xff\x13"
#define RSA_64_UNALIGNED_3_EXPONENT "\x0\x0\x3"
#define RSA_64_UNALIGNED_3_EXPONENT_SIZE 3
#define RSA_64_MODULAR_EXPONENTIATION_RESULT "\xff\xff\xff\x05\xff\xff\xf4\x5b"

#ifdef RSA_64_MODULAR_EXPONENTIATION
	memcpy((char *)in, RSA_64_BASE, RSA_64_SIZE);
	memcpy((char *)mod, RSA_64_MODULUS, RSA_64_SIZE);
	req->asym.modulus_size = RSA_64_SIZE;
	req->asym.modulus = mod;
	memcpy((char *)exp, RSA_64_UNALIGNED_3_EXPONENT, RSA_64_UNALIGNED_3_EXPONENT_SIZE);
	req->asym.exponent_size = RSA_64_UNALIGNED_3_EXPONENT_SIZE;
	req->asym.exponent = exp;

	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, in, out, RSA_64_SIZE);

	perform_pka_test(req,
			 RSA_64_UNALIGNED_3_MODULAR_EXPONENTIATION,
			 RSA_64_MODULAR_EXPONENTIATION_RESULT);
#endif

#define RSA_64_MODULAR_EXPONENTIATION_4_CHUNKS "RSA 64 bits modular exponentiation of 4 chunks"
#define RSA_64_SIZE (64 / 8)
#define RSA_64_BASE_4_CHUNKS "\xff\xff\xff\x05\xff\xff\xff\x05\xff\xff\xff\x05\xff\xff\xff\x05\xff\xff\xff\x05\xff\xff\xff\x05\xff\xff\xff\x05\xff\xff\xff\x05"
#define RSA_64_MODULUS "\xff\xff\xff\x05\xff\xff\xff\x13"
#define RSA_64_EXPONENT "\x0\x0\x0\x3"
#define RSA_64_EXPONENT_SIZE 4
#define RSA_64_MODULAR_EXPONENTIATION_4_CHUNKS_RESULT "\xff\xff\xff\x05\xff\xff\xf4\x5b\xff\xff\xff\x05\xff\xff\xf4\x5b\xff\xff\xff\x05\xff\xff\xf4\x5b\xff\xff\xff\x05\xff\xff\xf4\x5b"

#ifdef RSA_64_MODULAR_EXPONENTIATION
	memcpy((char *)in, RSA_64_BASE_4_CHUNKS, RSA_64_SIZE * 4);
	memcpy((char *)mod, RSA_64_MODULUS, RSA_64_SIZE);
	req->asym.modulus_size = RSA_64_SIZE;
	req->asym.modulus = mod;
	memcpy((char *)exp, RSA_64_EXPONENT, RSA_64_EXPONENT_SIZE);
	req->asym.exponent_size = RSA_64_EXPONENT_SIZE;
	req->asym.exponent = exp;

	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, in, out, RSA_64_SIZE * 4);

	perform_pka_test(req,
			 RSA_64_MODULAR_EXPONENTIATION_4_CHUNKS,
			 RSA_64_MODULAR_EXPONENTIATION_4_CHUNKS_RESULT);
#endif

#define RSA_160_MODULAR_EXPONENTIATION "RSA 160 bits modular exponentiation"
#define RSA_160_SIZE (160 / 8)
#define RSA_160_BASE "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x05"
#define RSA_160_MODULUS "\xc3\x56\xf1\x39\x36\x79\x82\xc4\x7a\xce\x8f\xad\xae\xce\xbf\x3d\x35\x6f\xd1\xe1"
#define RSA_160_EXPONENT "\x0\x0\x0\x0d"
#define RSA_160_EXPONENT_SIZE 4
#define RSA_160_MODULAR_EXPONENTIATION_RESULT "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x48\xc2\x73\x95"
#ifdef RSA_160_MODULAR_EXPONENTIATION
	memcpy((char *)in, RSA_160_BASE, RSA_160_SIZE);
	memcpy((char *)mod, RSA_160_MODULUS, RSA_160_SIZE);
	req->asym.modulus_size = RSA_160_SIZE;
	req->asym.modulus = mod;
	memcpy((char *)exp, RSA_160_EXPONENT, RSA_160_EXPONENT_SIZE);
	req->asym.exponent_size = RSA_160_EXPONENT_SIZE;
	req->asym.exponent = exp;

	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, in, out, RSA_160_SIZE);

	perform_pka_test(req,
			 RSA_160_MODULAR_EXPONENTIATION,
			 RSA_160_MODULAR_EXPONENTIATION_RESULT);
#endif

#define RSA_512_MODULAR_EXPONENTIATION "RSA 512 bits modular exponentiation"
#define RSA_512_SIZE (512 / 8)
#define RSA_512_BASE "\x40\xA0\x3A\x3D\xA6\x87\x69\x1E\x6C\x9C\xDF\x06\x41\xD2\x89\x8C\xA9\xC9\x25\x01\xB3\x43\x74\x1B\x09\xEF\x44\x6A\x60\x24\x30\x9D\x01\x0F\xA1\x80\x7E\xD9\xC1\x90\x67\x0B\xA5\x91\x51\x1D\x6A\xA6\xAA\x66\x65\xA5\x5E\x15\x2F\x82\xD6\x96\x79\x04\x80\x77\x4A\x36"

#define RSA_512_MODULUS "\xC7\x21\xCC\x07\x11\x11\xE0\x27\xBC\x5D\xDF\x01\xE0\x91\x85\x74\x18\x8B\x64\x5F\x1F\xF6\xF0\x3E\xC8\x41\x38\x5C\xD3\xEC\xE8\x4B\x13\xEE\x13\x69\x39\xFD\x5C\xC9\x7A\xCA\x5A\x5E\xC7\x9F\x1E\x07\xC7\x16\x83\x8C\xA2\x26\x0F\x1C\x43\xE1\x52\xBF\x69\x13\x50\x0b"
#define RSA_512_EXPONENT "\x0\x0\x0\x03"
#define RSA_512_EXPONENT_SIZE 4
#define RSA_512_MODULAR_EXPONENTIATION_RESULT "\xC0\x9A\x98\x1E\x87\xE3\x31\xA2\x1F\x36\x44\x4F\x33\x5F\x1F\xE8\x55\x1B\xA8\xEF\x77\x09\x48\x8E\x9C\x4A\xA6\xC6\x4A\x73\x7E\xC0\xA9\x3F\xB1\x52\x33\xE3\xE1\x22\x3B\x18\x3D\xD1\x76\x57\x4C\x8B\xC4\xA8\xCD\xA6\xD4\x64\xC9\x2F\x40\x0A\x4F\x94\x46\x0A\x1F\xF9"
#ifdef RSA_512_MODULAR_EXPONENTIATION
	memcpy((char *)in, RSA_512_BASE, RSA_512_SIZE);
	memcpy((char *)mod, RSA_512_MODULUS, RSA_512_SIZE);
	req->asym.modulus_size = RSA_512_SIZE;
	req->asym.modulus = mod;
	memcpy((char *)exp, RSA_512_EXPONENT, RSA_512_EXPONENT_SIZE);
	req->asym.exponent_size = RSA_512_EXPONENT_SIZE;
	req->asym.exponent = exp;

	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, in, out, RSA_512_SIZE);

	perform_pka_test(req,
			 RSA_512_MODULAR_EXPONENTIATION,
			 RSA_512_MODULAR_EXPONENTIATION_RESULT);
#endif

#define RSA_512_MODULAR_EXPONENTIATION_2_CHUNKS "RSA 512 bits modular exponentiation of 2 chunks"
#define RSA_512_SIZE (512 / 8)
#define RSA_512_BASE_2_CHUNKS "\x40\xA0\x3A\x3D\xA6\x87\x69\x1E\x6C\x9C\xDF\x06\x41\xD2\x89\x8C\xA9\xC9\x25\x01\xB3\x43\x74\x1B\x09\xEF\x44\x6A\x60\x24\x30\x9D\x01\x0F\xA1\x80\x7E\xD9\xC1\x90\x67\x0B\xA5\x91\x51\x1D\x6A\xA6\xAA\x66\x65\xA5\x5E\x15\x2F\x82\xD6\x96\x79\x04\x80\x77\x4A\x36\x40\xA0\x3A\x3D\xA6\x87\x69\x1E\x6C\x9C\xDF\x06\x41\xD2\x89\x8C\xA9\xC9\x25\x01\xB3\x43\x74\x1B\x09\xEF\x44\x6A\x60\x24\x30\x9D\x01\x0F\xA1\x80\x7E\xD9\xC1\x90\x67\x0B\xA5\x91\x51\x1D\x6A\xA6\xAA\x66\x65\xA5\x5E\x15\x2F\x82\xD6\x96\x79\x04\x80\x77\x4A\x36"

#define RSA_512_MODULUS "\xC7\x21\xCC\x07\x11\x11\xE0\x27\xBC\x5D\xDF\x01\xE0\x91\x85\x74\x18\x8B\x64\x5F\x1F\xF6\xF0\x3E\xC8\x41\x38\x5C\xD3\xEC\xE8\x4B\x13\xEE\x13\x69\x39\xFD\x5C\xC9\x7A\xCA\x5A\x5E\xC7\x9F\x1E\x07\xC7\x16\x83\x8C\xA2\x26\x0F\x1C\x43\xE1\x52\xBF\x69\x13\x50\x0b"
#define RSA_512_EXPONENT "\x0\x0\x0\x03"
#define RSA_512_EXPONENT_SIZE 4
#define RSA_512_MODULAR_EXPONENTIATION_2_CHUNKS_RESULT "\xC0\x9A\x98\x1E\x87\xE3\x31\xA2\x1F\x36\x44\x4F\x33\x5F\x1F\xE8\x55\x1B\xA8\xEF\x77\x09\x48\x8E\x9C\x4A\xA6\xC6\x4A\x73\x7E\xC0\xA9\x3F\xB1\x52\x33\xE3\xE1\x22\x3B\x18\x3D\xD1\x76\x57\x4C\x8B\xC4\xA8\xCD\xA6\xD4\x64\xC9\x2F\x40\x0A\x4F\x94\x46\x0A\x1F\xF9\xC0\x9A\x98\x1E\x87\xE3\x31\xA2\x1F\x36\x44\x4F\x33\x5F\x1F\xE8\x55\x1B\xA8\xEF\x77\x09\x48\x8E\x9C\x4A\xA6\xC6\x4A\x73\x7E\xC0\xA9\x3F\xB1\x52\x33\xE3\xE1\x22\x3B\x18\x3D\xD1\x76\x57\x4C\x8B\xC4\xA8\xCD\xA6\xD4\x64\xC9\x2F\x40\x0A\x4F\x94\x46\x0A\x1F\xF9"
#ifdef RSA_512_MODULAR_EXPONENTIATION_2_CHUNKS
	memcpy((char *)in, RSA_512_BASE_2_CHUNKS, RSA_512_SIZE * 2);
	memcpy((char *)mod, RSA_512_MODULUS, RSA_512_SIZE);
	req->asym.modulus_size = RSA_512_SIZE;
	req->asym.modulus = mod;
	memcpy((char *)exp, RSA_512_EXPONENT, RSA_512_EXPONENT_SIZE);
	req->asym.exponent_size = RSA_512_EXPONENT_SIZE;
	req->asym.exponent = exp;

	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, in, out, RSA_512_SIZE * 2);

	perform_pka_test(req,
			 RSA_512_MODULAR_EXPONENTIATION_2_CHUNKS,
			 RSA_512_MODULAR_EXPONENTIATION_2_CHUNKS_RESULT);
#endif
}

void perform_ecc_tests(struct ccc_crypto_req *req)
{
	struct curve *curve = &req->asym.curve;

	req->asym.alg = PK_ECC_ALG;

#define ECC_P112_SCALAR_MULTIPLICATION "ECC scalar multiplication of 112 bits curve"
#define ECC_P112_SIZE (sizeof(int) * SIZE_IN_WORDS(112))
#define ECC_P112_COORD "\x00\x00\x09\x48\x72\x39\x99\x5a\x5e\xe7\x6b\x55\xf9\xc2\xf0\x98\x00\x00\xa8\x9c\xe5\xaf\x87\x24\xc0\xa2\x3e\x0e\x0f\xf7\x75\x00"
#define ECC_P112_MODULUS "\x00\x00\xdb\x7c\x2a\xbf\x62\xe3\x5e\x66\x80\x76\xbe\xad\x20\x8b"
#define ECC_P112_A "\x00\x00\xdb\x7c\x2a\xbf\x62\xe3\x5e\x66\x80\x76\xbe\xad\x20\x88"
#define ECC_P112_A_SIGN 0x0
#define ECC_P112_SCALAR "\x00\x00\x12\x4c\x2b\x2d\x8c\x40\x70\x74\xbc\x51\xfb\x65\xf5\xe1"
#define ECC_P112_SCALAR_SIZE 16
#define ECC_P112_SCALAR_MULTIPLICATION_RESULT "\x00\x00\x7A\x52\x6B\x0A\xE2\xB0\xDB\x4F\x81\x13\x9B\x5B\xF0\x9B\x00\x00\xC7\xA1\xE7\xB0\xB5\x89\x2B\x07\xD2\x11\x66\x42\xF3\x1F"

#ifdef ECC_P112_SCALAR_MULTIPLICATION
	memcpy((char *)in, ECC_P112_COORD, ECC_P112_SIZE * 2);
	memcpy((char *)mod_ecc, ECC_P112_MODULUS, ECC_P112_SIZE);
	req->asym.modulus_size = ECC_P112_SIZE;
	req->asym.modulus = mod_ecc;
	curve->a_sign = ECC_P112_A_SIGN;
	memcpy((char *)a_ec, ECC_P112_A, ECC_P112_SIZE);
	curve->a = a_ec;
	memcpy((char *)k_ecc, ECC_P112_SCALAR, ECC_P112_SCALAR_SIZE);
	req->asym.k_size = ECC_P112_SCALAR_SIZE;
	req->asym.k = k_ecc;

	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, in, out, ECC_P112_SIZE * 2);

	perform_pka_test(req,
			 ECC_P112_SCALAR_MULTIPLICATION,
			 ECC_P112_SCALAR_MULTIPLICATION_RESULT);
#endif

#define ECC_P192_SCALAR_MULTIPLICATION "ECC scalar multiplication of NIST P-192 (k = 1) curve"
#define ECC_P192_SIZE (sizeof(int) * SIZE_IN_WORDS(192))
#define ECC_P192_COORD "\x18\x8d\xa8\x0e\xb0\x30\x90\xf6\x7c\xbf\x20\xeb\x43\xa1\x88\x00\xf4\xff\x0a\xfd\x82\xff\x10\x12\x07\x19\x2b\x95\xff\xc8\xda\x78\x63\x10\x11\xed\x6b\x24\xcd\xd5\x73\xf9\x77\xa1\x1e\x79\x48\x11"
#define ECC_P192_MODULUS "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xfe\xff\xff\xff\xff\xff\xff\xff\xff"
#define ECC_P192_A "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xfc"
#define ECC_P192_A_SIGN 0xffffffff
#define ECC_P192_SCALAR "\x00\x00\x00\x01"
#define ECC_P192_SCALAR_SIZE 4
#define ECC_P192_SCALAR_MULTIPLICATION_RESULT "\x18\x8d\xa8\x0e\xb0\x30\x90\xf6\x7c\xbf\x20\xeb\x43\xa1\x88\x00\xf4\xff\x0a\xfd\x82\xff\x10\x12\x07\x19\x2b\x95\xff\xc8\xda\x78\x63\x10\x11\xed\x6b\x24\xcd\xd5\x73\xf9\x77\xa1\x1e\x79\x48\x11"

#ifdef ECC_P192_SCALAR_MULTIPLICATION
	memcpy((char *)in, ECC_P192_COORD, ECC_P192_SIZE * 2);
	memcpy((char *)mod_ecc, ECC_P192_MODULUS, ECC_P192_SIZE);
	req->asym.modulus_size = ECC_P192_SIZE;
	req->asym.modulus = mod_ecc;
	curve->a_sign = ECC_P192_A_SIGN;
	memcpy((char *)a_ec, ECC_P192_A, ECC_P192_SIZE);
	curve->a = a_ec;
	memcpy((char *)k_ecc, ECC_P192_SCALAR, ECC_P192_SCALAR_SIZE);
	req->asym.k_size = ECC_P192_SCALAR_SIZE;
	req->asym.k = k_ecc;

	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, in, out, ECC_P192_SIZE * 2);

	perform_pka_test(req,
			 ECC_P192_SCALAR_MULTIPLICATION,
			 ECC_P192_SCALAR_MULTIPLICATION_RESULT);
#endif

#define ECC_P192_UNALIGNED_SCALAR_MULTIPLICATION "ECC word-unaligned scalar multiplication of NIST P-192 (k = 1) curve"
#define ECC_P192_SIZE (sizeof(int) * SIZE_IN_WORDS(192))
#define ECC_P192_COORD "\x18\x8d\xa8\x0e\xb0\x30\x90\xf6\x7c\xbf\x20\xeb\x43\xa1\x88\x00\xf4\xff\x0a\xfd\x82\xff\x10\x12\x07\x19\x2b\x95\xff\xc8\xda\x78\x63\x10\x11\xed\x6b\x24\xcd\xd5\x73\xf9\x77\xa1\x1e\x79\x48\x11"
#define ECC_P192_MODULUS "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xfe\xff\xff\xff\xff\xff\xff\xff\xff"
#define ECC_P192_A "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xfc"
#define ECC_P192_A_SIGN 0xffffffff
#define ECC_P192_UNALIGNED_SCALAR "\x01"
#define ECC_P192_UNALIGNED_SCALAR_SIZE 1
#define ECC_P192_SCALAR_MULTIPLICATION_RESULT "\x18\x8d\xa8\x0e\xb0\x30\x90\xf6\x7c\xbf\x20\xeb\x43\xa1\x88\x00\xf4\xff\x0a\xfd\x82\xff\x10\x12\x07\x19\x2b\x95\xff\xc8\xda\x78\x63\x10\x11\xed\x6b\x24\xcd\xd5\x73\xf9\x77\xa1\x1e\x79\x48\x11"

#ifdef ECC_P192_SCALAR_MULTIPLICATION
	memcpy((char *)in, ECC_P192_COORD, ECC_P192_SIZE * 2);
	memcpy((char *)mod_ecc, ECC_P192_MODULUS, ECC_P192_SIZE);
	req->asym.modulus_size = ECC_P192_SIZE;
	req->asym.modulus = mod_ecc;
	curve->a_sign = ECC_P192_A_SIGN;
	memcpy((char *)a_ec, ECC_P192_A, ECC_P192_SIZE);
	curve->a = a_ec;
	memcpy((char *)k_ecc, ECC_P192_UNALIGNED_SCALAR,
	       ECC_P192_UNALIGNED_SCALAR_SIZE);
	req->asym.k_size = ECC_P192_UNALIGNED_SCALAR_SIZE;
	req->asym.k = k_ecc;

	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, in, out, ECC_P192_SIZE * 2);

	perform_pka_test(req,
			 ECC_P192_UNALIGNED_SCALAR_MULTIPLICATION,
			 ECC_P192_SCALAR_MULTIPLICATION_RESULT);
#endif

#define ECC_P384_SCALAR_MULTIPLICATION "ECC scalar multiplication of a 384 bits curve"
#define ECC_P384_SIZE (sizeof(int) * SIZE_IN_WORDS(384))
#define ECC_P384_COORD "\xaa\x87\xca\x22\xbe\x8b\x05\x37\x8e\xb1\xc7\x1e\xf3\x20\xad\x74\x6e\x1d\x3b\x62\x8b\xa7\x9b\x98\x59\xf7\x41\xe0\x82\x54\x2a\x38\x55\x02\xf2\x5d\xbf\x55\x29\x6c\x3a\x54\x5e\x38\x72\x76\x0a\xb7\x36\x17\xde\x4a\x96\x26\x2c\x6f\x5d\x9e\x98\xbf\x92\x92\xdc\x29\xf8\xf4\x1d\xbd\x28\x9a\x14\x7c\xe9\xda\x31\x13\xb5\xf0\xb8\xc0\x0a\x60\xb1\xce\x1d\x7e\x81\x9d\x7a\x43\x1d\x7c\x90\xea\x0e\x5f"
#define ECC_P384_MODULUS "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xfe\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff"
#define ECC_P384_A "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xfe\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xfc"
#define ECC_P384_A_SIGN 0x0
#define ECC_P384_SCALAR "\x9b\xa7\x2e\xbf\x70\x9a\xab\x00\x53\x08\x89\x88\x75\xb0\x4a\x9f\x7f\x8e\x76\xc5\x8e\xb5\xe4\x8e\xfc\xce\xa4\xbd\xd0\xa4\x5a\x6b\x4c\x89\x2b\xbc\x24\xd6\xbd\x78\xdf\x47\x00\x54\xf8\x4b\xf3\x77"
#define ECC_P384_SCALAR_SIZE 48
#define ECC_P384_SCALAR_MULTIPLICATION_RESULT "\x13\x7C\x80\x88\xed\x08\xb3\x05\x6c\x42\x77\xfc\xa7\x62\x91\xa2\xbd\x89\xcd\x37\x6a\x7f\x1d\x43\xaf\x1e\xf0\x44\xb8\x8e\x7a\xd8\xc8\x9f\x34\x9c\xbb\xce\x63\xe2\x32\xc1\xf6\x06\x60\x30\x91\x03\x29\xc4\x36\x54\x54\xa0\x2c\xff\x70\x4d\x60\x66\x78\xc1\x5f\x78\xb1\x4d\x2d\xb7\xba\x8d\xae\xe0\x4e\x7c\xaa\xe7\x59\x2d\x4b\x87\x3a\xde\x42\xcf\x73\x5e\x48\x18\xa6\x0c\xe4\xdf\xce\xbe\xb6\xcc"

#ifdef ECC_P384_SCALAR_MULTIPLICATION
	memcpy((char *)in, ECC_P384_COORD, ECC_P384_SIZE * 2);
	memcpy((char *)mod_ecc, ECC_P384_MODULUS, ECC_P384_SIZE);
	req->asym.modulus_size = ECC_P384_SIZE;
	req->asym.modulus = mod_ecc;
	curve->a_sign = ECC_P384_A_SIGN;
	memcpy((char *)a_ec, ECC_P384_A, ECC_P384_SIZE);
	curve->a = a_ec;
	memcpy((char *)k_ecc, ECC_P384_SCALAR, ECC_P384_SCALAR_SIZE);
	req->asym.k_size = ECC_P384_SCALAR_SIZE;
	req->asym.k = k_ecc;

	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, in, out, ECC_P384_SIZE * 2);

	perform_pka_test(req,
			 ECC_P384_SCALAR_MULTIPLICATION,
			 ECC_P384_SCALAR_MULTIPLICATION_RESULT);
#endif

	req->asym.alg = PK_ECDSA_ALG;

#define ECDSA_192_SIGNATURE "ECDSA 192 signature generation"
#define ECDSA_192_SIZE (sizeof(int) * SIZE_IN_WORDS(192))
#define ECDSA_192_COORD "\x18\x8d\xa8\x0e\xb0\x30\x90\xf6\x7c\xbf\x20\xeb\x43\xa1\x88\x00\xf4\xff\x0a\xfd\x82\xff\x10\x12\x07\x19\x2b\x95\xff\xc8\xda\x78\x63\x10\x11\xed\x6b\x24\xcd\xd5\x73\xf9\x77\xa1\x1e\x79\x48\x11"
#define ECDSA_192_MODULUS "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xfe\xff\xff\xff\xff\xff\xff\xff\xff"
#define ECDSA_192_A "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03"
#define ECDSA_192_A_SIGN 0x01000000
#define ECDSA_192_ORDER "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x99\xde\xf8\x36\x14\x6b\xc9\xb1\xb4\xd2\x28\x31"
#define ECDSA_192_SECRET "\x86\x6f\x69\x9c\x2e\x6f\x67\xf1\x5d\x72\x80\x3a\x2d\x47\xd1\x38\xc5\xa8\x34\xb9\x24\x53\xec\x5b"
#define ECDSA_192_RANDOM "\x42\x39\x2e\xa2\xe1\xf4\x31\xf5\x17\xba\x4f\x74\x63\xb9\x7c\x53\xaf\x80\x81\xac\xa0\x92\xcb\x8c"
#define ECDSA_192_HASH "\xb3\x83\xc7\xc7\x5a\xf1\xaf\x60\x9d\x89\xae\x9f\xc8\xc4\xa7\x92\x0c\xe9\x78\xd3\x39\xa6\x38\x7b"
#define ECDSA_192_SIGNATURE_RESULT "\x16\xc5\xb1\xcc\xea\x6c\xc9\x08\xeb\x26\xd1\xb3\xea\x55\xa6\x96\x97\xd8\x28\xcf\x4f\x76\x54\x2f\x12\xc4\xb6\x15\x23\x88\xb3\xef\x3b\xf8\xdd\xd8\x76\x95\x54\x0b\xf9\xf7\xbc\x14\xa1\x58\xf1\x19"

#ifdef ECDSA_192_SIGNATURE
	memcpy((char *)in, ECDSA_192_COORD, ECDSA_192_SIZE * 2);
	memcpy((char *)mod_ecc, ECDSA_192_MODULUS, ECDSA_192_SIZE);
	req->asym.modulus_size = ECDSA_192_SIZE;
	req->asym.modulus = mod_ecc;
	curve->a_sign = ECDSA_192_A_SIGN;
	memcpy((char *)a_ec, ECDSA_192_A, ECDSA_192_SIZE);
	curve->a = a_ec;
	memcpy((char *)n_ecdsa, ECDSA_192_ORDER, ECDSA_192_SIZE);
	curve->n_size = ECDSA_192_SIZE;
	curve->n = n_ecdsa;
	memcpy((char *)d_ecdsa, ECDSA_192_SECRET, ECDSA_192_SIZE);
	curve->d = d_ecdsa;
	memcpy((char *)k_ecdsa, ECDSA_192_RANDOM, ECDSA_192_SIZE);
	curve->k = k_ecdsa;
	memcpy((char *)e_ecdsa, ECDSA_192_HASH, ECDSA_192_SIZE);
	curve->e = e_ecdsa;

	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, in, out, ECDSA_192_SIZE * 2);

	perform_pka_test(req,
			 ECDSA_192_SIGNATURE,
			 ECDSA_192_SIGNATURE_RESULT);
#endif

#define ECDSA_192_SIGNATURE_WITH_ZERO_HEADED_MODULUS "ECDSA 192 signature generation with a word-unaligned zero-headed modulus"
#define ECDSA_192_SIZE (sizeof(int) * SIZE_IN_WORDS(192))
#define ECDSA_200_SIZE SIZE_IN_BYTES(200)
#define ECDSA_192_0_HEADED_MODULUS_SIZE ECDSA_200_SIZE
#define ECDSA_192_0_HEADED_COORD "\x00\x18\x8d\xa8\x0e\xb0\x30\x90\xf6\x7c\xbf\x20\xeb\x43\xa1\x88\x00\xf4\xff\x0a\xfd\x82\xff\x10\x12\x00\x07\x19\x2b\x95\xff\xc8\xda\x78\x63\x10\x11\xed\x6b\x24\xcd\xd5\x73\xf9\x77\xa1\x1e\x79\x48\x11"
#define ECDSA_192_0_HEADED_MODULUS "\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xfe\xff\xff\xff\xff\xff\xff\xff\xff"
#define ECDSA_192_0_HEADED_A "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03"
#define ECDSA_192_A_SIGN 0x01000000
#define ECDSA_192_ORDER "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x99\xde\xf8\x36\x14\x6b\xc9\xb1\xb4\xd2\x28\x31"
#define ECDSA_192_SECRET "\x86\x6f\x69\x9c\x2e\x6f\x67\xf1\x5d\x72\x80\x3a\x2d\x47\xd1\x38\xc5\xa8\x34\xb9\x24\x53\xec\x5b"
#define ECDSA_192_RANDOM "\x42\x39\x2e\xa2\xe1\xf4\x31\xf5\x17\xba\x4f\x74\x63\xb9\x7c\x53\xaf\x80\x81\xac\xa0\x92\xcb\x8c"
#define ECDSA_192_HASH "\xb3\x83\xc7\xc7\x5a\xf1\xaf\x60\x9d\x89\xae\x9f\xc8\xc4\xa7\x92\x0c\xe9\x78\xd3\x39\xa6\x38\x7b"
#define ECDSA_192_0_HEADED_SIGNATURE_RESULT "\x00\x16\xc5\xb1\xcc\xea\x6c\xc9\x08\xeb\x26\xd1\xb3\xea\x55\xa6\x96\x97\xd8\x28\xcf\x4f\x76\x54\x2f\x00\x12\xc4\xb6\x15\x23\x88\xb3\xef\x3b\xf8\xdd\xd8\x76\x95\x54\x0b\xf9\xf7\xbc\x14\xa1\x58\xf1\x19"

#ifdef ECDSA_192_SIGNATURE_WITH_ZERO_HEADED_MODULUS
	memcpy((char *)in, ECDSA_192_0_HEADED_COORD, ECDSA_192_0_HEADED_MODULUS_SIZE * 2);
	memcpy((char *)mod_ecc, ECDSA_192_0_HEADED_MODULUS, ECDSA_192_0_HEADED_MODULUS_SIZE);
	req->asym.modulus_size = ECDSA_192_0_HEADED_MODULUS_SIZE;
	req->asym.modulus = mod_ecc;
	curve->a_sign = ECDSA_192_A_SIGN;
	memcpy((char *)a_ec, ECDSA_192_0_HEADED_A, ECDSA_192_0_HEADED_MODULUS_SIZE);
	curve->a = a_ec;
	memcpy((char *)n_ecdsa, ECDSA_192_ORDER, ECDSA_192_SIZE);
	curve->n_size = ECDSA_192_SIZE;
	curve->n = n_ecdsa;
	memcpy((char *)d_ecdsa, ECDSA_192_SECRET, ECDSA_192_SIZE);
	curve->d = d_ecdsa;
	memcpy((char *)k_ecdsa, ECDSA_192_RANDOM, ECDSA_192_SIZE);
	curve->k = k_ecdsa;
	memcpy((char *)e_ecdsa, ECDSA_192_HASH, ECDSA_192_SIZE);
	curve->e = e_ecdsa;

	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, in, out, ECDSA_192_0_HEADED_MODULUS_SIZE * 2);

	perform_pka_test(req,
			 ECDSA_192_SIGNATURE_WITH_ZERO_HEADED_MODULUS,
			 ECDSA_192_0_HEADED_SIGNATURE_RESULT);
#endif

#define ECDSA_192_VERIFICATION "ECDSA 192 signature verification"
#define ECDSA_192_SIZE (sizeof(int) * SIZE_IN_WORDS(192))
#define ECDSA_192_COORD "\x18\x8d\xa8\x0e\xb0\x30\x90\xf6\x7c\xbf\x20\xeb\x43\xa1\x88\x00\xf4\xff\x0a\xfd\x82\xff\x10\x12\x07\x19\x2b\x95\xff\xc8\xda\x78\x63\x10\x11\xed\x6b\x24\xcd\xd5\x73\xf9\x77\xa1\x1e\x79\x48\x11"
#define ECDSA_192_MODULUS "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xfe\xff\xff\xff\xff\xff\xff\xff\xff"
#define ECDSA_192_A "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03"
#define ECDSA_192_A_SIGN 0x01000000
#define ECDSA_192_ORDER "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x99\xde\xf8\x36\x14\x6b\xc9\xb1\xb4\xd2\x28\x31"
#define ECDSA_192_PUBLIC_KEY "\xda\x27\x32\xa2\xbd\xa6\x0f\x9f\x34\xcb\xeb\x54\xf2\x8c\xd4\x43\xed\xcb\x56\x36\xe6\xfd\x93\xfc\xb3\x43\x0b\x7f\x4b\x3e\xf7\xf1\x5c\x10\xc7\x35\x8d\x5a\xcf\xc1\x02\x1f\x37\xd1\xca\x7b\x97\xd4"
#define ECDSA_192_HASH "\xb3\x83\xc7\xc7\x5a\xf1\xaf\x60\x9d\x89\xae\x9f\xc8\xc4\xa7\x92\x0c\xe9\x78\xd3\x39\xa6\x38\x7b"
#define ECDSA_192_SIGNATURE_1ST_PART "\x16\xc5\xb1\xcc\xea\x6c\xc9\x08\xeb\x26\xd1\xb3\xea\x55\xa6\x96\x97\xd8\x28\xcf\x4f\x76\x54\x2f"
#define ECDSA_192_SIGNATURE_2ND_PART "\x12\xc4\xb6\x15\x23\x88\xb3\xef\x3b\xf8\xdd\xd8\x76\x95\x54\x0b\xf9\xf7\xbc\x14\xa1\x58\xf1\x19"
#define ECDSA_192_VERIFICATION_RESULT NULL

#ifdef ECDSA_192_VERIFICATION
	memcpy((char *)in, ECDSA_192_COORD, ECDSA_192_SIZE * 2);
	memcpy((char *)mod_ecc, ECDSA_192_MODULUS, ECDSA_192_SIZE);
	req->asym.modulus_size = ECDSA_192_SIZE;
	req->asym.modulus = mod_ecc;
	curve->a_sign = ECDSA_192_A_SIGN;
	memcpy((char *)a_ec, ECDSA_192_A, ECDSA_192_SIZE);
	curve->a = a_ec;
	memcpy((char *)n_ecdsa, ECDSA_192_ORDER, ECDSA_192_SIZE);
	curve->n_size = ECDSA_192_SIZE;
	curve->n = n_ecdsa;
	curve->d = NULL;
	curve->k = NULL;
	memcpy((char *)q_ecdsa, ECDSA_192_PUBLIC_KEY, ECDSA_192_SIZE * 2);
	curve->q = q_ecdsa;
	memcpy((char *)r_ecdsa, ECDSA_192_SIGNATURE_1ST_PART, ECDSA_192_SIZE);
	curve->r = r_ecdsa;
	memcpy((char *)s_ecdsa, ECDSA_192_SIGNATURE_2ND_PART, ECDSA_192_SIZE);
	curve->s = s_ecdsa;
	memcpy((char *)e_ecdsa, ECDSA_192_HASH, ECDSA_192_SIZE);
	curve->e = e_ecdsa;

	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, in, out, ECDSA_192_SIZE * 2);

	perform_pka_test(req,
			 ECDSA_192_VERIFICATION,
			 ECDSA_192_VERIFICATION_RESULT);
#endif

#define ECDSA_521_SIGNATURE "ECDSA NIST P-521 signature generation"
#define ECDSA_521_SIZE (sizeof(int) * SIZE_IN_WORDS(521))
#define ECDSA_521_COORD "\x00\x00\x00\xC6\x85\x8E\x06\xB7\x04\x04\xE9\xCD\x9E\x3E\xCB\x66\x23\x95\xB4\x42\x9C\x64\x81\x39\x05\x3F\xB5\x21\xF8\x28\xAF\x60\x6B\x4D\x3D\xBA\xA1\x4B\x5E\x77\xEF\xE7\x59\x28\xFE\x1D\xC1\x27\xA2\xFF\xA8\xDE\x33\x48\xB3\xC1\x85\x6A\x42\x9B\xF9\x7E\x7E\x31\xC2\xE5\xBD\x66\x00\x00\x01\x18\x39\x29\x6A\x78\x9A\x3B\xC0\x04\x5C\x8A\x5F\xB4\x2C\x7D\x1B\xD9\x98\xF5\x44\x49\x57\x9B\x44\x68\x17\xAF\xBD\x17\x27\x3E\x66\x2C\x97\xEE\x72\x99\x5E\xF4\x26\x40\xC5\x50\xB9\x01\x3F\xAD\x07\x61\x35\x3C\x70\x86\xA2\x72\xC2\x40\x88\xBE\x94\x76\x9F\xD1\x66\x50"
#define ECDSA_521_MODULUS "\x00\x00\x01\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
#define ECDSA_521_A "\x00\x00\x01\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFC"
#define ECDSA_521_A_SIGN 0x0
#define ECDSA_521_ORDER "\x00\x00\x01\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFA\x51\x86\x87\x83\xBF\x2F\x96\x6B\x7F\xCC\x01\x48\xF7\x09\xA5\xD0\x3B\xB5\xC9\xB8\x89\x9C\x47\xAE\xBB\x6F\xB7\x1E\x91\x38\x64\x09"
#define ECDSA_521_SECRET "\x00\x00\x00\xf7\x49\xd3\x27\x04\xbc\x53\x3c\xa8\x2c\xef\x0a\xcf\x10\x3d\x8f\x4f\xba\x67\xf0\x8d\x26\x78\xe5\x15\xed\x7d\xb8\x86\x26\x7f\xfa\xf0\x2f\xab\x00\x80\xdc\xa2\x35\x9b\x72\xf5\x74\xcc\xc2\x9a\x0f\x21\x8c\x86\x55\xc0\xcc\xcf\x9f\xee\x6c\x5e\x56\x7a\xa1\x4c\xb9\x26"
#define ECDSA_521_RANDOM "\x00\x00\x00\x3a\xf5\xab\x6c\xaa\x29\xa6\xde\x86\xa5\xba\xb9\xaa\x83\xc3\xb1\x6a\x17\xff\xcd\x52\xb5\xc6\x0c\x76\x9b\xe3\x05\x3c\xdd\xde\xac\x60\x81\x2d\x12\xfe\xcf\x46\xcf\xe1\xf3\xdb\x9a\xc9\xdc\xf8\x81\xfc\xec\x3f\x0a\xa7\x33\xd4\xec\xbb\x83\xc7\x59\x3e\x86\x4c\x6d\xf1"
#define ECDSA_521_MESSAGE "\x9e\xcd\x50\x0c\x60\xe7\x01\x40\x49\x22\xe5\x8a\xb2\x0c\xc0\x02\x65\x1f\xde\xe7\xcb\xc9\x33\x6a\xdd\xa3\x3e\x4c\x10\x88\xfa\xb1\x96\x4e\xcb\x79\x04\xdc\x68\x56\x86\x5d\x6c\x8e\x15\x04\x1c\xcf\x2d\x5a\xc3\x02\xe9\x9d\x34\x6f\xf2\xf6\x86\x53\x1d\x25\x52\x16\x78\xd4\xfd\x3f\x76\xbb\xf2\xc8\x93\xd2\x46\xcb\x4d\x76\x93\x79\x2f\xe1\x81\x72\x10\x81\x46\x85\x31\x03\xa5\x1f\x82\x4a\xcc\x62\x1c\xb7\x31\x1d\x24\x63\xc3\x36\x1e\xa7\x07\x25\x4f\x2b\x05\x2b\xc2\x2c\xb8\x01\x28\x73\xdc\xbb\x95\xbf\x1a\x5c\xc5\x3a\xb8\x9f"
#define ECDSA_521_HASH "\x00\x00\x00\x00\x65\xf8\x34\x08\x09\x22\x61\xbd\xa5\x99\x38\x9d\xf0\x33\x82\xc5\xbe\x01\xa8\x1f\xe0\x0a\x36\xf3\xf4\xbb\x65\x41\x26\x3f\x80\x16\x27\xc4\x40\xe5\x08\x09\x71\x2b\x0c\xac\xe7\xc2\x17\xe6\xe5\x05\x1a\xf8\x1d\xe9\xbf\xec\x32\x04\xdc\xd6\x3c\x4f\x9a\x74\x10\x47"
#define ECDSA_521_SIGNATURE_RESULT "\x00\x00\x00\x4d\xe8\x26\xea\x70\x4a\xd1\x0b\xc0\xf7\x53\x8a\xf8\xa3\x84\x3f\x28\x4f\x55\xc8\xb9\x46\xaf\x92\x35\xaf\x5a\xf7\x4f\x2b\x76\xe0\x99\xe4\xbc\x72\xfd\x79\xd2\x8a\x38\x0f\x8d\x4b\x4c\x91\x9a\xc2\x90\xd2\x48\xc3\x79\x83\xba\x05\xae\xa4\x2e\x2d\xd7\x9f\xdd\x33\xe8\x00\x00\x00\x87\x48\x8c\x85\x9a\x96\xfe\xa2\x66\xea\x13\xbf\x6d\x11\x4c\x42\x9b\x16\x3b\xe9\x7a\x57\x55\x90\x86\xed\xb6\x4a\xed\x4a\x18\x59\x4b\x46\xfb\x9e\xfc\x7f\xd2\x5d\x8b\x2d\xe8\xf0\x9c\xa0\x58\x7f\x54\xbd\x28\x72\x99\xf4\x7b\x2f\xf1\x24\xaa\xc5\x66\xe8\xee\x3b\x43"

#ifdef ECDSA_521_SIGNATURE
	memcpy((char *)in, ECDSA_521_COORD, ECDSA_521_SIZE * 2);
	memcpy((char *)mod_ecc, ECDSA_521_MODULUS, ECDSA_521_SIZE);
	req->asym.modulus_size = ECDSA_521_SIZE;
	req->asym.modulus = mod_ecc;
	curve->a_sign = ECDSA_521_A_SIGN;
	memcpy((char *)a_ec, ECDSA_521_A, ECDSA_521_SIZE);
	curve->a = a_ec;
	memcpy((char *)n_ecdsa, ECDSA_521_ORDER, ECDSA_521_SIZE);
	curve->n_size = ECDSA_521_SIZE;
	curve->n = n_ecdsa;
	memcpy((char *)d_ecdsa, ECDSA_521_SECRET, ECDSA_521_SIZE);
	curve->d = d_ecdsa;
	memcpy((char *)k_ecdsa, ECDSA_521_RANDOM, ECDSA_521_SIZE);
	curve->k = k_ecdsa;
	memcpy((char *)e_ecdsa, ECDSA_521_HASH, ECDSA_521_SIZE);
	curve->e = e_ecdsa;

	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, in, out, ECDSA_521_SIZE * 2);

	perform_pka_test(req,
			 ECDSA_521_SIGNATURE,
			 ECDSA_521_SIGNATURE_RESULT);
#endif

#define ECDSA_521_SIGNATURE_66 "ECDSA NIST P-521 signature generation with all parameters word-unaligned"
#define ECDSA_521_SIZE_66 66
#define ECDSA_521_COORD_66 "\x00\xC6\x85\x8E\x06\xB7\x04\x04\xE9\xCD\x9E\x3E\xCB\x66\x23\x95\xB4\x42\x9C\x64\x81\x39\x05\x3F\xB5\x21\xF8\x28\xAF\x60\x6B\x4D\x3D\xBA\xA1\x4B\x5E\x77\xEF\xE7\x59\x28\xFE\x1D\xC1\x27\xA2\xFF\xA8\xDE\x33\x48\xB3\xC1\x85\x6A\x42\x9B\xF9\x7E\x7E\x31\xC2\xE5\xBD\x66\x01\x18\x39\x29\x6A\x78\x9A\x3B\xC0\x04\x5C\x8A\x5F\xB4\x2C\x7D\x1B\xD9\x98\xF5\x44\x49\x57\x9B\x44\x68\x17\xAF\xBD\x17\x27\x3E\x66\x2C\x97\xEE\x72\x99\x5E\xF4\x26\x40\xC5\x50\xB9\x01\x3F\xAD\x07\x61\x35\x3C\x70\x86\xA2\x72\xC2\x40\x88\xBE\x94\x76\x9F\xD1\x66\x50"
#define ECDSA_521_MODULUS_66 "\x01\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
#define ECDSA_521_A_66 "\x01\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFC"
#define ECDSA_521_A_SIGN 0x0
#define ECDSA_521_ORDER_66 "\x01\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFA\x51\x86\x87\x83\xBF\x2F\x96\x6B\x7F\xCC\x01\x48\xF7\x09\xA5\xD0\x3B\xB5\xC9\xB8\x89\x9C\x47\xAE\xBB\x6F\xB7\x1E\x91\x38\x64\x09"
#define ECDSA_521_SECRET_66 "\x00\xf7\x49\xd3\x27\x04\xbc\x53\x3c\xa8\x2c\xef\x0a\xcf\x10\x3d\x8f\x4f\xba\x67\xf0\x8d\x26\x78\xe5\x15\xed\x7d\xb8\x86\x26\x7f\xfa\xf0\x2f\xab\x00\x80\xdc\xa2\x35\x9b\x72\xf5\x74\xcc\xc2\x9a\x0f\x21\x8c\x86\x55\xc0\xcc\xcf\x9f\xee\x6c\x5e\x56\x7a\xa1\x4c\xb9\x26"
#define ECDSA_521_RANDOM_66 "\x00\x3a\xf5\xab\x6c\xaa\x29\xa6\xde\x86\xa5\xba\xb9\xaa\x83\xc3\xb1\x6a\x17\xff\xcd\x52\xb5\xc6\x0c\x76\x9b\xe3\x05\x3c\xdd\xde\xac\x60\x81\x2d\x12\xfe\xcf\x46\xcf\xe1\xf3\xdb\x9a\xc9\xdc\xf8\x81\xfc\xec\x3f\x0a\xa7\x33\xd4\xec\xbb\x83\xc7\x59\x3e\x86\x4c\x6d\xf1"
#define ECDSA_521_MESSAGE "\x9e\xcd\x50\x0c\x60\xe7\x01\x40\x49\x22\xe5\x8a\xb2\x0c\xc0\x02\x65\x1f\xde\xe7\xcb\xc9\x33\x6a\xdd\xa3\x3e\x4c\x10\x88\xfa\xb1\x96\x4e\xcb\x79\x04\xdc\x68\x56\x86\x5d\x6c\x8e\x15\x04\x1c\xcf\x2d\x5a\xc3\x02\xe9\x9d\x34\x6f\xf2\xf6\x86\x53\x1d\x25\x52\x16\x78\xd4\xfd\x3f\x76\xbb\xf2\xc8\x93\xd2\x46\xcb\x4d\x76\x93\x79\x2f\xe1\x81\x72\x10\x81\x46\x85\x31\x03\xa5\x1f\x82\x4a\xcc\x62\x1c\xb7\x31\x1d\x24\x63\xc3\x36\x1e\xa7\x07\x25\x4f\x2b\x05\x2b\xc2\x2c\xb8\x01\x28\x73\xdc\xbb\x95\xbf\x1a\x5c\xc5\x3a\xb8\x9f"
#define ECDSA_521_HASH_66 "\x00\x00\x65\xf8\x34\x08\x09\x22\x61\xbd\xa5\x99\x38\x9d\xf0\x33\x82\xc5\xbe\x01\xa8\x1f\xe0\x0a\x36\xf3\xf4\xbb\x65\x41\x26\x3f\x80\x16\x27\xc4\x40\xe5\x08\x09\x71\x2b\x0c\xac\xe7\xc2\x17\xe6\xe5\x05\x1a\xf8\x1d\xe9\xbf\xec\x32\x04\xdc\xd6\x3c\x4f\x9a\x74\x10\x47"
#define ECDSA_521_SIGNATURE_RESULT_66 "\x00\x4d\xe8\x26\xea\x70\x4a\xd1\x0b\xc0\xf7\x53\x8a\xf8\xa3\x84\x3f\x28\x4f\x55\xc8\xb9\x46\xaf\x92\x35\xaf\x5a\xf7\x4f\x2b\x76\xe0\x99\xe4\xbc\x72\xfd\x79\xd2\x8a\x38\x0f\x8d\x4b\x4c\x91\x9a\xc2\x90\xd2\x48\xc3\x79\x83\xba\x05\xae\xa4\x2e\x2d\xd7\x9f\xdd\x33\xe8\x00\x87\x48\x8c\x85\x9a\x96\xfe\xa2\x66\xea\x13\xbf\x6d\x11\x4c\x42\x9b\x16\x3b\xe9\x7a\x57\x55\x90\x86\xed\xb6\x4a\xed\x4a\x18\x59\x4b\x46\xfb\x9e\xfc\x7f\xd2\x5d\x8b\x2d\xe8\xf0\x9c\xa0\x58\x7f\x54\xbd\x28\x72\x99\xf4\x7b\x2f\xf1\x24\xaa\xc5\x66\xe8\xee\x3b\x43"

#ifdef ECDSA_521_SIGNATURE_66
	memcpy((char *)in, ECDSA_521_COORD_66, ECDSA_521_SIZE_66 * 2);
	memcpy((char *)mod_ecc, ECDSA_521_MODULUS_66, ECDSA_521_SIZE_66);
	req->asym.modulus_size = ECDSA_521_SIZE_66;
	req->asym.modulus = mod_ecc;
	curve->a_sign = ECDSA_521_A_SIGN;
	memcpy((char *)a_ec, ECDSA_521_A_66, ECDSA_521_SIZE_66);
	curve->a = a_ec;
	memcpy((char *)n_ecdsa, ECDSA_521_ORDER_66, ECDSA_521_SIZE_66);
	curve->n_size = ECDSA_521_SIZE_66;
	curve->n = n_ecdsa;
	memcpy((char *)d_ecdsa, ECDSA_521_SECRET_66, ECDSA_521_SIZE_66);
	curve->d = d_ecdsa;
	memcpy((char *)k_ecdsa, ECDSA_521_RANDOM_66, ECDSA_521_SIZE_66);
	curve->k = k_ecdsa;
	memcpy((char *)e_ecdsa, ECDSA_521_HASH_66, ECDSA_521_SIZE_66);
	curve->e = e_ecdsa;

	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, in, out, ECDSA_521_SIZE_66 * 2);

	perform_pka_test(req,
			 ECDSA_521_SIGNATURE_66,
			 ECDSA_521_SIGNATURE_RESULT_66);
#endif

#define ECDSA_521_SIGNATURE_66_68 "ECDSA NIST P-521 signature generation with a modulus, an 'a' parameter and coordinates word-unaligned"
#define ECDSA_521_SIZE_66 66
#define ECDSA_521_COORD "\x00\x00\x00\xC6\x85\x8E\x06\xB7\x04\x04\xE9\xCD\x9E\x3E\xCB\x66\x23\x95\xB4\x42\x9C\x64\x81\x39\x05\x3F\xB5\x21\xF8\x28\xAF\x60\x6B\x4D\x3D\xBA\xA1\x4B\x5E\x77\xEF\xE7\x59\x28\xFE\x1D\xC1\x27\xA2\xFF\xA8\xDE\x33\x48\xB3\xC1\x85\x6A\x42\x9B\xF9\x7E\x7E\x31\xC2\xE5\xBD\x66\x00\x00\x01\x18\x39\x29\x6A\x78\x9A\x3B\xC0\x04\x5C\x8A\x5F\xB4\x2C\x7D\x1B\xD9\x98\xF5\x44\x49\x57\x9B\x44\x68\x17\xAF\xBD\x17\x27\x3E\x66\x2C\x97\xEE\x72\x99\x5E\xF4\x26\x40\xC5\x50\xB9\x01\x3F\xAD\x07\x61\x35\x3C\x70\x86\xA2\x72\xC2\x40\x88\xBE\x94\x76\x9F\xD1\x66\x50"
#define ECDSA_521_MODULUS "\x00\x00\x01\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
#define ECDSA_521_A "\x00\x00\x01\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFC"
#define ECDSA_521_A_SIGN 0x0
#define ECDSA_521_ORDER_66 "\x01\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFA\x51\x86\x87\x83\xBF\x2F\x96\x6B\x7F\xCC\x01\x48\xF7\x09\xA5\xD0\x3B\xB5\xC9\xB8\x89\x9C\x47\xAE\xBB\x6F\xB7\x1E\x91\x38\x64\x09"
#define ECDSA_521_SECRET_66 "\x00\xf7\x49\xd3\x27\x04\xbc\x53\x3c\xa8\x2c\xef\x0a\xcf\x10\x3d\x8f\x4f\xba\x67\xf0\x8d\x26\x78\xe5\x15\xed\x7d\xb8\x86\x26\x7f\xfa\xf0\x2f\xab\x00\x80\xdc\xa2\x35\x9b\x72\xf5\x74\xcc\xc2\x9a\x0f\x21\x8c\x86\x55\xc0\xcc\xcf\x9f\xee\x6c\x5e\x56\x7a\xa1\x4c\xb9\x26"
#define ECDSA_521_RANDOM_66 "\x00\x3a\xf5\xab\x6c\xaa\x29\xa6\xde\x86\xa5\xba\xb9\xaa\x83\xc3\xb1\x6a\x17\xff\xcd\x52\xb5\xc6\x0c\x76\x9b\xe3\x05\x3c\xdd\xde\xac\x60\x81\x2d\x12\xfe\xcf\x46\xcf\xe1\xf3\xdb\x9a\xc9\xdc\xf8\x81\xfc\xec\x3f\x0a\xa7\x33\xd4\xec\xbb\x83\xc7\x59\x3e\x86\x4c\x6d\xf1"
#define ECDSA_521_MESSAGE "\x9e\xcd\x50\x0c\x60\xe7\x01\x40\x49\x22\xe5\x8a\xb2\x0c\xc0\x02\x65\x1f\xde\xe7\xcb\xc9\x33\x6a\xdd\xa3\x3e\x4c\x10\x88\xfa\xb1\x96\x4e\xcb\x79\x04\xdc\x68\x56\x86\x5d\x6c\x8e\x15\x04\x1c\xcf\x2d\x5a\xc3\x02\xe9\x9d\x34\x6f\xf2\xf6\x86\x53\x1d\x25\x52\x16\x78\xd4\xfd\x3f\x76\xbb\xf2\xc8\x93\xd2\x46\xcb\x4d\x76\x93\x79\x2f\xe1\x81\x72\x10\x81\x46\x85\x31\x03\xa5\x1f\x82\x4a\xcc\x62\x1c\xb7\x31\x1d\x24\x63\xc3\x36\x1e\xa7\x07\x25\x4f\x2b\x05\x2b\xc2\x2c\xb8\x01\x28\x73\xdc\xbb\x95\xbf\x1a\x5c\xc5\x3a\xb8\x9f"
#define ECDSA_521_HASH_66 "\x00\x00\x65\xf8\x34\x08\x09\x22\x61\xbd\xa5\x99\x38\x9d\xf0\x33\x82\xc5\xbe\x01\xa8\x1f\xe0\x0a\x36\xf3\xf4\xbb\x65\x41\x26\x3f\x80\x16\x27\xc4\x40\xe5\x08\x09\x71\x2b\x0c\xac\xe7\xc2\x17\xe6\xe5\x05\x1a\xf8\x1d\xe9\xbf\xec\x32\x04\xdc\xd6\x3c\x4f\x9a\x74\x10\x47"
#define ECDSA_521_SIGNATURE_RESULT "\x00\x00\x00\x4d\xe8\x26\xea\x70\x4a\xd1\x0b\xc0\xf7\x53\x8a\xf8\xa3\x84\x3f\x28\x4f\x55\xc8\xb9\x46\xaf\x92\x35\xaf\x5a\xf7\x4f\x2b\x76\xe0\x99\xe4\xbc\x72\xfd\x79\xd2\x8a\x38\x0f\x8d\x4b\x4c\x91\x9a\xc2\x90\xd2\x48\xc3\x79\x83\xba\x05\xae\xa4\x2e\x2d\xd7\x9f\xdd\x33\xe8\x00\x00\x00\x87\x48\x8c\x85\x9a\x96\xfe\xa2\x66\xea\x13\xbf\x6d\x11\x4c\x42\x9b\x16\x3b\xe9\x7a\x57\x55\x90\x86\xed\xb6\x4a\xed\x4a\x18\x59\x4b\x46\xfb\x9e\xfc\x7f\xd2\x5d\x8b\x2d\xe8\xf0\x9c\xa0\x58\x7f\x54\xbd\x28\x72\x99\xf4\x7b\x2f\xf1\x24\xaa\xc5\x66\xe8\xee\x3b\x43"

#ifdef ECDSA_521_SIGNATURE_66_68
	memcpy((char *)in, ECDSA_521_COORD, ECDSA_521_SIZE * 2);
	memcpy((char *)mod_ecc, ECDSA_521_MODULUS, ECDSA_521_SIZE);
	req->asym.modulus_size = ECDSA_521_SIZE;
	req->asym.modulus = mod_ecc;
	curve->a_sign = ECDSA_521_A_SIGN;
	memcpy((char *)a_ec, ECDSA_521_A, ECDSA_521_SIZE);
	curve->a = a_ec;
	memcpy((char *)n_ecdsa, ECDSA_521_ORDER_66, ECDSA_521_SIZE_66);
	curve->n_size = ECDSA_521_SIZE_66;
	curve->n = n_ecdsa;
	memcpy((char *)d_ecdsa, ECDSA_521_SECRET_66, ECDSA_521_SIZE_66);
	curve->d = d_ecdsa;
	memcpy((char *)k_ecdsa, ECDSA_521_RANDOM_66, ECDSA_521_SIZE_66);
	curve->k = k_ecdsa;
	memcpy((char *)e_ecdsa, ECDSA_521_HASH_66, ECDSA_521_SIZE_66);
	curve->e = e_ecdsa;

	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, in, out, ECDSA_521_SIZE * 2);

	perform_pka_test(req,
			 ECDSA_521_SIGNATURE_66_68,
			 ECDSA_521_SIGNATURE_RESULT);
#endif

#define ECDSA_521_SIGNATURE_66_MODULUS "ECDSA NIST P-521 signature generation with a modulus and an 'a' parameter word-unaligned"
#define ECDSA_521_SIZE (sizeof(int) * SIZE_IN_WORDS(521))
#define ECDSA_521_COORD "\x00\x00\x00\xC6\x85\x8E\x06\xB7\x04\x04\xE9\xCD\x9E\x3E\xCB\x66\x23\x95\xB4\x42\x9C\x64\x81\x39\x05\x3F\xB5\x21\xF8\x28\xAF\x60\x6B\x4D\x3D\xBA\xA1\x4B\x5E\x77\xEF\xE7\x59\x28\xFE\x1D\xC1\x27\xA2\xFF\xA8\xDE\x33\x48\xB3\xC1\x85\x6A\x42\x9B\xF9\x7E\x7E\x31\xC2\xE5\xBD\x66\x00\x00\x01\x18\x39\x29\x6A\x78\x9A\x3B\xC0\x04\x5C\x8A\x5F\xB4\x2C\x7D\x1B\xD9\x98\xF5\x44\x49\x57\x9B\x44\x68\x17\xAF\xBD\x17\x27\x3E\x66\x2C\x97\xEE\x72\x99\x5E\xF4\x26\x40\xC5\x50\xB9\x01\x3F\xAD\x07\x61\x35\x3C\x70\x86\xA2\x72\xC2\x40\x88\xBE\x94\x76\x9F\xD1\x66\x50"
#define ECDSA_521_MODULUS_66 "\x01\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
#define ECDSA_521_A_66 "\x01\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFC"
#define ECDSA_521_A_SIGN 0x0
#define ECDSA_521_ORDER "\x00\x00\x01\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFA\x51\x86\x87\x83\xBF\x2F\x96\x6B\x7F\xCC\x01\x48\xF7\x09\xA5\xD0\x3B\xB5\xC9\xB8\x89\x9C\x47\xAE\xBB\x6F\xB7\x1E\x91\x38\x64\x09"
#define ECDSA_521_SECRET "\x00\x00\x00\xf7\x49\xd3\x27\x04\xbc\x53\x3c\xa8\x2c\xef\x0a\xcf\x10\x3d\x8f\x4f\xba\x67\xf0\x8d\x26\x78\xe5\x15\xed\x7d\xb8\x86\x26\x7f\xfa\xf0\x2f\xab\x00\x80\xdc\xa2\x35\x9b\x72\xf5\x74\xcc\xc2\x9a\x0f\x21\x8c\x86\x55\xc0\xcc\xcf\x9f\xee\x6c\x5e\x56\x7a\xa1\x4c\xb9\x26"
#define ECDSA_521_RANDOM "\x00\x00\x00\x3a\xf5\xab\x6c\xaa\x29\xa6\xde\x86\xa5\xba\xb9\xaa\x83\xc3\xb1\x6a\x17\xff\xcd\x52\xb5\xc6\x0c\x76\x9b\xe3\x05\x3c\xdd\xde\xac\x60\x81\x2d\x12\xfe\xcf\x46\xcf\xe1\xf3\xdb\x9a\xc9\xdc\xf8\x81\xfc\xec\x3f\x0a\xa7\x33\xd4\xec\xbb\x83\xc7\x59\x3e\x86\x4c\x6d\xf1"
#define ECDSA_521_MESSAGE "\x9e\xcd\x50\x0c\x60\xe7\x01\x40\x49\x22\xe5\x8a\xb2\x0c\xc0\x02\x65\x1f\xde\xe7\xcb\xc9\x33\x6a\xdd\xa3\x3e\x4c\x10\x88\xfa\xb1\x96\x4e\xcb\x79\x04\xdc\x68\x56\x86\x5d\x6c\x8e\x15\x04\x1c\xcf\x2d\x5a\xc3\x02\xe9\x9d\x34\x6f\xf2\xf6\x86\x53\x1d\x25\x52\x16\x78\xd4\xfd\x3f\x76\xbb\xf2\xc8\x93\xd2\x46\xcb\x4d\x76\x93\x79\x2f\xe1\x81\x72\x10\x81\x46\x85\x31\x03\xa5\x1f\x82\x4a\xcc\x62\x1c\xb7\x31\x1d\x24\x63\xc3\x36\x1e\xa7\x07\x25\x4f\x2b\x05\x2b\xc2\x2c\xb8\x01\x28\x73\xdc\xbb\x95\xbf\x1a\x5c\xc5\x3a\xb8\x9f"
#define ECDSA_521_HASH "\x00\x00\x00\x00\x65\xf8\x34\x08\x09\x22\x61\xbd\xa5\x99\x38\x9d\xf0\x33\x82\xc5\xbe\x01\xa8\x1f\xe0\x0a\x36\xf3\xf4\xbb\x65\x41\x26\x3f\x80\x16\x27\xc4\x40\xe5\x08\x09\x71\x2b\x0c\xac\xe7\xc2\x17\xe6\xe5\x05\x1a\xf8\x1d\xe9\xbf\xec\x32\x04\xdc\xd6\x3c\x4f\x9a\x74\x10\x47"
#define ECDSA_521_SIGNATURE_RESULT "\x00\x00\x00\x4d\xe8\x26\xea\x70\x4a\xd1\x0b\xc0\xf7\x53\x8a\xf8\xa3\x84\x3f\x28\x4f\x55\xc8\xb9\x46\xaf\x92\x35\xaf\x5a\xf7\x4f\x2b\x76\xe0\x99\xe4\xbc\x72\xfd\x79\xd2\x8a\x38\x0f\x8d\x4b\x4c\x91\x9a\xc2\x90\xd2\x48\xc3\x79\x83\xba\x05\xae\xa4\x2e\x2d\xd7\x9f\xdd\x33\xe8\x00\x00\x00\x87\x48\x8c\x85\x9a\x96\xfe\xa2\x66\xea\x13\xbf\x6d\x11\x4c\x42\x9b\x16\x3b\xe9\x7a\x57\x55\x90\x86\xed\xb6\x4a\xed\x4a\x18\x59\x4b\x46\xfb\x9e\xfc\x7f\xd2\x5d\x8b\x2d\xe8\xf0\x9c\xa0\x58\x7f\x54\xbd\x28\x72\x99\xf4\x7b\x2f\xf1\x24\xaa\xc5\x66\xe8\xee\x3b\x43"

#ifdef ECDSA_521_SIGNATURE_66_MODULUS
	memcpy((char *)in, ECDSA_521_COORD, ECDSA_521_SIZE * 2);
	memcpy((char *)mod_ecc, ECDSA_521_MODULUS_66, ECDSA_521_SIZE_66);
	req->asym.modulus_size = ECDSA_521_SIZE_66;
	req->asym.modulus = mod_ecc;
	curve->a_sign = ECDSA_521_A_SIGN;
	memcpy((char *)a_ec, ECDSA_521_A_66, ECDSA_521_SIZE_66);
	curve->a = a_ec;
	memcpy((char *)n_ecdsa, ECDSA_521_ORDER, ECDSA_521_SIZE);
	curve->n_size = ECDSA_521_SIZE;
	curve->n = n_ecdsa;
	memcpy((char *)d_ecdsa, ECDSA_521_SECRET, ECDSA_521_SIZE);
	curve->d = d_ecdsa;
	memcpy((char *)k_ecdsa, ECDSA_521_RANDOM, ECDSA_521_SIZE);
	curve->k = k_ecdsa;
	memcpy((char *)e_ecdsa, ECDSA_521_HASH, ECDSA_521_SIZE);
	curve->e = e_ecdsa;

	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, in, out, ECDSA_521_SIZE * 2);

	perform_pka_test(req,
			 ECDSA_521_SIGNATURE_66_MODULUS,
			 ECDSA_521_SIGNATURE_RESULT);
#endif
}

void perform_arithmetic_tests(struct ccc_crypto_req *req)
{
	struct arithmetic *arith = &req->asym.arithmetic;

	req->asym.alg = PK_ARITHMETIC_ALG;
	arith->arith_op = ARITH_MUL;
/* #define ARITH_32_ARITH_MULT "Arithmetic multiplication on 32 bits" */
#define ARITH_32_ARITH_MULT_SIZE (sizeof(int))
#define ARITH_32_ARITH_MULT_A_FACTOR "\x00\x00\x00\x06"
#define ARITH_32_ARITH_MULT_B_FACTOR "\x00\x00\x00\x07"
#define ARITH_32_ARITH_MULT_RESULT "\x00\x00\x00\x00\x00\x00\x00\x2a"
#ifdef ARITH_32_ARITH_MULT
	memcpy((char *)arith_alpha, ARITH_32_ARITH_MULT_A_FACTOR, ARITH_32_ARITH_MULT_SIZE);
	arith->alpha = arith_alpha;
	arith->alpha_size = ARITH_32_ARITH_MULT_SIZE;
	memcpy((char *)arith_beta, ARITH_32_ARITH_MULT_B_FACTOR, ARITH_32_ARITH_MULT_SIZE);
	arith->beta = arith_beta;
	arith->beta_size = ARITH_32_ARITH_MULT_SIZE;

	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, NULL, out, ARITH_32_ARITH_MULT_SIZE * 2);

	perform_pka_test(req,
			 ARITH_32_ARITH_MULT,
			 ARITH_32_ARITH_MULT_RESULT);
#endif

#define ARITH_64_ARITH_MULT "Arithmetic multiplication on 64 bits"
#define ARITH_64_ARITH_MULT_SIZE (64 / 8)
#define ARITH_64_ARITH_MULT_A_FACTOR "\x80\x00\x00\x00\x00\x00\x00\x06"
#define ARITH_64_ARITH_MULT_B_FACTOR "\x80\x00\x00\x00\x00\x00\x00\x07"
#define ARITH_64_ARITH_MULT_RESULT "\x40\x00\x00\x00\x00\x00\x00\x06\x80\x00\x00\x00\x00\x00\x00\x2a"
#ifdef ARITH_64_ARITH_MULT
	memcpy((char *)arith_alpha, ARITH_64_ARITH_MULT_A_FACTOR, ARITH_64_ARITH_MULT_SIZE);
	arith->alpha = arith_alpha;
	arith->alpha_size = ARITH_64_ARITH_MULT_SIZE;
	memcpy((char *)arith_beta, ARITH_64_ARITH_MULT_B_FACTOR, ARITH_64_ARITH_MULT_SIZE);
	arith->beta = arith_beta;
	arith->beta_size = ARITH_64_ARITH_MULT_SIZE;

	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, NULL, out, ARITH_64_ARITH_MULT_SIZE * 2);

	perform_pka_test(req,
			 ARITH_64_ARITH_MULT,
			 ARITH_64_ARITH_MULT_RESULT);
#endif

#define ARITH_256_ARITH_MULT "Arithmetic multiplication on 256 bits"
#define ARITH_256_ARITH_MULT_SIZE (256 / 8)
#define ARITH_256_ARITH_MULT_A_FACTOR "\xdc\x93\xbc\x85\xed\x8b\x2d\xb8\x8f\x0b\x39\xe4\x2f\x7c\xcc\xf2\x32\x2a\xbc\x4b\x7c\x01\xe0\x62\x96\x4b\xe3\x1e\xb5\xd8\xc8\x0f"
#define ARITH_256_ARITH_MULT_B_FACTOR "\x43\x86\x6d\xe3\x4d\xcc\x5f\xd2\x58\x5c\x29\x05\x81\xe9\x3c\xba\xe4\x11\x7f\x4b\x9b\x54\x18\x54\x4d\x20\xdf\xc9\x14\xdc\x2b\xc1"
#define ARITH_256_ARITH_MULT_RESULT "\x3a\x2e\x7e\x5a\x7c\x93\x7c\xb5\xb4\x60\xa1\x80\x4a\x81\x1e\x89\xb6\xce\xe9\x35\x52\x5d\x9c\x6c\x50\xe5\x55\x07\x3d\x5d\x77\x1d\xd7\x07\xdd\x0b\x32\xad\xd9\xfd\x30\x49\xb8\x14\x89\xd4\x3a\x01\x6c\xfe\x65\xc9\xcb\x56\xe0\x3d\xc5\x4f\xbd\x66\x9a\xed\x58\x4f"
#ifdef ARITH_256_ARITH_MULT
	memcpy((char *)arith_alpha, ARITH_256_ARITH_MULT_A_FACTOR, ARITH_256_ARITH_MULT_SIZE);
	arith->alpha = arith_alpha;
	arith->alpha_size = ARITH_256_ARITH_MULT_SIZE;
	memcpy((char *)arith_beta, ARITH_256_ARITH_MULT_B_FACTOR, ARITH_256_ARITH_MULT_SIZE);
	arith->beta = arith_beta;
	arith->beta_size = ARITH_256_ARITH_MULT_SIZE;

	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, NULL, out, ARITH_256_ARITH_MULT_SIZE * 2);

	perform_pka_test(req,
			 ARITH_256_ARITH_MULT,
			 ARITH_256_ARITH_MULT_RESULT);
#endif

	arith->arith_op = MOD_RED;
#define ARITH_256_MOD_RED "Modular reduction of 256 bits"
#define ARITH_256_MOD_RED_SIZE (256 / 8)
#define ARITH_256_MOD_RED_A "\xdc\x93\xbc\x85\xed\x8b\x2d\xb8\x8f\x0b\x39\xe4\x2f\x7c\xcc\xf2\x32\x2a\xbc\x4b\x7c\x01\xe0\x62\x96\x4b\xe3\x1e\xb5\xd8\xc8\x10"
#define ARITH_256_MOD_RED_MODULUS "\xdc\x93\xbc\x85\xed\x8b\x2d\xb8\x8f\x0b\x39\xe4\x2f\x7c\xcc\xf2\x32\x2a\xbc\x4b\x7c\x01\xe0\x62\x96\x4b\xe3\x1e\xb5\xd8\xc8\x0f"
#define ARITH_256_MOD_RED_RESULT "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01"
#ifdef ARITH_256_MOD_RED
	memcpy((char *)arith_alpha, ARITH_256_MOD_RED_A, ARITH_256_MOD_RED_SIZE);
	arith->alpha = arith_alpha;
	arith->alpha_size = ARITH_256_MOD_RED_SIZE;
	memcpy((char *)mod_ecc, ARITH_256_MOD_RED_MODULUS, ARITH_256_MOD_RED_SIZE);
	req->asym.modulus_size = ARITH_256_MOD_RED_SIZE;
	req->asym.modulus = mod_ecc;

	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, NULL, out, ARITH_256_MOD_RED_SIZE);

	perform_pka_test(req,
			 ARITH_256_MOD_RED,
			 ARITH_256_MOD_RED_RESULT);
#endif

	arith->arith_op = MOD_ADD;
#define ARITH_256_MOD_ADD "Modular addition on 256 bits"
#define ARITH_256_MOD_ADD_SIZE (256 / 8)
#define ARITH_256_MOD_ADD_A "\xdc\x93\xbc\x85\xed\x8b\x2d\xb8\x8f\x0b\x39\xe4\x2f\x7c\xcc\xf2\x32\x2a\xbc\x4b\x7c\x01\xe0\x62\x96\x4b\xe3\x1e\xb5\xd8\xc8\x0f"
#define ARITH_256_MOD_ADD_B "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01"
#define ARITH_256_MOD_ADD_MODULUS "\xdc\x93\xbc\x85\xed\x8b\x2d\xb8\x8f\x0b\x39\xe4\x2f\x7c\xcc\xf2\x32\x2a\xbc\x4b\x7c\x01\xe0\x62\x96\x4b\xe3\x1e\xb5\xd8\xc8\x0f"
#define ARITH_256_MOD_ADD_RESULT "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01"
#ifdef ARITH_256_MOD_ADD
	memcpy((char *)arith_alpha, ARITH_256_MOD_ADD_A, ARITH_256_MOD_ADD_SIZE);
	arith->alpha = arith_alpha;
	arith->alpha_size = ARITH_256_MOD_ADD_SIZE;
	memcpy((char *)arith_beta, ARITH_256_MOD_ADD_B, ARITH_256_MOD_ADD_SIZE);
	arith->beta = arith_beta;
	arith->beta_size = ARITH_256_MOD_ADD_SIZE;
	memcpy((char *)mod_ecc, ARITH_256_MOD_ADD_MODULUS, ARITH_256_MOD_ADD_SIZE);
	req->asym.modulus_size = ARITH_256_MOD_ADD_SIZE;
	req->asym.modulus = mod_ecc;

	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, NULL, out, ARITH_256_MOD_ADD_SIZE);

	perform_pka_test(req,
			 ARITH_256_MOD_ADD,
			 ARITH_256_MOD_ADD_RESULT);
#endif

	arith->arith_op = ARITH_MUL__MOD_RED__MOD_ADD;
#define ARITH_64_AMMRMA "Arith mul then mod red then mod add on 64 bits"
#define ARITH_64_AMMRMA_SIZE (64 / 8)
#define ARITH_64_AMMRMA_A_FACTOR "\x80\x00\x00\x00\x00\x00\x00\x00"
#define ARITH_64_AMMRMA_B_FACTOR "\x80\x00\x00\x00\x00\x00\x00\x00"
#define ARITH_64_AMMRMA_C_ADDEND "\x00\x00\x00\x00\x00\x00\x00\x2a"
#define ARITH_64_AMMRMA_MODULUS "\x80\x00\x00\x00\x00\x00\x00\x00"
#define ARITH_64_AMMRMA_RESULT "\x00\x00\x00\x00\x00\x00\x00\x2a"
#ifdef ARITH_64_AMMRMA
	memcpy((char *)arith_alpha, ARITH_64_AMMRMA_A_FACTOR, ARITH_64_AMMRMA_SIZE);
	arith->alpha = arith_alpha;
	arith->alpha_size = ARITH_64_AMMRMA_SIZE;
	memcpy((char *)arith_beta, ARITH_64_AMMRMA_B_FACTOR, ARITH_64_AMMRMA_SIZE);
	arith->beta = arith_beta;
	arith->beta_size = ARITH_64_AMMRMA_SIZE;
	memcpy((char *)arith_gamma, ARITH_64_AMMRMA_C_ADDEND, ARITH_64_AMMRMA_SIZE);
	arith->gamma = arith_gamma;
	arith->gamma_size = ARITH_64_AMMRMA_SIZE;
	memcpy((char *)mod_ecc, ARITH_64_AMMRMA_MODULUS, ARITH_64_AMMRMA_SIZE);
	req->asym.modulus_size = ARITH_64_AMMRMA_SIZE;
	req->asym.modulus = mod_ecc;

	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, NULL, out, ARITH_64_AMMRMA_SIZE);

	perform_pka_test(req,
			 ARITH_64_AMMRMA,
			 ARITH_64_AMMRMA_RESULT);
#endif
}

void perform_rng_test(struct ccc_crypto_req *req, const char *title)
{
	void *ctx;
	int err;
	struct entry *entry;

	DUMP_ADDR(&req->scatter);

	ctx = ccc_crypto_init(req);
	if (!ctx) {
		TRACE_ERR("%s: ccc_crypto_init fails %x\n", __func__, ctx);
		return;
	}

	entry = &req->scatter.entries[0];	/* Consider 1st entry only */
	/* Use src as reference. */
	memset(entry->src, '\0', entry->size);
	memset(entry->dst, '\0', entry->size);

	err = ccc_crypto_run(ctx);
	if (err)
		TRACE_INFO("%s: ccc_crypto_run returns %d\n", __func__, err);

	HEXDUMP(entry->dst, entry->size);

	if (0 != memcmp(entry->dst, entry->src, entry->size))
		TRACE_ERR("%s [passed]\n", title);
	else {
		TRACE_ERR("%s [failed]\n", title);
		RNG_LOOP_ON_FAILURE();
	}
}


void perform_rng_tests(struct ccc_crypto_req *req)
{
	req->randomize.alg = RNG_ALG;

#define RANDOM_SIZE(n) (n / 8)
#define RANDOM_32_GENERATION "Generate a 32 bits random"

#ifdef RANDOM_32_GENERATION
	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, in, out, RANDOM_SIZE(32));
	perform_rng_test(req, RANDOM_32_GENERATION);
	perform_rng_test(req, RANDOM_32_GENERATION);
	perform_rng_test(req, RANDOM_32_GENERATION);
	perform_rng_test(req, RANDOM_32_GENERATION);
	perform_rng_test(req, RANDOM_32_GENERATION);
#endif

#define RANDOM_64_GENERATION "Generate a 64 bits random"

#ifdef RANDOM_64_GENERATION
	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, in, out, RANDOM_SIZE(64));
	perform_rng_test(req, RANDOM_64_GENERATION);
#endif

#define RANDOM_128_GENERATION "Generate a 128 bits random"

#ifdef RANDOM_128_GENERATION
	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, in, out, RANDOM_SIZE(128));
	perform_rng_test(req, RANDOM_128_GENERATION);
#endif

#define RANDOM_192_GENERATION "Generate a 192 bits random"

#ifdef RANDOM_192_GENERATION
	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, in, out, RANDOM_SIZE(192));
	perform_rng_test(req, RANDOM_192_GENERATION);
#endif

#define RANDOM_256_GENERATION "Generate a 256 bits random"

#ifdef RANDOM_192_GENERATION
	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, in, out, RANDOM_SIZE(256));
	perform_rng_test(req, RANDOM_256_GENERATION);
#endif
}

void perform_aes_ccm_tests(struct ccc_crypto_req *req)
{
#ifndef NO_AES_CCM_TESTS
	unsigned char *key_backup;

	req->sym.alg = AES_CCM_ALG;
	req->sym.dst = (unsigned char *)out;

#define AES_128_CCM_KEY "\x40\x41\x42\x43\x44\x45\x46\x47\x48\x49\x4a\x4b\x4c\x4d\x4e\x4f"
	memcpy((char *)aes_ae_key, AES_128_CCM_KEY, AES_KEYSIZE_128);

#define AES_128_CCM_NIST1_IV "\x10\x11\x12\x13\x14\x15\x16"
#define AES_128_CCM_NIST1_IV_SIZE 7
#define AES_128_CCM_NIST1_HEADER "\x00\x08\x00\x01\x02\x03\x04\x05\x06\x07"
#define AES_128_CCM_NIST1_HEADER_SIZE 10
#define AES_128_CCM_NIST1_PAYLOAD "\x20\x21\x22\x23"
#define AES_128_CCM_NIST1_PAYLOAD_SIZE 4
#define AES_128_CCM_NIST1_TAG "\x4d\xac\x25\x5d"
#define AES_128_CCM_NIST1_TAG_SIZE 4
#define AES_128_CCM_NIST1_ENCRYPT "AES-128-CCM-NIST-1 encryption"

#ifdef AES_128_CCM_NIST1_ENCRYPT
	req->sym.key_size = AES_KEYSIZE_128;
	key_backup = req->sym.key;
	req->sym.key = aes_ae_key;

	memset((unsigned char *)in, '\0', AES_BLOCK_SIZE * 2);
	memcpy((char *)in, AES_128_CCM_NIST1_HEADER,
	       AES_128_CCM_NIST1_HEADER_SIZE);
	req->sym.header = in;
	req->sym.header_size = AES_128_CCM_NIST1_HEADER_SIZE;

	memcpy((char *)(in + AES_BLOCK_SIZE), AES_128_CCM_NIST1_PAYLOAD,
	       AES_128_CCM_NIST1_PAYLOAD_SIZE);
		req->sym.payload = in + AES_BLOCK_SIZE;
	req->sym.payload_size = AES_128_CCM_NIST1_PAYLOAD_SIZE;
	req->sym.direction = AES_ENCRYPT;
	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, req->sym.payload, req->sym.dst,
			   req->sym.payload_size);

	memset((unsigned char *)iv, '\0', AES_IV_SIZE);
	memcpy((char *)iv, AES_128_CCM_NIST1_IV,
	       AES_128_CCM_NIST1_IV_SIZE);
	req->sym.iv = iv;
	req->sym.iv_size = AES_128_CCM_NIST1_IV_SIZE;

	memset((unsigned char *)tag, '\0', AES_GCM_TAG_SIZE);
	req->sym.tag = tag;
	req->sym.tag_size = AES_128_CCM_NIST1_TAG_SIZE;

	perform_aes_ae_test(req, AES_128_CCM_NIST1_ENCRYPT,
			    AES_128_CCM_NIST1_TAG,
			    AES_128_CCM_NIST1_TAG_SIZE);
#endif

#define AES_128_CCM_NIST1_DECRYPT "AES-128-CCM-NIST-1 decryption"

#ifdef AES_128_CCM_NIST1_DECRYPT
	/* Copy cypher text generated by previous test */
	memcpy((char *)(in + AES_BLOCK_SIZE),
	       (char *)(req->sym.dst + AES_BLOCK_SIZE),
	       AES_128_CCM_NIST1_PAYLOAD_SIZE);
	req->sym.payload = in + AES_BLOCK_SIZE;
	req->sym.payload_size = AES_128_CCM_NIST1_PAYLOAD_SIZE;
	req->sym.direction = AES_DECRYPT;
	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, req->sym.payload, req->sym.dst,
			   req->sym.payload_size);

	memset((unsigned char *)iv, '\0', AES_IV_SIZE);
	memcpy((char *)iv, AES_128_CCM_NIST1_IV,
	       AES_128_CCM_NIST1_IV_SIZE);
	req->sym.iv = iv;
	req->sym.iv_size = AES_128_CCM_NIST1_IV_SIZE;

	perform_aes_ccm_test(req, AES_128_CCM_NIST1_DECRYPT,
			     (req->sym.dst + AES_BLOCK_SIZE),
			     AES_128_CCM_NIST1_PAYLOAD,
			     AES_128_CCM_NIST1_PAYLOAD_SIZE,
			     AES_128_CCM_NIST1_TAG,
			     AES_128_CCM_NIST1_TAG_SIZE);
	req->sym.key = key_backup;
#endif

#define AES_128_CCM_NIST2_IV "\x10\x11\x12\x13\x14\x15\x16\x17"
#define AES_128_CCM_NIST2_IV_SIZE 8
#define AES_128_CCM_NIST2_HEADER "\x00\x10\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
#define AES_128_CCM_NIST2_HEADER_SIZE 18
#define AES_128_CCM_NIST2_PAYLOAD "\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f"
#define AES_128_CCM_NIST2_PAYLOAD_SIZE 16
#define AES_128_CCM_NIST2_TAG "\x1f\xc6\x4f\xbf\xac\xcd"
#define AES_128_CCM_NIST2_TAG_SIZE 6
#define AES_128_CCM_NIST2_ENCRYPT "AES-128-CCM-NIST-2 encryption"

#ifdef AES_128_CCM_NIST2_ENCRYPT
	req->sym.key_size = AES_KEYSIZE_128;
	key_backup = req->sym.key;
	req->sym.key = aes_ae_key;

	memset((unsigned char *)in, '\0', AES_BLOCK_SIZE * 3);
	memcpy((char *)in, AES_128_CCM_NIST2_HEADER,
	       AES_128_CCM_NIST2_HEADER_SIZE);
	req->sym.header = in;
	req->sym.header_size = AES_128_CCM_NIST2_HEADER_SIZE;

	memcpy((char *)(in + (AES_BLOCK_SIZE * 2)), AES_128_CCM_NIST2_PAYLOAD,
	       AES_128_CCM_NIST2_PAYLOAD_SIZE);
	req->sym.payload = in + (AES_BLOCK_SIZE * 2);
	req->sym.payload_size = AES_128_CCM_NIST2_PAYLOAD_SIZE;
	req->sym.direction = AES_ENCRYPT;
	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, req->sym.payload, req->sym.dst,
			   req->sym.payload_size);

	memset((unsigned char *)iv, '\0', AES_IV_SIZE);
	memcpy((char *)iv, AES_128_CCM_NIST2_IV,
	       AES_128_CCM_NIST2_IV_SIZE);
	req->sym.iv = iv;
	req->sym.iv_size = AES_128_CCM_NIST2_IV_SIZE;

	memset((unsigned char *)tag, '\0', AES_GCM_TAG_SIZE);
	req->sym.tag = tag;
	req->sym.tag_size = AES_128_CCM_NIST2_TAG_SIZE;

	perform_aes_ae_test(req, AES_128_CCM_NIST2_ENCRYPT,
			    AES_128_CCM_NIST2_TAG,
			    AES_128_CCM_NIST2_TAG_SIZE);
#endif

#define AES_128_CCM_NIST2_DECRYPT "AES-128-CCM-NIST-2 decryption"

#ifdef AES_128_CCM_NIST2_DECRYPT
	/* Copy cypher text generated by previous test */
	memcpy((char *)(in + (AES_BLOCK_SIZE * 2)),
	       (char *)(req->sym.dst + (AES_BLOCK_SIZE * 2)),
	       AES_128_CCM_NIST2_PAYLOAD_SIZE);
	req->sym.payload = in + (AES_BLOCK_SIZE * 2);
	req->sym.payload_size = AES_128_CCM_NIST2_PAYLOAD_SIZE;
	req->sym.direction = AES_DECRYPT;
	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, req->sym.payload, req->sym.dst,
			   req->sym.payload_size);

	memset((unsigned char *)iv, '\0', AES_IV_SIZE);
	memcpy((char *)iv, AES_128_CCM_NIST2_IV,
	       AES_128_CCM_NIST2_IV_SIZE);
	req->sym.iv = iv;
	req->sym.iv_size = AES_128_CCM_NIST2_IV_SIZE;

	perform_aes_ccm_test(req, AES_128_CCM_NIST2_DECRYPT,
			     (req->sym.dst + (AES_BLOCK_SIZE * 2)),
			     AES_128_CCM_NIST2_PAYLOAD,
			     AES_128_CCM_NIST2_PAYLOAD_SIZE,
			     AES_128_CCM_NIST2_TAG,
			     AES_128_CCM_NIST2_TAG_SIZE);

	req->sym.key = key_backup;
#endif

#define AES_128_CCM_NIST3_IV "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b"
#define AES_128_CCM_NIST3_IV_SIZE 12
#define AES_128_CCM_NIST3_HEADER "\x00\x14\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13"
#define AES_128_CCM_NIST3_HEADER_SIZE 22
#define AES_128_CCM_NIST3_PAYLOAD "\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f\x30\x31\x32\x33\x34\x35\x36\x37"
#define AES_128_CCM_NIST3_PAYLOAD_SIZE 24
#define AES_128_CCM_NIST3_TAG "\x48\x43\x92\xfb\xc1\xb0\x99\x51"
#define AES_128_CCM_NIST3_TAG_SIZE 8
#define AES_128_CCM_NIST3_ENCRYPT "AES-128-CCM-NIST-3 encryption"

#ifdef AES_128_CCM_NIST3_ENCRYPT
	req->sym.key_size = AES_KEYSIZE_128;
	key_backup = req->sym.key;
	req->sym.key = aes_ae_key;

	memset((unsigned char *)in, '\0', AES_BLOCK_SIZE * 4);
	memcpy((char *)in, AES_128_CCM_NIST3_HEADER,
	       AES_128_CCM_NIST3_HEADER_SIZE);
	req->sym.header = in;
	req->sym.header_size = AES_128_CCM_NIST3_HEADER_SIZE;

	memcpy((char *)(in + (AES_BLOCK_SIZE * 2)), AES_128_CCM_NIST3_PAYLOAD,
	       AES_128_CCM_NIST3_PAYLOAD_SIZE);
	req->sym.payload = in + (AES_BLOCK_SIZE * 2);
	req->sym.payload_size = AES_128_CCM_NIST3_PAYLOAD_SIZE;
	req->sym.direction = AES_ENCRYPT;
	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, req->sym.payload, req->sym.dst,
			   req->sym.payload_size);

	memset((unsigned char *)iv, '\0', AES_IV_SIZE);
	memcpy((char *)iv, AES_128_CCM_NIST3_IV,
	       AES_128_CCM_NIST3_IV_SIZE);
	req->sym.iv = iv;
	req->sym.iv_size = AES_128_CCM_NIST3_IV_SIZE;

	memset((unsigned char *)tag, '\0', AES_GCM_TAG_SIZE);
	req->sym.tag = tag;
	req->sym.tag_size = AES_128_CCM_NIST3_TAG_SIZE;

	perform_aes_ae_test(req, AES_128_CCM_NIST3_ENCRYPT,
			    AES_128_CCM_NIST3_TAG,
			    AES_128_CCM_NIST3_TAG_SIZE);
#endif

#define AES_128_CCM_NIST3_DECRYPT "AES-128-CCM-NIST-3 decryption"

#ifdef AES_128_CCM_NIST3_DECRYPT
	/* Copy cypher text generated by previous test */
	memcpy((char *)(in + (AES_BLOCK_SIZE * 2)),
	       (char *)(req->sym.dst + (AES_BLOCK_SIZE * 2)),
	       AES_128_CCM_NIST3_PAYLOAD_SIZE);
	req->sym.payload = in + (AES_BLOCK_SIZE * 2);
	req->sym.payload_size = AES_128_CCM_NIST3_PAYLOAD_SIZE;
	req->sym.direction = AES_DECRYPT;
	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, req->sym.payload, req->sym.dst,
			   req->sym.payload_size);

	memset((unsigned char *)iv, '\0', AES_IV_SIZE);
	memcpy((char *)iv, AES_128_CCM_NIST3_IV,
	       AES_128_CCM_NIST3_IV_SIZE);
	req->sym.iv = iv;
	req->sym.iv_size = AES_128_CCM_NIST3_IV_SIZE;

	perform_aes_ccm_test(req, AES_128_CCM_NIST3_DECRYPT,
			     (req->sym.dst + (AES_BLOCK_SIZE * 2)),
			     AES_128_CCM_NIST3_PAYLOAD,
			     AES_128_CCM_NIST3_PAYLOAD_SIZE,
			     AES_128_CCM_NIST3_TAG,
			     AES_128_CCM_NIST3_TAG_SIZE);

	req->sym.key = key_backup;
#endif

#define AES_128_CCM_NIST4_IV "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c"
#define AES_128_CCM_NIST4_IV_SIZE 13
#define AES_128_CCM_NIST4_HEADER_HEADER "\xff\xfe\x00\x01\x00\x00"
#define AES_128_CCM_NIST4_HEADER_HEADER_SIZE 6
#define AES_128_CCM_NIST4_HEADER_SIZE 65542
#define AES_128_CCM_NIST4_PAYLOAD "\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d\x3e\x3f"
#define AES_128_CCM_NIST4_PAYLOAD_SIZE 32
#define AES_128_CCM_NIST4_TAG "\xb4\xac\x6b\xec\x93\xe8\x59\x8e\x7f\x0d\xad\xbc\xea\x5b"
#define AES_128_CCM_NIST4_TAG_SIZE 14
#define AES_128_CCM_NIST4_ENCRYPT "AES-128-CCM-NIST-4 encryption"

#ifdef AES_128_CCM_NIST4_ENCRYPT
	unsigned char *p, c = '\0';
	unsigned int i;

	req->sym.key_size = AES_KEYSIZE_128;
	key_backup = req->sym.key;
	req->sym.key = aes_ae_key;

	/* Fill AData header */
	memcpy((char *)in, AES_128_CCM_NIST4_HEADER_HEADER,
	       AES_128_CCM_NIST4_HEADER_HEADER_SIZE);

	/* AData size is 64KB */
	p = in + AES_128_CCM_NIST4_HEADER_HEADER_SIZE;
	for (i = 0; i < (64 * 1024); i++)
		*(p++) = c++;
	req->sym.header = in;
	req->sym.header_size = AES_128_CCM_NIST4_HEADER_SIZE;

	memcpy((char *)(in + ((64 * 1024) + 8)), AES_128_CCM_NIST4_PAYLOAD,
	       AES_128_CCM_NIST4_PAYLOAD_SIZE);
	req->sym.payload = in + ((64 * 1024) + 8);
	req->sym.payload_size = AES_128_CCM_NIST4_PAYLOAD_SIZE;
	req->sym.direction = AES_ENCRYPT;
	ccc_scatter_init(&req->scatter);
	ccc_scatter_append(&req->scatter, req->sym.payload, req->sym.dst,
			   req->sym.payload_size);

	memset((unsigned char *)iv, '\0', AES_IV_SIZE);
	memcpy((char *)iv, AES_128_CCM_NIST4_IV,
	       AES_128_CCM_NIST4_IV_SIZE);
	req->sym.iv = iv;
	req->sym.iv_size = AES_128_CCM_NIST4_IV_SIZE;

	memset((unsigned char *)tag, '\0', AES_GCM_TAG_SIZE);
	req->sym.tag = tag;
	req->sym.tag_size = AES_128_CCM_NIST4_TAG_SIZE;

	perform_aes_ae_test(req, AES_128_CCM_NIST4_ENCRYPT,
			    AES_128_CCM_NIST4_TAG,
			    AES_128_CCM_NIST4_TAG_SIZE);
	req->sym.key = key_backup;
#endif

	/* Clean-up */
	req->sym.payload = NULL;
	req->sym.tag = NULL;

#endif /* !NO_AES_CCM_TESTS */
}

void ccc_tests(void)
{
	struct ccc_crypto_req req = {0,};

	TRACE_ERR("CCC tests begin...\n");

	req.control = CRYPTO_REQ_CONTROL_WORD;
	req.sym.alg = AES_NONE_ALG;
	req.asym.alg = PK_NONE_ALG;
	req.randomize.alg = RNG_NONE_ALG;
	req.sym.load_key = true;

	req.hash.digest = digest;
	TRACE_ERR(" HASH tests begin...\n");
	perform_hash_tests(&req, in, in);
	TRACE_ERR(" ... HASH tests end.\n");

	TRACE_ERR(" HASH and MOVE tests begin...\n");
	perform_hash_tests(&req, in, out);
	TRACE_ERR(" ... HASH and MOVE tests end.\n");

	TRACE_ERR(" HASH on large messages begin...\n");
	perform_hash_big_data_tests(&req, in, in);
	TRACE_ERR(" ... HASH on large messages end.\n");

	TRACE_ERR(" HASH intermixed computations begin...\n");
	perform_hash_intermix_tests(&req, in, in, in, in);
	TRACE_ERR(" ... HASH intermixed computations end.\n");

	req.hash.alg = HASH_NONE_ALG;
	TRACE_ERR(" MOVE test begin...\n");
	perform_move_tests(&req);
	TRACE_ERR(" ... MOVE test end.\n");

	TRACE_ERR(" AES cipher with Special key slot tests begin...\n");
	req.hash.alg = HASH_NONE_ALG;
	/* Assume that key slot content is zeroed. */
	req.sym.key_slot = 0;
	req.sym.key = NULL;

	TRACE_ERR("  with unprovisioned key slot, size defaults to 128b...\n");
	TRACE_ERR("   in place with unprovisioned key slot #%d...\n",
		  req.sym.key_slot);
	perform_aes_tests(&req, in, in);
	TRACE_ERR("   ... done.\n");

	TRACE_ERR("   with translation with unprovisioned key slot #%d...\n",
		  req.sym.key_slot);
	perform_aes_tests(&req, in, out);
	TRACE_ERR("   ... done.\n");

#if 0
	TRACE_ERR("   AE in place with unprovisioned key slot #%d...\n",
		  req.sym.key_slot);
	perform_aes_ae_big_data_tests(&req, in, in);
	TRACE_ERR("   ... done.\n");

	TRACE_ERR("   cipher then hash in place with unprovisioned key slot "
		  "#%d...\n", req.sym.key_slot);
	perform_aes_then_hash_tests(&req, in, in);
	TRACE_ERR("   ... done.\n");
	TRACE_ERR("  ... done.\n");
	req.hash.alg = HASH_NONE_ALG;
#endif

	memcpy(&(aes_key[0]), AES_KEY, sizeof(aes_key));
	req.sym.key = aes_key;
	req.sym.lock_special_keys = 0;
	TRACE_ERR("  in place with provisioned key slot #%d...\n",
		  req.sym.key_slot);
	perform_aes_tests(&req, in, in);
	TRACE_ERR("  ... done.\n");

	req.sym.key_slot = 1;
	TRACE_ERR("  in place with provisioned key slot #%d...\n",
		  req.sym.key_slot);
	perform_aes_tests(&req, in, in);
	TRACE_ERR("  ... done.\n");

	req.sym.key_slot = 0;
	req.sym.lock_special_keys = 1;
	TRACE_ERR("  in place with provisioned locked key slot #%d...\n",
		  req.sym.key_slot);
	perform_aes_tests(&req, in, in);
	TRACE_ERR("  ... done.\n");

	req.sym.key_slot = 1;
	TRACE_ERR("  in place with provisioned locked key slot #%d...\n",
		  req.sym.key_slot);
	perform_aes_tests(&req, in, in);
	TRACE_ERR("  ... done.\n");
	TRACE_ERR(" ... done.\n");

	TRACE_ERR(" AES cipher with General Purpose key slot tests begin...\n");
	req.sym.lock_special_keys = 0;
	for (req.sym.key_slot = 2; req.sym.key_slot < 16; req.sym.key_slot++) {
		TRACE_ERR("  in place with provisioned key slot #%d...\n",
			  req.sym.key_slot);
		perform_aes_tests(&req, in, in);
		TRACE_ERR("  ... done.\n");
	}
	req.sym.key_slot = 2;
	TRACE_ERR(" AES CCM mode cipher tests begin...\n");
	perform_aes_ccm_tests(&req);
	TRACE_ERR(" ... done.\n");
	TRACE_ERR(" ... AES ciphering tests end.\n");

	req.sym.alg = AES_NONE_ALG;

	TRACE_ERR(" RSA cipher tests begin...\n");
	perform_rsa_tests(&req);
	TRACE_ERR(" ... RSA ciphering tests end.\n");

	TRACE_ERR(" ECC cipher tests begin...\n");
	perform_ecc_tests(&req);
	TRACE_ERR(" ... ECC ciphering tests end.\n");

	TRACE_ERR(" Arithmetic tests begin...\n");
	perform_arithmetic_tests(&req);
	TRACE_ERR(" ... Arithmetic tests end.\n");

	req.asym.alg = PK_NONE_ALG;

	TRACE_ERR(" Random generation tests begin...\n");
	perform_rng_tests(&req);
	TRACE_ERR(" ... Random generation tests end.\n");

	req.randomize.alg = RNG_NONE_ALG;

	TRACE_ERR("... CCC tests end.\n");

	while (1)
		vTaskDelay(pdMS_TO_TICKS(5000));
}
