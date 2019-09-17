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
 * @file    loadstore.c
 * @brief	Flash load/store abstraction source file.
 * @details	Set of functions to load and store key image.
 *
 *
 * @addtogroup LOADSTORE
 * @{
 */

#include <string.h>

#include "sta_common.h"
#include "utils.h"
#include "FreeRTOS.h" /* For malloc/free */
#include "sta_crc.h"
#include "sta_image.h"
#include "cmd_mailbox.h"
#include "loadstore.h"

#define NEW_KS_FORCED 0 /* Set to 1 use new_KS on cut1 for development */
//#define DEBUG 2 /* Uncomment for traces, 1 normal, 2 for full data traces */
//#define LS_TESTS /* Uncomment to enable TESTS */

#ifdef DEBUG /* Define it for debug traces */
#define trace_debug trace_printf
#else
#define trace_debug(format, ...) (void)0
#endif

#if KS_MEMTYPE == NVM_SQI
#define LS_SECTOR_SIZE		SQI_FIFO_SIZE
#define LS_BLOCKERASE_SIZE	(64 * 1024)
#elif KS_MEMTYPE == NVM_MMC
#define LS_SECTOR_SIZE		MMC_BLOCK_SIZE
#else
#define LS_SECTOR_SIZE		1
#endif

#define KVT_ID			0

enum ls_magic_t {
#if KS_MEMTYPE == NVM_MMC
	LS_INVALID_ELM = 0xFFFFFFFF,
	LS_FREE_ELM = 0x0,
	LS_ERASED_BLOCK = 0x0,
#else
	LS_INVALID_ELM = 0x0,
	LS_FREE_ELM = 0xFFFFFFFF,
	LS_ERASED_BLOCK = 0xFFFFFFFF,
#endif
	LS_VALID_ELM = 0xAA55AA55,
	LS_GARBAGING_BLOCK = 0xFF55FF55,
};

/*
 * NVM Key storage element header
 */
struct ls_header_elm_t {
	enum ls_magic_t magic; /* See above definition */
	uint32_t id;    /* EHSM given identifier */
	uint32_t write_count; /* To evince old element if duplicated */
	uint32_t payload_size;
	uint32_t size_in_sectors; /* Size in sectors (LS_SECTOR_SIZE)
				   * header included */
};

#define BUF_MAX_SIZE	(MAX_PAYLOAD_DATA + sizeof(struct ls_header_elm_t))

/*===========================================================================*/
/* Module local variables.                                                   */
/*===========================================================================*/
static enum ks_status_t ks_state;
static struct lsblock_t ks_buff;

/**
 * @brief  This routine invalidate a previous element with given offset in NVM
 * @param  ctx: NVM context
 * @param  offset: offset in NVM of previous content to invalidate
 * @return 0 if OK else error status
 */
static enum RPMx_ErrTy ls_invalidate(t_nvm_ctx *ctx, uint32_t offset)
{
	int ret = 0;

	/* Really invalidate previous blob */
#if KS_MEMTYPE == NVM_SQI
	struct ls_header_elm_t ls_header;

	sqi_read(ctx, offset, &ls_header, sizeof(ls_header));
	ls_header.magic = LS_INVALID_ELM;
	ret = sqi_write(ctx, offset, (uint32_t *)&ls_header, sizeof(ls_header));
#elif KS_MEMTYPE == NVM_MMC
	uint32_t sector_count;
	uint32_t sector;
	struct ls_header_elm_t ls_header;

	ret = sdmmc_read(ctx, offset, &ls_header, sizeof(ls_header));
	sector_count = ls_header.size_in_sectors;
	for (sector = 0; sector < sector_count && ret >= 0; sector++) {
		ret = sdmmc_read(ctx, offset + sector,
				 &ls_header, sizeof(ls_header));
		if (ret < 0)
			break;
		ls_header.magic = LS_INVALID_ELM;
		/*
		 * Update number of following invalidated sectors,
		 * in case the previous one is re-used,
		 * the header will be consistent
		 */
		ls_header.size_in_sectors = sector_count - sector;
		ret = sdmmc_write(ctx, offset + sector,
				  &ls_header, sizeof(ls_header));
	}
#endif

	if (ret < 0)
		return RPMx_LS_DEVICE_FAILURE;
	else
		return RPMx_OK;
}

/**
 * @brief  Find in NVM previous valid id offset and next offset to write
 * @param  ctx: NVM context
 * @param  id: EHSM NVM ID
 * @param  size: Size of the lement to save in KS NVM
 * @param  free_offset [output]: next offset to write
 *				 or 0 if KS NVM full or error
 * @return offset where previous valid id is found or 0 if not found
 */
static uint32_t find_valid_id_and_new_offset(t_nvm_ctx *ctx,
					     uint32_t id,
					     uint32_t size,
					     uint32_t *free_offset)
{
	uint32_t ks_size, len;
	uint32_t ks_offset, offset, offset_id;
	uint32_t write_count = 0;
#if KS_MEMTYPE == NVM_SQI
	uint32_t active_block_offset, free_block_offset;
#endif
	struct ls_header_elm_t ls_header;

	offset_id = 0;
	if (free_offset)
		*free_offset = 0;
	if (get_partition_info(KEY_STORAGE_ID, &ks_offset, &ks_size))
		return 0;
#if KS_MEMTYPE == NVM_SQI
	/* For SQI we use 2 erase blocks size and only one for data */
	TRACE_ASSERT(ks_size == (2 * LS_BLOCKERASE_SIZE));
	active_block_offset = 0;
	free_block_offset = 0;
	/* Search the current active block and free block */
	for (offset = ks_offset; offset < (ks_offset + ks_size);
	     offset += LS_BLOCKERASE_SIZE) {
		sqi_read(ctx, offset, &ls_header, sizeof(ls_header));
		if (ls_header.magic == LS_ERASED_BLOCK ||
		    ls_header.magic == LS_GARBAGING_BLOCK)
			free_block_offset = offset;
		else /* Valid block */
			active_block_offset = offset;
	}
	if (active_block_offset == 0 || free_block_offset == 0) {
		/* All blocks erased or all used => use first one */
		active_block_offset = ks_offset;
		free_block_offset = ks_offset + LS_BLOCKERASE_SIZE;
	}
	ks_size = LS_BLOCKERASE_SIZE; /* Only one block for data */
	offset = active_block_offset;
	trace_debug("Active block @%X, Free @%X\n", active_block_offset,
		    free_block_offset);
#else
	offset = ks_offset;
#endif

	len = 0;
	while (len < ks_size) {
#if KS_MEMTYPE == NVM_SQI
		sqi_read(ctx, offset, &ls_header, sizeof(ls_header));
#elif KS_MEMTYPE == NVM_MMC
		int ret;

		ret = sdmmc_read(ctx, offset, &ls_header, sizeof(ls_header));
		if (ret < 0)
			return 0;
#else
		memset(&ls_header, 0, sizeof(ls_header));
#endif
		switch (ls_header.magic) {
#if KS_MEMTYPE == NVM_MMC
		case LS_INVALID_ELM: /* try to re-use it if the size is OK */
			if (free_offset && (*free_offset == 0)
			    && size <= (ls_header.size_in_sectors
					* LS_SECTOR_SIZE))
				*free_offset = offset;
			break;
#endif

		case LS_FREE_ELM: /* End reached => exit */
			if (free_offset && (*free_offset == 0))
				*free_offset = offset;
			return offset_id;

		case LS_VALID_ELM:
			if (id == ls_header.id
			    && ls_header.write_count > write_count) {
				/* Invalidate previous duplicated id found ? */
				if (offset_id)
					ls_invalidate(ctx, offset_id);
				write_count = ls_header.write_count;
				offset_id = offset;
			}
			break;

		default:
			break;
		}
		/* Iterate to next element */
#if KS_MEMTYPE == NVM_SQI
		/* On SQI the offset is a byte address */
		offset += ls_header.size_in_sectors * LS_SECTOR_SIZE;
#elif KS_MEMTYPE == NVM_MMC
		/* On MMC the offset is a sector address */
		offset += ls_header.size_in_sectors;
#endif
		len += ls_header.size_in_sectors * LS_SECTOR_SIZE;
	}

#if KS_MEMTYPE == NVM_SQI
	/* Needs to garbage collect ? */
	if (free_offset && (*free_offset == 0)) {
		uint32_t *buf;
		uint32_t offset_recopy;

		buf = pvPortMalloc(BUF_MAX_SIZE);
		if (!buf)
			return RPMx_KO;

		/* Read and recopy active block to the free block */
		len = 0;
		offset = active_block_offset;
		offset_recopy = free_block_offset;
		trace_debug("Start GC, recopy from %X to %X\n",
			    offset, offset_recopy);
		/* Force erasing the free block */
		sqi_erase_block(ctx, free_block_offset, SQI_64KB_GRANULARITY);
		while (len < ks_size) {
			sqi_read(ctx, offset, &ls_header, sizeof(ls_header));
			if (ls_header.magic == LS_VALID_ELM) {
				if (offset_recopy == free_block_offset) {
					/* Don't mark the block as valid yet */
					ls_header.magic = LS_GARBAGING_BLOCK;
				}
				memcpy(buf, &ls_header, sizeof(ls_header));
				sqi_read(ctx, offset + sizeof(ls_header),
					 (void *)buf + sizeof(ls_header),
					 ls_header.payload_size);
				sqi_write(ctx, offset_recopy, buf,
					  ls_header.payload_size
					  + sizeof(ls_header));
				/* Update new offset id ? */
				if (ls_header.id == id)
					offset_id = offset_recopy;
				offset_recopy += ls_header.size_in_sectors * LS_SECTOR_SIZE;
			}
			offset += ls_header.size_in_sectors * LS_SECTOR_SIZE;
			len += ls_header.size_in_sectors * LS_SECTOR_SIZE;
		}
		/* Finally mark the garbaging block as valid */
		sqi_read(ctx, free_block_offset, &ls_header, sizeof(ls_header));
		ls_header.magic = LS_VALID_ELM;
		sqi_write(ctx, free_block_offset,
			  (uint32_t *)&ls_header, sizeof(ls_header));
		/* And erase the old valid block */
		sqi_erase_block(ctx, active_block_offset, SQI_64KB_GRANULARITY);
		/* Update the free offset where to write for the caller */
		*free_offset = offset_recopy;
		vPortFree(buf);
	}
#endif

	return offset_id;
}

/**
 * @brief  This routine writes given blob with given id
 * @param  ctx: NVM context
 * @param  id: EHSM NVM ID
 * @param  p_offset_id (Output): Offset of the previous valid ID or 0 if none
 * @param  payload: pointer to the KVT payload source buffer
 * @param  size: size of the payload
 * @return 0 if OK else error status
 */
static enum RPMx_ErrTy ls_common_write(t_nvm_ctx *ctx, uint32_t id,
				       uint32_t *p_offset_id,
				       const void *payload, uint32_t size)
{
	int ret = 0;
	struct ls_header_elm_t ls_header;
	uint32_t offset;
	uint32_t *buf;

	ls_header.id = id;
	ls_header.write_count = 1; /* Default write count */
	ls_header.magic = LS_VALID_ELM;
	ls_header.payload_size = size;
	ls_header.size_in_sectors = (sizeof(ls_header) + size
				     + LS_SECTOR_SIZE - 1) / LS_SECTOR_SIZE;
	/* Search the ID and free space */
	*p_offset_id = find_valid_id_and_new_offset(ctx, id, size, &offset);
	trace_debug("Write ID:%d, @%X, sz:%d, sz_sect:%d", id, offset, size,
		    ls_header.size_in_sectors);
	if (*p_offset_id) { /* Same id already written ? */
		struct ls_header_elm_t ls_prev_header;
		/* Get write count from it and increased it for this write */
#if KS_MEMTYPE == NVM_SQI
		sqi_read(ctx, *p_offset_id, &ls_prev_header,
			 sizeof(ls_prev_header));
#elif KS_MEMTYPE == NVM_MMC
		ret = sdmmc_read(ctx, *p_offset_id, &ls_prev_header,
				 sizeof(ls_prev_header));
		if (ret < 0)
			return RPMx_KO;
#endif
		ls_header.write_count = ls_prev_header.write_count + 1;
		trace_debug(", Prev offset: @%X", *p_offset_id);
	}

	if (!offset)
		return RPMx_LS_DEVICE_FULL; /* NVM KS full */

	buf = pvPortMalloc(ls_header.size_in_sectors * LS_SECTOR_SIZE);
	if (!buf)
		return RPMx_LS_DEVICE_ERR;

	memcpy(buf, &ls_header, sizeof(ls_header));
	memcpy((void *)buf + sizeof(ls_header), payload, size);

#if KS_MEMTYPE == NVM_SQI
	/* SQI write size must be multiple of 4 */
	ret = sqi_write(ctx, offset, buf, (size + sizeof(ls_header) + 3) & ~3);
#elif KS_MEMTYPE == NVM_MMC
	ret = sdmmc_write(ctx, offset, buf, size + sizeof(ls_header));
#endif
	trace_debug(", WC=%d\n", ls_header.write_count);
#if (DEBUG > 1)
	{
		uint32_t i;

		trace_printf("W: ");
		for (i = 0; i < size; i++)
			trace_printf("%02X ", *(uint8_t *)(payload + i));
		trace_printf("\n");
	}
#endif
	vPortFree(buf);

	if (ret < 0)
		return RPMx_LS_DEVICE_FAILURE;
	else
		return RPMx_OK;
}

/**
 * @brief  This routine writes given NVM blob
 * @param  ctx: NVM context
 * @param  id: EHSM NVM ID
 * @param  payload: pointer to the payload source buffer
 * @param  size: size of the payload
 * @return 0 if OK else error status
 */
enum RPMx_ErrTy ls_write(t_nvm_ctx *ctx, uint32_t id,
			 const void *payload, uint32_t size)
{
	enum RPMx_ErrTy ret = RPMx_OK;
	uint32_t offset_id;

	ret = ls_common_write(ctx, id, &offset_id, payload, size);

	if (ret != RPMx_OK)
		return ret;

	/*
	 * If this id is already written, commit the transaction
	 * by invalidation of this previous content at offset_id
	 */
	if (offset_id)
		ret = ls_invalidate(ctx, offset_id);

	return ret;
}

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
				     const void *key_payload, uint32_t key_size)
{
	enum RPMx_ErrTy ret = RPMx_OK;
	uint32_t kvt_offset_id, key_offset_id;

	/* First write the Key */
	ret = ls_common_write(ctx, key_id, &key_offset_id, key_payload,
			      key_size);
	if (ret != RPMx_OK)
		return ret;

	/* Then write the Key Vector Table (KVT) */
	ret = ls_common_write(ctx, KVT_ID, &kvt_offset_id, kvt_payload,
			      kvt_size);
	if (ret != RPMx_OK)
		return ret;

	/* First invalidate previous KVT */
	if (kvt_offset_id)
		ret = ls_invalidate(ctx, kvt_offset_id);
	/* Then invalidate the previous key */
	if ((ret == RPMx_OK) && key_offset_id)
		ret = ls_invalidate(ctx, key_offset_id);

	return ret;
}

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
			void *payload, uint32_t *p_size)
{
	enum RPMx_ErrTy ret = RPMx_KO;
#if KS_MEMTYPE != NVM_NONE
	uint32_t offset_id;

	offset_id = find_valid_id_and_new_offset(ctx, id, *p_size, NULL);
	trace_debug("Read ID:%d @%X, sz:%d\n", id, offset_id, *p_size);
#endif

#if KS_MEMTYPE == NVM_SQI
	if (offset_id) {
		struct ls_header_elm_t ls_header;

		/* Read header to get actual size */
		sqi_read(ctx, offset_id, &ls_header, sizeof(ls_header));
		/* Check size vs max size */
		if (ls_header.payload_size <= *p_size)
			*p_size = ls_header.payload_size;
			sqi_read(ctx, offset_id + sizeof(ls_header),
				 payload, *p_size);
	}
#elif KS_MEMTYPE == NVM_MMC
	if (offset_id) {
		struct ls_header_elm_t *buf;

		buf = pvPortMalloc(LS_SECTOR_SIZE);
		if (!buf)
			return RPMx_LS_DEVICE_ERR;
		/* Read first sector */
		ret = sdmmc_read(ctx, offset_id, buf, LS_SECTOR_SIZE);
		if (ret >= 0 && buf->payload_size <= *p_size) {
			*p_size = buf->payload_size;
			memcpy(payload,
			       (void *)buf + sizeof(struct ls_header_elm_t),
			       buf->size_in_sectors > 1
			       ? LS_SECTOR_SIZE - sizeof(struct ls_header_elm_t)
			       : *p_size);
			/* Remaining payload if needed */
			if (buf->size_in_sectors > 1) {
				ret = sdmmc_read(ctx,
						 offset_id,
						 payload + LS_SECTOR_SIZE
						 - sizeof(struct ls_header_elm_t),
						 *p_size - LS_SECTOR_SIZE
						 + sizeof(struct ls_header_elm_t));
			}
			if (ret < 0)
				ret = RPMx_LS_DEVICE_FAILURE;
			else
				ret = RPMx_OK;
		}
		vPortFree(buf);

	}
#endif

	return ret;
}

/**
 * @brief  This routine stores a new blob to the non volatile external device
 * @param  ctx: NVM context
 * @param  addr: address belonging to erasable block
 * @param  payload: pointer to the key storage source buffer
 * @param  size: size of the payload
 * @return 0 if OK else error status
 */
enum RPMx_ErrTy ls_store(t_nvm_ctx *ctx, uint32_t addr,
			 void *payload, uint32_t size)
{
	enum RPMx_ErrTy ret = RPMx_LS_DEVICE_ERR;

	/* Clear the temporary buffer */
	memset((uint8_t *)&ks_buff, 0, sizeof(struct lsblock_t));

	/* Set magic number(s) */
	ks_buff.magic[0] = LS_MAGIC0;
	ks_buff.magic[1] = LS_MAGIC1;
	ks_buff.magic[2] = LS_MAGIC2;
	ks_buff.magic[3] = LS_MAGIC3;

	/* Set the size of the payload */
	ks_buff.size = size;

	/* Copy the payload */
	memcpy(ks_buff.payload, payload, size);

	/* Compute CRC */
	ks_buff.crc = compute_crc32(0, (uint8_t *)&ks_buff,
				    (sizeof(struct lsblock_t) - 4));

#if KS_MEMTYPE == NVM_SQI
	/* Erase the block containing the instance */
	sqi_erase_block(ctx, addr, SQI_64KB_GRANULARITY);
	ret = sqi_write(ctx, addr, (uint32_t *)&ks_buff,
			sizeof(struct lsblock_t));
#elif KS_MEMTYPE == NVM_MMC
	/* Write to SDMMC */
	ret = sdmmc_write(ctx, addr, &ks_buff, sizeof(struct lsblock_t));
#endif
	if (ret < 0)
		ret = RPMx_LS_DEVICE_ERR;
	else
		ret = RPMx_OK;

	return ret;
}

/**
 * @brief  This routine discovers the key storage device
 * @param  ctx: NVM context
 * @param  addr: address of KS in NVM storage
 * @return 0 if OK else error status
 */
enum RPMx_ErrTy ls_discover(t_nvm_ctx *ctx, uint32_t addr)
{
	uint32_t crc;

	if (NEW_KS_FORCED || (get_cut_rev() >= CUT_23)) /* Nothing to do from Cut2 BH and higher */
		return RPMx_LS_DEVICE_INIT;

	/* Clear the temporary key storage buffer */
	memset(&ks_buff, 0, sizeof(struct lsblock_t));

	/* Copy the payload location */
#if KS_MEMTYPE == NVM_SQI
	sqi_read(ctx, addr, &ks_buff, sizeof(struct lsblock_t));
#elif KS_MEMTYPE == NVM_MMC
	if (sdmmc_read(ctx, addr, &ks_buff, sizeof(struct lsblock_t)) < 0)
		return RPMx_LS_DEVICE_ERR;
#endif

	/* Verify if the blob is valid */
	if(ks_buff.size == 0)
		ks_state = KS_NOTVALID;
	else {
		/* Compute the CRC for integrity check */
		crc = compute_crc32(0, (uint8_t *) &ks_buff,
				(sizeof(struct lsblock_t) - 4));

		/* Check CRC */
		if (crc == ks_buff.crc) {
			/* Verify magic number(s) */
			if ((ks_buff.magic[0] != LS_MAGIC0) ||
			(ks_buff.magic[1] != LS_MAGIC1) ||
			(ks_buff.magic[2] != LS_MAGIC2) ||
			(ks_buff.magic[3] != LS_MAGIC3)) {
				ks_state = KS_NOTVALID;
			} else
				ks_state = KS_LOADED;
		} else
			/* CRC is not correct */
			ks_state = KS_NOTVALID;
	}

	return RPMx_LS_DEVICE_INIT;
}

/**
 * @brief  This routine copies the latest valid instance
 *         to the destination payload buffer
 * @param  ctx: NVM context
 * @param  payload: pointer to the key storage destination buffer
 * @param  size: payload size
 * @return 0 if OK else error status
 */
enum RPMx_ErrTy ls_load(t_nvm_ctx *ctx, uint32_t addr,
			void *payload, uint32_t size)
{
	enum RPMx_ErrTy ret = RPMx_OK;

	if (ks_state == KS_LOADED) {
		/* Verify the payload size */
		if (ks_buff.size == size)
			/* Copy the latest valid instance */
			memcpy(payload, ks_buff.payload, size);
		else
			/* Size error */
			ret = RPMx_LS_DEVICE_ERR;
	} else if (ks_state == KS_NOTVALID)
		ret = RPMx_LS_DEVICE_ERR;
	else
		ret = RPMx_LS_DEVICE_FAILURE;

	return ret;
}

/**
 * @brief  This routine erase the latest valid instance
 * @param  ctx: NVM context
 * @param  addr: NVM address to clean
 * @return 0 if OK else error status
 */
enum RPMx_ErrTy ls_erase(t_nvm_ctx *ctx, uint32_t addr)
{
	enum RPMx_ErrTy ret = RPMx_LS_DEVICE_ERR;

#if KS_MEMTYPE == NVM_SQI
	/* Erase the block containing the instance */
	ret = sqi_erase_block(ctx, addr, SQI_64KB_GRANULARITY);
#elif KS_MEMTYPE == NVM_MMC
	if ((!NEW_KS_FORCED) && (get_cut_rev() <= CUT_22)) {
		/* Clear the temporary buffer */
		memset((uint8_t *)&ks_buff, 0, sizeof(ks_buff));
		/* Write null data to SDMMC */
		ret = sdmmc_write(ctx, addr, &ks_buff, sizeof(ks_buff));
	} else {
		/* New Key Storage area erasure */
		uint32_t ks_offset, ks_size;
		uint32_t sector;

		if (get_partition_info(KEY_STORAGE_ID, &ks_offset, &ks_size)) {

			ret = RPMx_LS_DEVICE_ERR;
		} else {
			/*
			 * Prepare a LS_SECTOR_SIZE (512) bytes array
			 * to 'erase' key storage dedicated sectors
			 */
			uint8_t *buf = pvPortMalloc(LS_SECTOR_SIZE);
			if (!buf)
				return RPMx_LS_DEVICE_ERR;

			memset((uint8_t *)buf, 0x00U, LS_SECTOR_SIZE);

			trace_debug("Erase @offset: %X, #%d sectors\n",
				    ks_offset, ks_size / LS_SECTOR_SIZE);

			for (sector = 0; sector < ks_size / LS_SECTOR_SIZE; sector++) {
				trace_debug("Sector %d\r", sector);
				ret = sdmmc_write(ctx, ks_offset + sector,
						  buf, LS_SECTOR_SIZE);
				if (ret < 0) /* exit the 'for' loop */
					break;
				trace_debug("\n");
			}
			vPortFree(buf);
		}
	}
#endif
	if (ret < 0)
		ret = RPMx_LS_DEVICE_ERR;
	else
		ret = RPMx_OK;

	return ret;
}

/**
 * @brief  This routine dumps the content of the key storage framework
 *         without decrypting it
 * @param  ctx: NVM context
 * @return 0 if OK else error status
 */
enum RPMx_ErrTy ls_dump_ks_area(t_nvm_ctx *ctx)
{
	enum RPMx_ErrTy ret = RPMx_LS_DEVICE_ERR;

	uint32_t ks_offset, ks_size;
	uint32_t sector;
	uint32_t i;
	uint8_t *buf = pvPortMalloc(LS_SECTOR_SIZE);

	if (!buf)
		return RPMx_LS_DEVICE_ERR;

	if (get_partition_info(KEY_STORAGE_ID, &ks_offset, &ks_size)) {
		ret = RPMx_LS_DEVICE_ERR;
	} else {
		trace_debug("Dump from %d #%d sectors\n",
			    ks_offset, ks_size / LS_SECTOR_SIZE);

		for (sector = 0; sector < ks_size / LS_SECTOR_SIZE; sector++) {
			ret = sdmmc_read(ctx, ks_offset + sector,
					 buf, LS_SECTOR_SIZE);
			if (ret < 0) /* exit the 'for' loop */
				break;

			trace_debug("Sector %d\n", sector);
			for (i = 0; i < LS_SECTOR_SIZE; i++)
				trace_debug("%02X ", *(((uint8_t *)buf) + i));
			trace_debug("\n");
		}
	}
	vPortFree(buf);

	if (ret < 0)
		ret = RPMx_LS_DEVICE_ERR;
	else
		ret = RPMx_OK;

	return ret;
}

/**
 * @brief  This routine turns off the key storage framework
 * @param  ctx: NVM context
 * @return : None
 */
void ls_deinit(t_nvm_ctx *ctx)
{
	/* Deinit the device */
	return;
}

#define MAX_KEYS 20
void ls_tests(t_nvm_ctx *ctx)
{
#if defined LS_TESTS
	uint32_t i, len, id;
	enum RPMx_ErrTy err;
	const char *payload = "0123456789ABCDFEFGHIJKLMNOPQRSTUVWXYZ";
	char *buf = pvPortMalloc(BUF_MAX_SIZE);

	if (!buf)
		return;

	TRACE_NOTICE("%s start\n", __func__);

	/* First write MAX_KEYS keys */
	for (id = 0; id < MAX_KEYS; id++) {
		err = ls_write(ctx, id, payload, (id + 1) % strlen(payload));
		if (err != RPMx_OK) {
			trace_printf("ls_write error = %d @ #%d\n",
				     err, __LINE__);
			return;
		} else {
			trace_printf("Key%d written\n", id);
		}
	}
	trace_printf("#%d KS IDs written\n", id);

	/* Read & Replace some keys */
	for (id = 0; id < MAX_KEYS; id++) {
		if ((id % 5) == 0) {
			err = ls_read(ctx, id, buf, (id + 1) % strlen(payload));
			trace_printf("Write Key%d = ", id);
			for (i = 0; i < (id + 1) % strlen(payload); i++)
				trace_printf("%02X ", buf[i]);
			trace_printf("\n");
			len = strlen(payload) - (id % strlen(payload));
			err = ls_write(ctx, id, &payload[id % strlen(payload)],
				       len);
			/* Read again modified IDs */
			err = ls_read(ctx, id, buf, len);
			trace_printf("Read Key%d = ", id);
			for (i = 0; i < len; i++)
				trace_printf("%02X ", buf[i]);
			trace_printf("\n");
		}
	}

	vPortFree(buf);
#endif /* LS_TESTS */
}
/** @} */
