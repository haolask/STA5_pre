/**
 * @file hmacsha256.h
 * @brief This file provides HMAC Sha256 algo intefaces
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: ADG-MID team
 */

#define SHA256_DIGEST_SIZE (256 / 8)
#define SHA256_BLOCK_SIZE  (512 / 8)

typedef struct {
	unsigned int tot_len;
	unsigned int len;
	unsigned char block[2 * SHA256_BLOCK_SIZE];
	uint32_t h[8];
} sha256_ctx;

void sha256_init(sha256_ctx *ctx);

void sha256_update(sha256_ctx *ctx, const unsigned char *message,
                   unsigned int len);

void sha256_final(sha256_ctx *ctx, unsigned char *digest);

void sha256(const unsigned char *message, unsigned int len,
	    unsigned char *digest);
