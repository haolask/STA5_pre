/**
 * @file sta_image.h
 * @brief This file provides flash read image definitions.
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#ifndef _STA_IMAGE_H_
#define _STA_IMAGE_H_

/*
 * Partition type IDs for M3/XL1 managed partitions
 */
/* Partition type IDs */
enum partition_id {
	/* System partitions */
	M3_XL_ID = 0,
	AP_XL_ID = 1,
	M3_OS_ID = 2,
	SPLASH_ID = 3,
	FILE1_ID = 4,
	FILE2_ID = 5,
	AUDIO_LIB_ID = 6,
	AUDIO_STATUS_ID = 7,
	SPLASH_ANIMATION_ID = 8,
	KEY_STORAGE_ID = 9,
	M3_OS_PART2_ID = 10,

	/* General purpose user partitons */
	USER_PART1 = 0x11,
	USER_PART2,
	USER_PART3,
	USER_PART4,
	USER_PART5,
	USER_PART6,
	USER_PART7,
	USER_PART8,
	USER_PART9,
	USER_PART10,

	/*... normal partitions < 0x20 */

	/* STA DSP firmware partitions */
	DSP0_X_ID = 0x20,
	DSP0_Y_ID,
	DSP0_P_ID,
	DSP1_X_ID,
	DSP1_Y_ID,
	DSP1_P_ID,
	DSP2_X_ID,
	DSP2_Y_ID,
	DSP2_P_ID,

	/* Other firmware partitions */
	FIRMWARE1_ID = 0x41,
	FIRMWARE2_ID,
	FIRMWARE3_ID,
	FIRMWARE4_ID,
	FIRMWARE5_ID,
	FIRMWARE6_ID,
	FIRMWARE7_ID,
	FIRMWARE8_ID,
	FIRMWARE9_ID,
	FIRMWARE10_ID,

	/*... other partitions */

	/* Backup partitions */
	M3_XL2_ID = 0x80,
	AP_XL2_ID = 0x81,
	M3_OS2_ID = 0x82,

	/* Raw Partitions without XL1 header */
	AP_BL32_SNAPSHOT_ID = 0xC0,
	AP_RAMDUMP_ID = 0xC1,
	AP_TEE_SNAPSHOT_ID = 0xC2
};

enum read_image_status_t {
	OK = 0,
	GENERIC_ERROR = -1,
	BAD_HEADER_CRC = -1000,
	BAD_IMAGE_CRC = -1001,
	BAD_MAGIC = -1002,
	BAD_IMAGE_TYPE = -1003,
	BAD_CODE_OFFSET = -1004,
};

/*
 * WARNING!!! These 2 following enum values must be aligned
 * with corresponding values defined in flashLoader STSecure.pm file
 */
enum hash_type_t {
	HASH_NONE = 0,
	HASH_SHA_256 = 2,
	HASH_SHA_224 = 3,
	HASH_SHA_384 = 4,
	HASH_SHA_512 = 5,
};

enum cipher_type_t {
	CIPHER_NONE = 0,
	AES_CBC = 0x0e,
	AES_GCM = 0x1b,
};

#define XL1_MAGIC		0x009010EB

#define MAX_PARTITIONS		40

#define IMAGE_SHADOW_ALLOC	0x01u
#define IMAGE_SHADOW_ALLOC_LIBC	0x02u

#define MMC_SIZE_IN_SECTORS(x)	(uint32_t)(((x + MMC_BLOCK_SIZE - 1) & ~(MMC_BLOCK_SIZE - 1)) >> MMC_BLOCK_POWER)
#define MMC_SIZE_IN_SECTORS_ROUND_DOWN(x)     (uint32_t)(((x) & ~(MMC_BLOCK_SIZE - 1)) >> MMC_BLOCK_POWER)

typedef void entry_point_t(void);

/*
 * Partitioning configuration
 */
struct partition_t {
	uint32_t id;			/* See above Entities IDs */
	uint32_t offset;        /* In sector number (512 bytes for MMC,
				 * page size 2048 or 4096 bytes for NAND)
				 */
	uint32_t size;
};

struct partition_config_t {
	/* Partitions desc for AP U-boot based Flasher and Xloader */
	uint32_t partition_number;
	struct partition_t partitions[MAX_PARTITIONS];
} __attribute__ ((aligned (512)));

#ifndef RESERVE_TOC
#define RESERVE_TOC extern
#endif

/**
 * @struct system_config
 * @brief System configuration (Partitions mapping for M3)
 */
RESERVE_TOC struct partition_config_t m3_toc
		__attribute__((section(".sysconfig")));

/*
 * XL1/M3 image header packed format: 64 bytes
 */
struct xl1_header_t {
	uint32_t xl1_magic;           /* Always XL1_MAGIC */
	union {
		uint8_t  rom_bootdev_params[12]; /* depends on boot device type */
		struct {
			uint8_t  free[4];
			uint8_t  type_id;  /* Image partition type ID see above */
			uint8_t  hash_type;  /* See above hash_type_t enum */
			uint8_t  cipher_type;  /* See above cipher_type_t enum */
			uint8_t  reserved[5];
		} xl1;
	} row0;

	uint32_t size;                /* Image size without header */
	uint32_t replica;             /* Replica number: 1 by default */
	uint32_t replica_distance;    /* 0 by default */
	uint32_t code_offset;         /* code offset in byte from header start */

	uint8_t  markers[16];         /* Always: A0 A1 A2 A3 A4 A5 A6 A7 */
                                      /*         A8 A9 AA AB AC AD AE AF */

	union {
		uint8_t  rom_bootdev_params[8]; /* depends on boot device type */
		struct {
			/* Address where to shadow image */
			void *shadow_address;
			/* Code entry point if executable */
			entry_point_t *entry_address;
		} xl1;
	} row3;
	uint32_t xl1_image_crc;       /* Image CRC */
	uint32_t xl1_header_crc;      /* Header CRC */
};

struct xl1_part_info_t {
	void *shadow_address;         /* Address where to shadow image */
	entry_point_t *entry_address; /* Code entry point if executable */
	uint32_t size;                /* Image size without header */
	enum hash_type_t hash_type;   /* 0 if not hashed */
	enum cipher_type_t cipher_type; /* 0 if no ciphering */
};

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
		       uint32_t *addr, uint32_t *size_max);

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
		  struct xl1_part_info_t *part_info, uint32_t flags);

/**
 * @brief	This routine try to read a binary image.
 * @param	part_id: Partition ID type
 * @param	part_info: partition informations (out)
 * @return	0 if no error, 1 if not present and negative if error
 */
int read_image(enum partition_id part_id, struct sta *context,
	       struct xl1_part_info_t *part_info, uint32_t flags);

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
		     uint32_t src_addr, uint32_t src_size);
#endif
