/**
 * @file sta_nand.h
 * @brief This file provides all the NAND firmware definitions
 *
 * Copyright (C) ST-Microelectronics SA 2017
 * @author: ADG-MID team
 */

#ifndef _NAND_H_
#define _NAND_H_

/* Common definitions */
#define NAND_MAKER_ID_MICRON	        0x2C
#define NAND_MAKER_ID_MACRONIX	        0xC2

#define NAND_TIMING_SET			0x07
#define NAND_TIMING_HOLD		0x03
#define NAND_TIMING_WAIT		0x07

#define NAND_BLOCK_SIZE_KB              512

#if NAND_BLOCK_SIZE_KB == 128
#define NAND_DATA_BYTES_PER_SECTOR	2048
#define NAND_SPARE_BYTES_PER_SECTOR	224
#define NAND_SECTORS_PER_BLOCK		64
#define NAND_BLOCK_TO_SECTOR_SHIFTER    6
#elif NAND_BLOCK_SIZE_KB == 512
#define NAND_DATA_BYTES_PER_SECTOR	4096
#define NAND_SPARE_BYTES_PER_SECTOR	224
#define NAND_SECTORS_PER_BLOCK		128
#define NAND_BLOCK_TO_SECTOR_SHIFTER    7
#else
#error "NAND_BLOCK_SIZE_KB must be 128 or 512\n"
#endif

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

/*
 * IO block device interfaces
 */
typedef struct nand_flash_ops {
	int (*init)(void);
	int (*read)(int lba, uintptr_t buf, size_t size);
} nand_flash_ops_t;

size_t nand_flash_read_blocks(int lba, uintptr_t buf, size_t size);
int sta_nand_init(void);

#endif /* _NAND_H_ */
