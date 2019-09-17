/**
 * @file sta_pka.c
 * @brief CCC pka driver header.
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: ADG-MID team
 */

#define DEV_NAME "pka"

/* RSA / ECC configurations. */
#define NO_COUNTERMEASURE 0
#define AGAINST_SPA 1
#define AGAINST_DPA 2

#define MONTY_PAR 0x11
#define OP_ID_SHIFT 23
#define COUNTERMEASURE_SHIFT 21
#define LENGTH_SHIFT 0

/*
 * Cryptographic materials memory region shared between all algorithms supported
 * by PKA channel. Set this region size to the maximum memory consumption of all
 * algorithms i.e. RSA.
 */
#define MAX_PKA_DATA_SIZE 2332
struct pka_shared_data {
	unsigned char blob[MAX_PKA_DATA_SIZE];
};

struct pka_context {
	struct ccc_channel channel;
	struct ccc_crypto_context *parent;
	struct pka_shared_data *shared_data;
	bool hash;
	struct pka_alg *alg;
	void *data;
	struct ccc_chunk chunks[MAX(RSA_NR_CHUNKS, ECC_NR_CHUNKS)];
	unsigned int nr_chunks;
};

struct pka_alg {
	void *(*crypto_init)(struct ccc_crypto_req *req,
			     struct pka_context *context);
	void (*program_prologue)(struct pka_context *context);
	void (*program)(struct pka_context *context);
	void (*program_epilogue)(struct pka_context *context);
	int (*post_process)(struct pka_context *context);
};

extern struct pka_alg pka_rsa_alg;
extern struct pka_alg pka_ecc_alg;
extern struct pka_alg pka_ecdsa_alg;
extern struct pka_alg pka_arithmetic_alg;

unsigned int get_dimension(unsigned char *buf, unsigned int size);
unsigned int size_in_bytes(unsigned int size);
unsigned int size_rounded_to_word_in_bytes(unsigned int size);
int append_bit_size(unsigned char **buf, unsigned char *bignum,
		    unsigned int size, unsigned int *bit_size);
int append_bignum(unsigned char **dst, unsigned char *src,
		  unsigned int src_size, unsigned int dst_size);
int append_align_bignum(unsigned char **dst, unsigned char *scr,
			unsigned int src_size, unsigned int dst_size);
struct operation get_opcode(int index, unsigned char code,
			    unsigned int nr_param, ...);
unsigned int get_countermeasure(void *arg);
unsigned int get_ecdsa_no_fault_value(void *arg);

static inline unsigned char *get_pka_shared_blob(struct pka_context *context)
{
	return &context->shared_data->blob[0];
}

#define ECC_SIZE_IN_WORDS MAX_ECC_SIZE_IN_WORDS
static inline bool ecc_check_modulus_size(unsigned int size)
{
	if (size > MAX_ECC_MODULUS_SIZE)
		return false;
	if (size <= sizeof(unsigned int))
		return false;
#ifdef REJECT_UNALIGNED_DATA
	if (!THIRTY_TWO_BITS_ALIGNED(size))
		return false;
#endif
	return true;
}

static inline bool ecc_check_scalar_size(unsigned int size)
{
	if (size > MAX_ECC_ORDER_SIZE)
		return false;
#ifdef REJECT_UNALIGNED_DATA
	if (!THIRTY_TWO_BITS_ALIGNED(size))
		return false;
#endif
	return true;
}

#define MOD_REDUCTION 0x0
#define MOD_ADDITION 0x2
#define ECDSA_SIGN 0x8
#define ECDSA_VERIFY 0xC
#define ARITH_MULTIPLICATION 0xD
#define ECC_MUL 0x16
#define ECC_MONTY_MUL 0x17
struct operation arith_get_opcode(int index, unsigned char code,
				  unsigned int nr_param, ...);
