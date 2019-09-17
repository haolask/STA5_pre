/**
 * @file sta_image.c
 * @brief This file provides all flash image management.
 *
 * Copyright (C) ST-Microelectronics SA 2017
 * @author: APG-MID team
 */

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "FreeRTOS.h" /* For malloc/free */

#include "sta_common.h"
#include "sdmmc.h"
#include "sta_nand.h"
#include "sta_crc.h"
#include "trace.h"
#include "sta_sqi.h"
#include "sta_mtu.h"
#include "utils.h"

#define RESERVE_TOC
#include "sta_image.h"
#include "sta_cot.h"

/**
 * @brief	This routine returns the storage flash address and
 * size_max related to a partition IDs stored in the system configuration area.
 * return -1 if the flash/emmc address isn't stored in the system configuration.
 * For Nand returned address is a nand sector/page number (2048 or 4096 bytes)
 * For MMC returned address is a sector number (512 bytes)
 * @param	part_id: partition ID
 * @param	addr: Out partition offset
 * @param	size_max: Out partition size
 * @return	status 0 if found
 */
int get_partition_info(enum partition_id part_id,
		       uint32_t *addr, uint32_t *size_max)
{
	volatile const struct partition_t *p;

	TRACE_ASSERT(addr);
	/* Check if partition configuration is valid */
	if (m3_toc.partition_number != 0 &&
	    m3_toc.partition_number <= MAX_PARTITIONS) {
		for (p = &m3_toc.partitions[0];
				p < &m3_toc.partitions[MAX_PARTITIONS]; p++) {
			if ((enum partition_id)p->id == part_id) {
				*addr = p->offset;
				if (size_max)
					*size_max = p->size;
				return 0;
			}
		}
	}
	return -1;
}

/**
 * @brief	This routine check XL1/ROM header
 * @param	header: Header data pointer
 * @param	part_id: Partition ID type
 * @return	0 if no error, not 0 otherwise
 */
static int check_header(
		struct xl1_header_t *header,
		enum partition_id part_id)
{
	uint32_t header_crc;

	if (header->xl1_magic != XL1_MAGIC)
		return BAD_MAGIC;
	if (header->row0.xl1.type_id != part_id)
		return BAD_IMAGE_TYPE;
	if (header->code_offset != sizeof(struct xl1_header_t))
		return BAD_CODE_OFFSET; /* code_offset != 64 not managed */
	header_crc = compute_crc32(0, (uint8_t *)header, sizeof(*header)
				   - sizeof(header->xl1_header_crc));
	if (header_crc != header->xl1_header_crc)
		return BAD_HEADER_CRC;
	return OK;
}

/**
 * @brief	This routine read a binary image at specific address offset
 * @param	part_id: Partition ID type
 * @param	addr: Flash address offset
 * @param   size_max: Partition max size
 * @param	part_info: partition informations (out)
 * @return	0 if no error, 1 if not present and negative if error
 */
int read_image_at(enum partition_id part_id, struct sta *context,
		uint32_t addr, uint32_t size_max,
		struct xl1_part_info_t *part_info, uint32_t flags)
{
	int err = GENERIC_ERROR;
	struct xl1_header_t *header;
	uint32_t xl1_image_crc;
	uint32_t already_read_sz;
	uint32_t *buf;
	__maybe_unused uint32_t time;
#if defined SQI
	uint32_t buf_sz = sizeof(struct xl1_header_t);
#elif defined NAND
	uint32_t buf_sz = nand_page_size();
#elif defined MMC
	uint32_t buf_sz = MMC_BLOCK_SIZE;
#endif

#if TRACE_LEVEL >= TRACE_LEVEL_INFO
	time = mtu_get_time(0);
#endif

	/* First read header */
	buf = pvPortMalloc(buf_sz);
	if (!buf)
		return -ENOMEM;

#if defined SQI
	/* The offset is in bytes */
	sqi_read(context->sqi_ctx, addr, buf, sizeof(struct xl1_header_t));
#elif defined NAND
	/* Read image first nand page */
	err = nand_read_pages(addr, buf, buf_sz, size_max);
	if (err)
		goto read_image_error;
#elif defined MMC
	/* Read image first sector */
	err = sdmmc_read((struct t_mmc_ctx *)context->mmc_ctx, addr,
			 buf, buf_sz);
	if (err < 0)
		goto read_image_error;
#else
#error "You must define one of NAND, SQI or MMC"
#endif

	header = (struct xl1_header_t *)buf;
	err = check_header(header, part_id);
	if (err == 0) {
		uint32_t size = header->size;
		uint8_t *shadow_address = header->row3.xl1.shadow_address;
		if (flags & IMAGE_SHADOW_ALLOC) {
			shadow_address =  pvPortMalloc(size);
			if (!shadow_address)
				return -ENOMEM;
		}
		else if (flags & IMAGE_SHADOW_ALLOC_LIBC) {
			shadow_address =  malloc(size);
			if (!shadow_address)
				return -ENOMEM;
		}
		if (part_info) {
			part_info->shadow_address = shadow_address;
			part_info->entry_address = header->row3.xl1.entry_address;
			part_info->size = header->size;
			part_info->hash_type = header->row0.xl1.hash_type;
			part_info->cipher_type = header->row0.xl1.cipher_type;
		}
		xl1_image_crc = header->xl1_image_crc;
		already_read_sz = buf_sz - sizeof(struct xl1_header_t);
		if (already_read_sz > size)
			already_read_sz = size;

		/* Recopy useful image data already read with header */
		if (already_read_sz)
			memcpy(shadow_address, (uint8_t *)buf
			       + sizeof(struct xl1_header_t), already_read_sz);

		if (size > already_read_sz) {
#if defined SQI
			sqi_read(context->sqi_ctx, addr
				 + sizeof(struct xl1_header_t),
				 shadow_address, size - already_read_sz);
#elif defined NAND
			err = nand_read_pages(addr + 1,
					      (uint32_t *)(shadow_address
					      + already_read_sz),
					      size - already_read_sz, size_max);
			if (err)
				goto read_image_error;
#elif defined MMC
			err = sdmmc_read((struct t_mmc_ctx *)context->mmc_ctx,
					 addr + 1,
					 shadow_address + already_read_sz,
					 size - already_read_sz);
			if (err < 0)
				goto read_image_error;
#endif
		}

#if TRACE_LEVEL >= TRACE_LEVEL_INFO
		if (size > (10 *1024))
			TRACE_INFO("\n%s: (%dKB) speed %dKB/s\n", __func__,
				   size >> 10,
				   ((size >> 10) * 1000000)
				   / mtu_get_time(time));

		time = mtu_get_time(0);
#endif
		/* Check CRC if image not signed & not ciphered */
		if (header->row0.xl1.hash_type == HASH_NONE &&
		    header->row0.xl1.cipher_type == CIPHER_NONE) {
			if (compute_crc32(0, shadow_address, size) != xl1_image_crc) {
				err = BAD_IMAGE_CRC;
				goto read_image_error;
			}
			TRACE_INFO("%s: CRC %dKB/s\n", __func__,
				   ((size >> 10) * 1000000) / mtu_get_time(time));
		}

		/* M3 entry is the reset vector: content of shadow_address+4 */
		if (part_info && (part_id == M3_OS_ID || part_id == M3_OS2_ID))
			part_info->entry_address =
				(entry_point_t *)((uint32_t *)shadow_address)[1];

		TRACE_NOTICE("P:%d\n", part_id);
		err = 0; /* OK */
	}

read_image_error:
	vPortFree(buf);
	return err; /* Error */
}

/**
 * @brief	This routine try to read a binary image.
 * @param	part_id: Partition ID type
 * @param	part_info: partition informations (out)
 * @return	0 if no error, 1 if not present and negative if error
 */
int read_image(enum partition_id part_id, struct sta *context,
			struct xl1_part_info_t *part_info, uint32_t flags)
{
	int err = GENERIC_ERROR;
	uint32_t addr = 0;
	uint32_t size_max = 0;

	err = get_partition_info(part_id, &addr, &size_max);

	if (!size_max || err) /* No partition => nothing to do */
		return 1;

	err = read_image_at(part_id, context, addr, size_max, part_info, flags);

#if defined COT
	if (!err && (part_id == AP_XL_ID))
		err = cot_check_ap_xl(part_info);
#endif /* COT */

	return err;
}

/**
 * @brief	This routine write a raw binary image from memory area.
 * @param	context : sta context
 * @param	dst_addr : nvm destination address
 * @param	dst_size : nvm destination size (coherency test)
 * @param	src_addr : memory source address
 * @param	src_size : size of buffer to store
 * @return	0 if no error, negative if error
 */
int write_raw_binary(struct sta *context,
		     uint32_t dst_addr, uint32_t dst_size,
		     uint32_t src_addr, uint32_t src_size)
{
#if defined MMC
	int ret;
	uint32_t bytes_to_write;
	uint32_t length;
	struct t_mmc_ctx *mmc;
#endif

	TRACE_ASSERT(dst_size && src_size);
	TRACE_ASSERT(dst_size >= src_size);

#if defined SQI
	TRACE_ERR("%s: SQI Not supported\n", __func__);
	return 1;
#elif defined NAND
	TRACE_ERR("%s: NAND Not supported\n", __func__);
	return 1;
#elif defined MMC
	mmc = (struct t_mmc_ctx *)context->mmc_ctx;

	dst_addr /= MMC_BLOCK_SIZE;

	bytes_to_write = src_size;

	while (bytes_to_write) {
		length = bytes_to_write > MMC_TRANSFERT_MAX_SIZE ?
			MMC_TRANSFERT_MAX_SIZE : bytes_to_write;
		TRACE_NOTICE("%s: sdmmc writting sector:0x%x Src@%x size %dB\n",
			     __func__,  dst_addr, src_addr, length);
		ret = sdmmc_write(mmc, dst_addr, (void *)src_addr, length);
		if (ret != (int)length) {
			TRACE_ERR("%s: sdmmc writting E:%d\n", __func__, ret);
			break;
		}
		TRACE_NOTICE("%s: sdmmc written 0x%xB\n", __func__, length);
		src_addr += length;
		dst_addr += length / MMC_BLOCK_SIZE;
		bytes_to_write -= length;
	}

	return (ret < 0 ? -1 : 0);
#endif
}
