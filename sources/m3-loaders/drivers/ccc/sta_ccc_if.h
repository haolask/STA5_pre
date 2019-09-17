/**
 * @file sta_ccc_if.c
 * @brief CCC driver interface header.
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: ADG-MID team
 */

#ifndef _CCC_IF_H_
#define _CCC_IF_H_

#define CRYPTO_REQ_CONTROL_WORD 0xe0e6233f

#define RNG_NONE_ALG 0xffffffff
#define RNG_ALG 1

#define AES_NONE_ALG 0xffffffff
#define AES_LOAD_ALG 0x04
#define AES_ECB_ALG 0x0a
#define AES_CBC_ALG 0x0e
#define AES_CMAC_ALG 0x11
#define AES_GCM_ALG 0x1b
#define AES_CCM_ALG 0x22
#define AES_KEYSIZE_128 16
#define AES_KEYSIZE_192 24
#define AES_KEYSIZE_256 32
#define AES_ENCRYPT 0
#define AES_DECRYPT 1

#define HASH_NONE_ALG 0xffffffff
#define HASH_VERSION_MASK 0xffff
#define HASH_VERSION_SHIFT 16
#define HASH_ALG_SHIFTED_MASK ~((HASH_VERSION_MASK << HASH_VERSION_SHIFT))
#define UH_VERSION 1
#define UH_ALG(a) ((a) | (UH_VERSION << HASH_VERSION_SHIFT))
#define HASH_MD5_ALG UH_ALG(0)
#define HASH_SHA_1_ALG UH_ALG(1)
#define HASH_SHA_256_ALG UH_ALG(2)
#define HASH_SHA_224_ALG UH_ALG(3)
#define UH2_VERSION 2
#define UH2_ALG(a) ((a) | (UH2_VERSION << HASH_VERSION_SHIFT))
#define HASH_SHA_384_ALG UH2_ALG(0)
#define HASH_SHA_512_ALG UH2_ALG(1)

#define PK_NONE_ALG 0xffffffff
#define PK_RSA_ALG 0
#define PK_ECC_ALG 1
#define PK_ECDSA_ALG 2
#define PK_ARITHMETIC_ALG 3
#define MAX_RSA_SIZE_IN_BITS 3072
#define MAX_RSA_SIZE_IN_BYTES SIZE_IN_BYTES(MAX_RSA_SIZE_IN_BITS)
#define MAX_RSA_SIZE_IN_WORDS SIZE_IN_WORDS(MAX_RSA_SIZE_IN_BITS)
#define MAX_RSA_MODULUS_SIZE (MAX_RSA_SIZE_IN_WORDS * sizeof(unsigned int))
#define MAX_RSA_EXPONENT_SIZE (MAX_RSA_SIZE_IN_WORDS * sizeof(unsigned int))
#define MAX_ECC_SIZE_IN_BITS 521
#define MAX_ECC_SIZE_IN_BYTES SIZE_IN_BYTES(MAX_ECC_SIZE_IN_BITS)
#define MAX_ECC_SIZE_IN_WORDS SIZE_IN_WORDS(MAX_ECC_SIZE_IN_BITS)
#define MAX_ECC_MODULUS_SIZE (MAX_ECC_SIZE_IN_WORDS * sizeof(unsigned int))
#define MAX_ECC_EXPONENT_SIZE (MAX_ECC_SIZE_IN_WORDS * sizeof(unsigned int))
#define MAX_ECC_ORDER_SIZE (MAX_ECC_SIZE_IN_WORDS * sizeof(unsigned int))
#define ECC_PLUS_SIGN 0UL
#define ECC_MINUS_SIGN 0xffffffff

#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#define ROUND_UP(n, d) ((d) * DIV_ROUND_UP(n, d))
#define SIZE_IN_BYTES(size_in_bits) DIV_ROUND_UP(size_in_bits, 8)
#define SIZE_IN_WORDS(size_in_bits) DIV_ROUND_UP(SIZE_IN_BYTES(size_in_bits), \
						 sizeof(unsigned int))

struct curve {
	unsigned int a_sign;
	/* a must be modulus_size long. */
	unsigned char *a;
	/* q must be twice modulus_size long. */
	unsigned char *q;
	unsigned int n_size;
	/* n, d, k, e, r and s must be n_size long. */
	unsigned char *n;
	unsigned char *d;
	unsigned char *k;
	unsigned char *e;
	unsigned char *r;
	unsigned char *s;
};

enum arithmetic_operation {
	/*
	 * alpha * beta
	 * alpha and beta must be of the same size.
	 */
	ARITH_MUL = 1,
	/*
	 * alpha mod modulus
	 * alpha and modulus size must be set.
	 */
	MOD_RED,
	/*
	 * (alpha + beta) mod modulus
	 * alpha and beta must be of modulus size.
	 */
	MOD_ADD,
	/*
	 * (((alpha * beta) mod modulus) + gamma) mod modulus
	 * alpha, beta and gamma must be of modulus size.
	 */
	ARITH_MUL__MOD_RED__MOD_ADD
};

struct arithmetic {
	enum arithmetic_operation arith_op;
	unsigned char *alpha;
	unsigned int alpha_size;
	unsigned char *beta;
	unsigned int beta_size;
	unsigned char *gamma;
	unsigned int gamma_size;
};

struct symmetric_crypto {
	unsigned int alg;
	unsigned int direction;
	unsigned int key_slot;
	unsigned int key_size;
	unsigned int load_key;
	unsigned int lock_special_keys;
	unsigned char *key;
	unsigned char *iv;
	unsigned char *payload;
	unsigned char *tag;
	unsigned char *header;
	unsigned int header_size;
	unsigned int iv_size;
	unsigned int payload_size;
	unsigned int tag_size;
	unsigned int header_discard;
	unsigned int extra_bit_size;
	unsigned char *dst;
};

struct asymmetric_crypto {
	unsigned int alg;
	unsigned int modulus_size;
	unsigned char *modulus;
	union {
		unsigned int exponent_size;
		unsigned int k_size;
	};
	union {
		unsigned char *exponent;
		unsigned char *k;
	};
	struct curve curve;
	struct arithmetic arithmetic;
};

struct hash {
	unsigned int alg;
	unsigned char *digest;
};

struct randomize {
	unsigned int alg;
};

struct entry {
	void *src;
	void *dst;
	unsigned int size;
};

#define MAX_SCATTER_ENTRIES 4
struct ccc_scatter {
	unsigned int control;
	struct entry entries[MAX_SCATTER_ENTRIES];
	unsigned int nr_entries;
	unsigned int nr_bytes;
	unsigned int max_chunk_size;
};

struct ccc_crypto_req {
	unsigned int control;
	struct symmetric_crypto sym;
	struct asymmetric_crypto asym;
	struct hash hash;
	struct randomize randomize;
	struct ccc_scatter scatter;
};

struct ccc_crypto_wrap {
	unsigned int alg;
	unsigned int direction;
	unsigned int key_slot;
	void *key;
	unsigned int key_size;
	void *iv;
	unsigned int iv_size;
	void *header;
	unsigned int header_size;
	void *payload;
	unsigned int payload_size;
	unsigned int extra_bit_size;
	void *tag;
	unsigned int tag_size;
	void *dst;
	struct ccc_scatter scatter;
};

int ccc_init(void);
void *ccc_crypto_init(struct ccc_crypto_req *req);
int ccc_crypto_update(void *crypto_context, struct ccc_scatter *scatter);
int ccc_crypto_finish(void *crypto_context);
int ccc_crypto_run(void *crypto_context);
void ccc_deinit(void);
/* Helpers */
void ccc_crypto_sym_wrap_init(void *);
int ccc_crypto_sym_wrap(void *);
int ccc_trng_read_data(unsigned int, unsigned int *);
int ccc_hash_req_init(struct ccc_crypto_req *req, unsigned int alg);

int ccc_scatter_init(struct ccc_scatter *scatter);
int ccc_scatter_append(struct ccc_scatter *scatter, void *src, void *dst,
		       unsigned int size);
int ccc_scatter_deinit(struct ccc_scatter *scatter);
bool ccc_scatter_is_list_full(struct ccc_scatter *scatter);

unsigned char ccc_memcmp_tac(const unsigned char *, const unsigned char *,
			     unsigned int);
#endif /* _CCC_IF_H_ */
