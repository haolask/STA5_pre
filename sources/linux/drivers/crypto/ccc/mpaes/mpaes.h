/*
 * ST C3 Channel Controller v3.0
 *
 * Author: Gerald Lejeune <gerald.lejeune@st.com>
 *
 * Copyright (C) 2017 STMicroelectronics Limited
 *
 * This code is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#ifndef _MPAES_H_
#define _MPAES_H_

#define MAX_DATA_SIZE 65520

#define IV_SIZE AES_BLOCK_SIZE
#define AES_ENCRYPT 0
#define AES_DECRYPT 1
#define MPAES_MAX_KEY_SIZE 16
#define UNKNOWN_MODE 0
#define ECB_MODE 0x0a
#define CBC_MODE 0x0e
#define GCM_MODE 0x1b

struct mpaes_data {
	unsigned short gp_key_slots;
	struct dma_pool *pool;
};

enum qualifier {
	ENCRYPT = AES_ENCRYPT,
	DECRYPT = AES_DECRYPT,
	HEADER
};

struct mpaes_context {
	struct crypto_tfm *tfm;
	struct ablkcipher_request *ablk_req;
	struct ablkcipher_walk *walk;
	struct aead_request *aead_req;
	struct mpaes_alg *alg;
	enum qualifier direction;
	struct ccc_dma key;
	struct ccc_dma iv;
	struct ccc_dma tag;
	struct ccc_dma final;
	struct list_head chunks;
	u32 blocksize;
	struct ccc_channel *channel;
	struct ccc_dispatcher *dispatcher;
	struct device *dma_owner;
};

struct mpaes_alg {
	unsigned int mode;
	void (*program_prolog)(struct mpaes_context *);
	bool (*need_iv)(struct mpaes_context *);
	int (*program_crypto_instructions)(struct mpaes_context *);
	struct operation (*get_opcode)(int, unsigned char, unsigned short);
	unsigned char (*get_usequence)(struct mpaes_context *);
	void (*program_epilog)(struct mpaes_context *);
	int (*postprocess)(struct mpaes_context *);
};

static inline struct mpaes_data *get_mpaes_data(struct mpaes_context *context)
{
	return (struct mpaes_data *)ccc_get_channel_data(context->channel);
}

static inline bool produce_tag(struct mpaes_alg *alg)
{
	return alg->mode == GCM_MODE;
}

#define OP_ID_LSB 22
#define M_LSB 16
#define N_LSB 0

#define SETUP BIT(3)
#define EXEC (SETUP | BIT(2))
#define EXEC_P (EXEC | 0)
#define EXEC_D (EXEC | 1)
#define EXEC_S (EXEC | 2)
#define EXEC_PD (EXEC | 1)
#define EXEC_PS (EXEC | 2)
#define EXEC_SD (EXEC | 3)
#define EXEC_PSD (EXEC | 3)

#define HALT 0
#define KEY_LOAD 0x02
#define KEY_LOAD_SCHED 0x04

struct operation mpaes_get_opcode(int, unsigned char, unsigned short);
void mpaes_program_crypto_instruction(struct mpaes_context *,
				      struct ccc_chunk *, bool *);
int mpaes_program_crypto_instructions(struct mpaes_context *);
int mpaes_set_key(struct mpaes_context *, const u8 *, unsigned int);
int mpaes_xcrypt(struct mpaes_context *, unsigned int);

#endif /* _MPAES_H_ */
