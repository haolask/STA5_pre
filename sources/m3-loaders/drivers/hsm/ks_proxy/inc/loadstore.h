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
 * @file    loadstore.h
 * @brief   Flash load/store abstraction header file.
 *
 * @addtogroup LOADSTORE
 * @{
 */

#ifndef _LOADSTORE_H_
#define _LOADSTORE_H_

#include "lspayload.h"
#include "rpmx_common.h"

/*
 * Number of magic numbers to be prefixed in flash block.
 */
#define LS_NUM_MAGIC	4

#define LS_MAGIC0	0xF34F5A00
#define LS_MAGIC1	0x0AAF055F
#define LS_MAGIC2	0x137FF731
#define LS_MAGIC3	0xEC8008CE

#define LS_PAYLOAD_SIZE sizeof(struct lspayload_t)

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/
/*
 * Type of a flash block.
 */
struct lsblock_t {
	/* Magic numbers for block validation. */
	uint32_t magic[LS_NUM_MAGIC];
	/* Block instance number. */
	uint32_t instance;
	/* Size of the payload. */
	uint32_t size;
	/*
	 * Block payload as defined by upper layers.
	 * @note     It is opaque from the load/store unit perspective.
	 */
	uint8_t payload[LS_PAYLOAD_SIZE];
	/* Block CRC32 calculated and checked by the load/store unit. */
	uint32_t crc;
};

/*
 * Type of an error.
 */
enum lserror_t {
	LS_NOERROR        = 0,
	LS_NOTINIT        = 1,
	LS_WARNING        = 2,
	LS_NOTFOUND       = 3,
	LS_FLASH_FAILURE  = 4,
	LS_INTERNAL       = 5
};

/*
 * Type of an error.
 */
enum ks_status_t {
	KS_NOTVALID       = 0,
	KS_LOADED         = 1,
	KS_VALID          = 2
};

/**
 * @brief  This routine discovers the key storage device
 * @param  ctx: NVM context
 * @param  addr: address of KS in NVM storage
 * @return 0 if OK else error status
 */
enum RPMx_ErrTy ls_discover(t_nvm_ctx *ctx, uint32_t addr);

/**
 * @brief  This routine stores a new blob to the non volatile external device
 * @param  ctx: NVM context
 * @param  addr: address belonging to erasable block
 * @param  payload: pointer to the key storage source buffer
 * @param  size: size of the payload
 * @return 0 if OK else error status
 */
enum RPMx_ErrTy ls_store(t_nvm_ctx *ctx, uint32_t addr,
			 void *payload, uint32_t size);

/**
 * @brief  This routine copies the latest valid instance
 *         to the destination payload buffer
 * @param  ctx: NVM context
 * @param  payload: pointer to the key storage destination buffer
 * @param  size: payload size
 * @return 0 if OK else error status
 */
enum RPMx_ErrTy ls_load(t_nvm_ctx *ctx, uint32_t addr,
			void *payload, uint32_t size);

/**
 * @brief  This routine copies the latest valid instance
 *         to the destination payload buffer
 * @param  ctx: NVM context
 * @param  payload: pointer to the key storage destination buffer
 * @param  size: payload size
 * @return 0 if OK else error status
 */
enum RPMx_ErrTy ls_load(t_nvm_ctx *ctx, uint32_t addr,
			void *payload, uint32_t size);

/**
 * @brief  This routine read NVM key blob with given id
 * @param  ctx: NVM context
 * @param  id: EHSM NVM ID
 * @param  payload: pointer to the key storage destination buffer
 * @param  p_size: IN: Max size of the payload,
 *		   OUT: actual size read from ls_header
 * @return 0 if OK else error status
 */
enum RPMx_ErrTy ls_read(t_nvm_ctx *ctx, uint32_t id,
			void *payload, uint32_t *p_size);
/**
 * @brief  This routine writes given NVM key blob with given id
 * @param  ctx: NVM context
 * @param  id: EHSM NVM ID
 * @param  payload: pointer to the key payload source buffer
 * @param  size: size of the payload
 * @return 0 if OK else error status
 */
enum RPMx_ErrTy ls_write(t_nvm_ctx *ctx, uint32_t id,
			 const void *payload, uint32_t size);

/**
 * @brief  This routine writes given NVM key blob with given id and updated KVT
 * @param  ctx: NVM context
 * @param  key_id: EHSM NVM Key ID
 * @param  kvt_payload: pointer to the kvt payload source buffer (id = 0)
 * @param  kvt_size: size of the kvt payload
 * @param  key_payload: pointer to the key payload source buffer
 * @param  key_size: size of the key payload
 * @return 0 if OK else error status
 */
enum RPMx_ErrTy ls_write_key_and_kvt(t_nvm_ctx *ctx, uint32_t key_id,
				     const void *kvt_payload, uint32_t kvt_size,
				     const void *key_payload, uint32_t key_size);

/**
 * @brief  This routine turns off the key storage framework
 * @param  ctx: NVM context
 * @return : None
 */
void ls_deinit(t_nvm_ctx *ctx);

/**
 * @brief  This routine erase the latest valid instance
 * @param  ctx: NVM context
 * @param  addr: NVM address to clean
 * @return 0 if OK else error status
 */
enum RPMx_ErrTy ls_erase(t_nvm_ctx *ctx, uint32_t addr);

/**
 * @brief  This routine dumps the content of the key storage framework
 *         without decrypting it
 * @param  ctx: NVM context
 * @return 0 if OK else error status
 */
enum RPMx_ErrTy ls_dump_ks_area(t_nvm_ctx *ctx);

#endif /* _LOADSTORE_H_ */
