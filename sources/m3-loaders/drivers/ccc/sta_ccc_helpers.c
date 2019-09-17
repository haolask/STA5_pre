/**
 * @file sta_ccc_helpers.c
 * @brief CCC driver helpers.
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: ADG-MID team
 */

#include <errno.h>
#include <string.h>

#include "sta_ccc_plat.h"
#include "sta_ccc_osal.h"
#include "sta_ccc_if.h"
#include "sta_ccc.h"

#define DEV_NAME "ccc"
#define PREFIX DEV_NAME ": "

/* Timing-attack resistant memcmp implementation. */
unsigned char ccc_memcmp_tac(const unsigned char *s1, const unsigned char *s2,
			     unsigned int n)
{
	unsigned int i;
	unsigned char ret = 0;

	for (i = n; i--;)
		ret |= *s1++ ^ *s2++;

	return ret;
}

int ccc_scatter_init(struct ccc_scatter *scatter)
{
	if (!scatter)
		return -EINVAL;
	scatter->control = SCATTER_CONTROL_WORD;
	scatter->nr_entries = 0;
	scatter->nr_bytes = 0;
	scatter->max_chunk_size = MAX_CHUNK_SIZE;
	return 0;
}

int ccc_scatter_append(struct ccc_scatter *scatter, void *src, void *dst,
		       unsigned int size)
{
	struct entry *entry;

	if (!scatter)
		return -EINVAL;
	if (scatter->control != SCATTER_CONTROL_WORD)
		return -ENOENT;
	if (scatter->nr_entries >= MAX_SCATTER_ENTRIES)
		return -ENOENT;
	entry = &scatter->entries[scatter->nr_entries++];
	entry->src = src;
	entry->dst = dst;
	entry->size = size;
	scatter->nr_bytes += size;
	return 0;
}

int ccc_scatter_deinit(struct ccc_scatter *scatter)
{
	if (!scatter)
		return -EINVAL;
	memset(scatter, 0, sizeof(struct ccc_scatter));
	return 0;
}

bool ccc_scatter_is_list_full(struct ccc_scatter *scatter)
{
	ASSERT(scatter);
	ASSERT(scatter->control == SCATTER_CONTROL_WORD);
	if (scatter->nr_entries >= MAX_SCATTER_ENTRIES)
		return true;
	return false;
}

void ccc_crypto_sym_wrap_init(void *arg)
{
	struct ccc_crypto_wrap *wrap = (struct ccc_crypto_wrap *)arg;

	memset(wrap, 0, sizeof(struct ccc_crypto_wrap));
}

int ccc_crypto_sym_wrap(void *arg)
{
	struct ccc_crypto_wrap *wrap = (struct ccc_crypto_wrap *)arg;
	struct ccc_crypto_req req = {0,};
	void *ccc_ctx;
	int ret = -EINVAL;

	req.control = CRYPTO_REQ_CONTROL_WORD;
	req.hash.alg = HASH_NONE_ALG;
	req.randomize.alg = RNG_NONE_ALG;
	req.asym.alg = PK_NONE_ALG;
	req.sym.alg = wrap->alg;
	if (wrap->alg == AES_LOAD_ALG)
		/* Force decryption direction for key loading request */
		req.sym.direction = AES_DECRYPT;
	else
		req.sym.direction = wrap->direction;
	req.sym.key_slot = wrap->key_slot;
	req.sym.key_size = wrap->key_size;
	req.sym.key = wrap->key;
	if (wrap->key)
		req.sym.load_key = true;
	else
		req.sym.load_key = false;
	req.sym.lock_special_keys = false;
	req.sym.header = wrap->header;
	req.sym.header_size = wrap->header_size;
	req.sym.header_discard = true;	/* discard header output */
	req.sym.iv = wrap->iv;
	req.sym.iv_size = wrap->iv_size;
	req.sym.payload = wrap->payload;
	req.sym.payload_size = wrap->payload_size;
	req.sym.extra_bit_size = wrap->extra_bit_size;
	req.sym.tag = wrap->tag;
	req.sym.tag_size = wrap->tag_size;
	req.sym.dst = wrap->dst;
	if (wrap->scatter.control == SCATTER_CONTROL_WORD) {
		req.scatter = wrap->scatter;
	} else {
		ccc_scatter_init(&req.scatter);
		ccc_scatter_append(&req.scatter, wrap->payload, wrap->dst,
				   wrap->payload_size);
	}

	ccc_ctx = ccc_crypto_init(&req);
	if (ccc_ctx)
		ret = ccc_crypto_run(ccc_ctx);

	return ret;
}

#define RANDOM_SIZE(n) ((n) / 8)
int ccc_trng_read_data(unsigned int size, unsigned int *data)
{
	void *ccc_ctx;
	struct ccc_crypto_req req = {0, };
	int ret = -EINVAL;

	req.control = CRYPTO_REQ_CONTROL_WORD;
	ccc_scatter_init(&req.scatter);
	ccc_scatter_append(&req.scatter, data, data, RANDOM_SIZE(size));
	req.hash.alg = HASH_NONE_ALG;
	req.sym.alg = AES_NONE_ALG;
	req.asym.alg = PK_NONE_ALG;
	req.randomize.alg = RNG_ALG;

	ccc_ctx = ccc_crypto_init(&req);
	if (ccc_ctx)
		ret = ccc_crypto_run(ccc_ctx);

	return ret;
}

int ccc_hash_req_init(struct ccc_crypto_req *req, unsigned int alg)
{
	if (!req || !hash_is_alg_supported(alg))
		return -EINVAL;
	memset(req, 0, sizeof(*req));
	req->control = CRYPTO_REQ_CONTROL_WORD;
	req->sym.alg = AES_NONE_ALG;
	req->randomize.alg = RNG_NONE_ALG;
	req->asym.alg = PK_NONE_ALG;
	req->hash.alg = alg;
	return 0;
}
