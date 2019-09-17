/**
 * @file sta_nand.h
 * @brief This file provides all the NAND firmware definitions
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#ifndef _NAND_H_
#define _NAND_H_

#include "sta_map.h"

#define FSMC_PISR_ERROR8		0x00FF0000

/* Common definitions */
#define NAND_MAKER_ID_MICRON		0x2C
#define NAND_MAKER_ID_MACRONIX		0xC2
#define NAND_DEVICE_ID			0xF1
#define NAND_TIMING_SET			0x07
#define NAND_TIMING_HOLD		0x03
#define NAND_TIMING_WAIT		0x07

#define NAND_BLOCK_SIZE_KB		512

#if NAND_BLOCK_SIZE_KB == 128
#define NAND_DATA_BYTES_PER_SECTOR	2048
#define NAND_SPARE_BYTES_PER_SECTOR	224
#define NAND_SECTORS_PER_BLOCK		64
#define NAND_BLOCK_TO_SECTOR_SHIFTER	6
#elif NAND_BLOCK_SIZE_KB == 512
#define NAND_DATA_BYTES_PER_SECTOR	4096
#define NAND_SPARE_BYTES_PER_SECTOR	224
#define NAND_SECTORS_PER_BLOCK		128
#define NAND_BLOCK_TO_SECTOR_SHIFTER	7
#else
#error "NAND_BLOCK_SIZE_KB must be 128 or 512\n"
#endif

/*
 * Standard NAND flash commands
 */
#define NAND_CMD_READ			0x00
#define NAND_CMD_READ_PAGE		0x30
#define NAND_CMD_PROGRAM_PAGE_START	0x80
#define NAND_CMD_PROGRAM_PAGE_END	0x10
#define NAND_CMD_RANDOM_DATA_READ_START	0x05
#define NAND_CMD_RANDOM_DATA_READ_END	0xE0
#define NAND_CMD_RANDOM_DATA_INPUT	0x85
#define NAND_CMD_READ_ID		0x90
#define NAND_CMD_READ_PARAM		0xEC
#define NAND_CMD_RESET			0xFF
#define NAND_CMD_ERASE_BLOCK_START	0x60
#define NAND_CMD_ERASE_BLOCK_END	0xD0
#define NAND_CMD_STATUS			0x70

/* Extended commands for read cache page */
#define NAND_CMD_READ_CACHE		0x31
#define NAND_CMD_READ_CACHE_END		0x3F

/* Status bits */
#define NAND_STATUS_FAIL		0x01
#define NAND_STATUS_FAIL_N1		0x02
#define NAND_STATUS_TRUE_READY		0x20
#define NAND_STATUS_READY		0x40
#define NAND_STATUS_WP			0x80

/**
 * @brief	Return the size of a NAND page
 * @param	size: Size in bytes
 * @return	the size in bytes
 */
uint32_t nand_page_size(void);

/**
 * @brief	Return the size rounded up in NAND page size
 * @param	size: Size in bytes
 * @return	the size in page/sector
 */
uint32_t nand_size_in_pages(uint32_t size);

/**
 * @brief	Init the NAND device for NAND Chip Select 0
 * @param	None
 * @return	0 if no error, not 0 otherwise
 */
int nand_init(void);

/**
 * @brief	Read NAND pages depending on the input size parameters
 * @param	sector_address: starting nand page address
 * @param	buff: data buffer pointer
 * @param	size: size in bytes
 * @param	size_max: maximum size in bytes (partition size)
 * @return	0 if no error, not 0 otherwise
 */
int nand_read_pages(uint32_t sector_address, uint32_t *buff,
		uint32_t size, uint32_t size_max);

/**
 * @brief	erase NAND block by calling ERASE BLOCK command: sends the block
 * address (on 3 bytes) data in the common area
 * @param	ba: the block address to be erased
 * @return	0 if no error, not 0 otherwise
 */
int nand_erase_block(uint32_t ba);

/**
 * @brief	Write NAND pages depending on the input size parameters
 * @param	sector_address: starting sector address
 * @param	buff: data buffer pointer
 * @param	size: size in bytes
 * @param	size_max: maximum size in bytes (partition size)
 * @return	0 if no error, not 0 otherwise
 */
int nand_write_pages(uint32_t sector_address, uint8_t *buff,
		uint32_t size, uint32_t size_max);

/**
 * @brief	FSMC IRQ handler
 */
void nand_irq_handler(void);

#endif /* _NAND_H_ */
