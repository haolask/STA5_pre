/**
 * @file sta_nand.c
 * @brief This file provides all the NAND firmware functions
 *
 * Copyright (C) ST-Microelectronics SA 2017
 * @author: ADG-MID team
 */

#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <string.h>
#include <errno.h>
#include <utils.h>
#include <stdbool.h>
#include <mmio.h>
#include <hsem.h>

#include "sta_nand.h"

extern int ffs(int i);
extern void nand_fast_read_512(uint32_t *buf, uint32_t *nand_base);

#define NAND_BLOCK_SIZE	(ctx.sectors_per_block * ctx.data_bytes_per_sector)

/* FSMC_BCR0/1 register */
#define MBKEN				0
#define MUXEN				1
#define MTYP				2
#define MTYP_MASK			0x3
#define MWID				4
#define MWID_MASK			0x3
#define FRSTLVL				6
#define FWPRLVL				7
#define BURSTEN				8
#define WAITPOL				9
#define WRAPMOD				10
#define WAITCFG				11
#define WREN				12
#define FORCE_BUS_TURN			13
#define EXTDMOD				14
#define WAITASYN			15
#define CPAGESIZE			16
#define CPAGESIZE_MASK			0x7
#define CBURSTRW			19
#define OENDELAY			20

/* FSMC_PCR0 register */
#define PRSTLVL				0
#define PWAITEN				1
#define PBKEN				2
#define PTYP				3
#define PWID				4
#define PWID_MASK			3
#define ECCEN				6
#define ECCPLEN				7
#define ADLOW				8
#define TCLR				9
#define TCLR_MASK			0xF
#define TAR				13
#define TAR_MASK			0xF
#define PAGESIZE			17
#define PAGESIZE_MASK			0x7

/* FSMC_PSR(x) register */
#define PSR_INTRRE			0
#define PSR_INTRLVL			1
#define PSR_INTRFE			2
#define PSR_ENRE			3
#define PSR_ENLVL			4
#define PSR_ENFE			5
#define PSR_FIFOEMP			6
#define PSR_ERRFOUND			10
#define PSR_ERRFOUND_MASK		0xF
#define PSR_CODE_RDY			15

/* FSMC_PMEM(x) register */
#define MEMXHOLD_SHIFT			16
#define MEMXWAIT_SHIFT			8

/* FSMC_PERIPHID(x) register */
#define FSMC_REVISION_SHIFT		0x4
#define FSMC_REVISION_MASK		0xF

/* ECC management */
#define FSMC_ECCR_ERROR1_SHIFT		0
#define FSMC_ECCR_ERROR2_SHIFT		13
#define FSMC_ECCR_ERROR3_SHIFT		26

#define FSMC_ECC2R_ERROR3_SHIFT		0
#define FSMC_ECC2R_ERROR4_SHIFT		7
#define FSMC_ECC2R_ERROR5_SHIFT		20

#define FSMC_ECC3R_ERROR5_SHIFT		0
#define FSMC_ECC3R_ERROR6_SHIFT		1
#define FSMC_ECC3R_ERROR7_SHIFT		14
#define FSMC_ECC3R_ERROR8_SHIFT		27

#define FSMC_PISR_ERROR8_SHIFT		16

#define FSMC_ECCR_ERROR1		0x00001FFF
#define FSMC_ECCR_ERROR2		0x03FFE000
#define FSMC_ECCR_ERROR3		0xFC000000

#define FSMC_ECC2R_ERROR3		0x0000007F
#define FSMC_ECC2R_ERROR4		0x000FFF80
#define FSMC_ECC2R_ERROR5		0xFFF00000

#define FSMC_ECC3R_ERROR5		0x00000001
#define FSMC_ECC3R_ERROR6		0x00003FFE
#define FSMC_ECC3R_ERROR7		0x07FFC000
#define FSMC_ECC3R_ERROR8		0xF8000000

#define FSMC_GET_BITS(x, y, z)		((((x) & (z)) >> y))
#define FSMC_PISR_ERROR8		0x00FF0000

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

/* NAND Registers Offsets */
#define NAND_CS0_CMD_OFFSET		0x10000
#define NAND_CS0_COMMON_AREA_OFFSET	0x20000

#define NAND_MANUF_ID_MASK		0xFF

#define NAND_ONFI_PARAM_LENGTH		(256 / sizeof(uint32_t))
/* ONFI optional Read Cache commands supported? */
#define ONFI_OPT_CMD_READ_CACHE_CMDS	(1 << 1)

#define NAND_MAX_ERROR_TO_CORRECT	8

/** @brief FSMC */
typedef volatile struct {
    union {
        struct {
            uint32_t bank_enable:1;
            uint32_t muxed:1;
            uint32_t memory_type:2;
            uint32_t device_width:2;
            uint32_t rst_prwdwn:1;
            uint32_t w_prot:1;
            uint32_t burst_enable:1;
            uint32_t wait_pol:1;
            uint32_t wrap_support:1;
            uint32_t wait_during:1;
            uint32_t if_we:1;
            uint32_t frc_bus_turn:1;
            uint32_t extend_mode:1;
            uint32_t wait_asynch:1;
            uint32_t c_page_size:3;
            uint32_t c_burst_rw:1;
            uint32_t OEn_delay:1;
            uint32_t reserved:11;
        } bit;
        uint32_t reg;
    } fsmc_bcr0;		/* 0x0 */

    union {
        struct {
            uint32_t addr_st:4;
            uint32_t hold_addr:4;
            uint32_t data_st:8;
            uint32_t bus_turn:4;
            uint32_t burst_c_len:4;
            uint32_t data_lat:4;
            uint32_t access_mode:2;
            uint32_t reserved:2;
        } bit;
        uint32_t reg;
    } fsmc_btr0;		/* 0x4 */

    union {
        struct {
            uint32_t bank_enable:1;
            uint32_t muxed:1;
            uint32_t memory_type:2;
            uint32_t device_width:2;
            uint32_t rst_prwdwn:1;
            uint32_t w_prot:1;
            uint32_t burst_enable:1;
            uint32_t wait_pol:1;
            uint32_t wrap_support:1;
            uint32_t wait_during:1;
            uint32_t if_we:1;
            uint32_t frc_bus_turn:1;
            uint32_t extend_mode:1;
            uint32_t wait_asynch:1;
            uint32_t c_page_size:3;
            uint32_t c_burst_rw:1;
            uint32_t OEn_delay:1;
            uint32_t reserved:11;
        } bit;
        uint32_t reg;
    } fsmc_bcr1;		/* 0x8 */

    union {
        struct {
            uint32_t addr_st:4;
            uint32_t hold_addr:4;
            uint32_t data_st:8;
            uint32_t bus_turn:4;
            uint32_t burst_c_len:4;
            uint32_t data_lat:4;
            uint32_t access_mode:2;
            uint32_t reserved:2;
        } bit;
        uint32_t reg;
    } fsmc_btr1;		/* 0xc */

    union {
        struct {
            uint32_t bank_enable:1;
            uint32_t muxed:1;
            uint32_t memory_type:2;
            uint32_t device_width:2;
            uint32_t rst_prwdwn:1;
            uint32_t w_prot:1;
            uint32_t burst_enable:1;
            uint32_t wait_pol:1;
            uint32_t wrap_support:1;
            uint32_t wait_during:1;
            uint32_t if_we:1;
            uint32_t frc_bus_turn:1;
            uint32_t extend_mode:1;
            uint32_t wait_asynch:1;
            uint32_t c_page_size:3;
            uint32_t c_burst_rw:1;
            uint32_t OEn_delay:1;
            uint32_t reserved:11;
        } bit;
        uint32_t reg;
    } fsmc_bcr2;		/* 0x10 */

    union {
        struct {
            uint32_t addr_st:4;
            uint32_t hold_addr:4;
            uint32_t data_st:8;
            uint32_t bus_turn:4;
            uint32_t burst_c_len:4;
            uint32_t data_lat:4;
            uint32_t access_mode:2;
            uint32_t reserved:2;
        } bit;
        uint32_t reg;
    } fsmc_btr2;		/* 0x14 */

    uint8_t unused_0[0x40 - 0x18];

    union {
        struct {
            uint32_t reset:1;
            uint32_t wait_on:1;
            uint32_t enable:1;
            uint32_t device_type:1;
            uint32_t device_width:2;
            uint32_t ecc_enable:1;
            uint32_t ecc_plen:1;
            uint32_t addr_mux:1;
            uint32_t tclr:4;
            uint32_t tar:4;
            uint32_t page_size:3;
            uint32_t fifo_thresh:3;
            uint32_t ecc_mask:1;
            uint32_t reserved:8;
        } bit;
        uint32_t reg;
    } fsmc_pcr0;		/* 0x40 */

    union {
        struct {
            uint32_t int_re_stat:1;
            uint32_t int_le_stat:1;
            uint32_t int_fe_stat:1;
            uint32_t int_re_en:1;
            uint32_t int_le_en:1;
            uint32_t int_fe_en:1;
            uint32_t fifo_empty:1;
            uint32_t ecc_type:3;
            uint32_t errors:4;
            uint32_t code_type:1;
            uint32_t code_ready:1;
            uint32_t ecc_byte_13:8;
            uint32_t reserved:8;
        } bit;
        uint32_t reg;
    } fsmc_psr0;		/* 0x44 */

    union {
        struct {
            uint32_t t_set:8;
            uint32_t t_wait:8;
            uint32_t t_hold:8;
            uint32_t t_hiz:8;
        } bit;
        uint32_t reg;
    } fsmc_mem0;		/* 0x48 */

    union {
        struct {
            uint32_t t_set:8;
            uint32_t t_wait:8;
            uint32_t t_hold:8;
            uint32_t t_hiz:8;
        } bit;
        uint32_t reg;
    } fsmc_patt0;		/* 0x4c */

    union {
        struct {
            uint32_t t_set:8;
            uint32_t t_wait:8;
            uint32_t t_hold:8;
            uint32_t t_hiz:8;
        } bit;
        uint32_t reg;
    } fsmc_pio0;		/* 0x50 */

    uint32_t fsmc_eccr0[3];	/* 0x54 - 0x5c */

    union {
        struct {
            uint32_t reset:1;
            uint32_t wait_om:1;
            uint32_t enable:1;
            uint32_t device_type:1;
            uint32_t device_width:2;
            uint32_t ecc_enable:1;
            uint32_t ecc_plen:1;
            uint32_t addr_mux:1;
            uint32_t tclr:4;
            uint32_t tar:4;
            uint32_t page_size:3;
            uint32_t fifo_thresh:3;
            uint32_t ecc_mask:1;
            uint32_t reserved:8;
        } bit;
        uint32_t reg;
    } fsmc_pcr1;		/* 0x60 */

    union {
        struct {
            uint32_t int_re_stat:1;
            uint32_t int_le_stat:1;
            uint32_t int_fe_stat:1;
            uint32_t int_re_en:1;
            uint32_t int_le_en:1;
            uint32_t int_fe_en:1;
            uint32_t fifo_empty:1;
            uint32_t ecc_type:3;
            uint32_t errors:4;
            uint32_t code_type:1;
            uint32_t code_ready:1;
            uint32_t ecc_byte_13:8;
            uint32_t reserved:8;
        } bit;
        uint32_t reg;
    } fsmc_psr1;		/* 0x64 */

    union {
        struct {
            uint32_t t_set:8;
            uint32_t t_wait:8;
            uint32_t t_hold:8;
            uint32_t t_hiz:8;
        } bit;
        uint32_t reg;
    } fsmc_mem1;		/* 0x68 */

    union {
        struct {
            uint32_t t_set:8;
            uint32_t t_wait:8;
            uint32_t t_hold:8;
            uint32_t t_hiz:8;
        } bit;
        uint32_t reg;
    } fsmc_patt1;		/* 0x6c */

    union {
        struct {
            uint32_t t_set:8;
            uint32_t t_wait:8;
            uint32_t t_hold:8;
            uint32_t t_hiz:8;
        } bit;
        uint32_t reg;
    } fsmc_pio1;		/* 0x70 */

    uint32_t fsmc_eccr1[3];	/* 0x74 - 0x7c */

    uint8_t unused_1[0x104 - 0x80];

    union {
        struct {
            uint32_t add_set:4;
            uint32_t add_hold:4;
            uint32_t dat_ast:8;
            uint32_t bus_turn:4;
            uint32_t clk_div:4;
            uint32_t dat_lat:4;
            uint32_t acc_mode:2;
            uint32_t reserved:2;
        } bit;
        uint32_t reg;
    } fsmc_btw0;		/* 0x104 */

    uint32_t unused_2;

    union {
        struct {
            uint32_t add_set:4;
            uint32_t add_hold:4;
            uint32_t dat_ast:8;
            uint32_t bus_turn:4;
            uint32_t clk_div:4;
            uint32_t dat_lat:4;
            uint32_t acc_mode:2;
            uint32_t reserved:2;
        } bit;
        uint32_t reg;
    } fsmc_btw1;		/* 0x10c */

    uint32_t unused_3;

    union {
        struct {
            uint32_t add_set:4;
            uint32_t add_hold:4;
            uint32_t dat_ast:8;
            uint32_t bus_turn:4;
            uint32_t clk_div:4;
            uint32_t dat_lat:4;
            uint32_t acc_mode:2;
            uint32_t reserved:2;
        } bit;
        uint32_t reg;
    } fsmc_btw2;		/* 0x114 */

} fmsc_regs_t;

#define fsmc_regs ((fmsc_regs_t *) FSMC_BASE)

/**
 * struct nand_context - define current NAND context
 * @man_id: manufacturer id
 * @data_bytes_per_sector: number of data bytes per sector
 * @spare_bytes_per_sector: number of spare bytes per sector
 * @sectors_per_block: number of sectors per block
 * @block_to_sector_factor: block to sector factor
 * @has_read_cache: has read page cache support
 * @bad_blocks: number of bad blocks
 */
struct nand_context {
	uint8_t man_id;
	uint32_t data_bytes_per_sector;
	uint32_t spare_bytes_per_sector;
	uint32_t sectors_per_block;
	uint8_t block_to_sector_factor;
	bool has_read_cache;
	int bad_blocks;
};

static struct nand_context ctx;

/**************************************************************************/

/**
 * @brief	Return the size of a NAND page
 * @param	size: Size in bytes
 * @return	the size in bytes
 */
uint32_t nand_page_size(void)
{
	return ctx.data_bytes_per_sector;
}

/**
 * @brief	Return the size rounded up in NAND page size
 * @param	size: Size in bytes
 * @return	the size in page/sector
 */
uint32_t nand_size_in_pages(uint32_t size)
{
	uint32_t nand_page_size = ctx.data_bytes_per_sector;

	return (size + nand_page_size - 1) / nand_page_size;
}

/**
 * @brief	Initialize and execute a NAND command with a given command id.
 * @param	cmd_id: the NAND command  identifier to be initialized and then executed.
 * @return	None
 */
static inline void nand_simple_cmd(uint8_t cmd_id)
{
	mmio_write_8(NAND_CS0_BASE + NAND_CS0_CMD_OFFSET, cmd_id);
}

/**
 * @brief	Initialize and execute a NAND command with a given command id.
 * @param	cmd_id: the NAND command  identifier to be initialized and then executed.
 * @param	ca: column byte address
 * @return	uint32_t result
 */
static inline uint32_t nand_simple_cmd_ca(uint8_t cmd_id, uint8_t ca)
{
	mmio_write_8(NAND_CS0_BASE + NAND_CS0_CMD_OFFSET, cmd_id);
	mmio_write_8(NAND_CS0_BASE + NAND_CS0_COMMON_AREA_OFFSET, ca);

	return mmio_read_32(NAND_CS0_BASE);
}

/**
 * @brief	execute READ DATA command: sends the column address (on 2
 * bytes) using the 2 required cycles.
 * @param	ca: the column address to be used
 * @return	NA
 */
static void nand_cmd_exec_data_read(uint16_t ca)
{
	mmio_write_8(NAND_CS0_BASE + NAND_CS0_CMD_OFFSET,
			NAND_CMD_RANDOM_DATA_READ_START);

	/* write the 3 bytes containing the page address */
	mmio_write_8(NAND_CS0_BASE + NAND_CS0_COMMON_AREA_OFFSET, ca & 0xFF);
	mmio_write_8(NAND_CS0_BASE + NAND_CS0_COMMON_AREA_OFFSET, ca >> 8);

	mmio_write_8(NAND_CS0_BASE + NAND_CS0_CMD_OFFSET,
			NAND_CMD_RANDOM_DATA_READ_END);
}

/**
 * @brief	read the entire ECC code (13 bytes), then check for errors
 * @return	0 if no ECC errors, > 0 if ECC errors are detected, < 0 for other
 * errors
 */
static int nand_get_err_calc(void)
{
	int i;
	int res = 0;
	uint32_t t[4];

	/* Read ECC code 12 bytes (3 words) */
	for (i = 0; i < 3; i++) {
		t[i] = mmio_read_32(NAND_CS0_BASE);
		if (t[i] != 0xFFFFFFFF)
			res++;
	}

	/* Read last byte */
	if (mmio_read_8(NAND_CS0_BASE) != 0xFF)
		res++;

	return res;
}

/**
 * @brief	Read the syndrome (location of errors) and try
 *		to correct data on a given buffer.
 * @param	buf: data buffer pointer
 * @param	err_cnt: error counter
 * @return	0 if no error, not 0 otherwise
 */
static int nand_correct_data(uint8_t *buf, uint32_t err_cnt)
{
	unsigned int	i;
	uint16_t err_pos = 0;
	uint16_t byte2change = 0;
	uint16_t byte2invert = 0;

	static const uint8_t flip[8] = {3, 2, 1, 0, 7, 6, 5, 4};

	if (err_cnt > 8)
		return -EINVAL;

	for (i = 1; i <= err_cnt; i++) {
		switch (i) {
		case 1:
			err_pos = FSMC_GET_BITS(fsmc_regs->fsmc_eccr0[0],
					FSMC_ECCR_ERROR1_SHIFT,
					FSMC_ECCR_ERROR1);
			break;
		case 2:
			err_pos = FSMC_GET_BITS(fsmc_regs->fsmc_eccr0[0],
					FSMC_ECCR_ERROR2_SHIFT,
					FSMC_ECCR_ERROR2);
			break;
		case 3:
			err_pos = FSMC_GET_BITS(fsmc_regs->fsmc_eccr0[0],
					FSMC_ECCR_ERROR3_SHIFT,
					FSMC_ECCR_ERROR3) |
					(FSMC_GET_BITS(fsmc_regs->fsmc_eccr0[1],
					FSMC_ECC2R_ERROR3_SHIFT,
					FSMC_ECC2R_ERROR3) << 6);
			break;
		case 4:
			err_pos = FSMC_GET_BITS(fsmc_regs->fsmc_eccr0[1],
					FSMC_ECC2R_ERROR4_SHIFT,
					FSMC_ECC2R_ERROR4);
			break;
		case 5:
			err_pos = FSMC_GET_BITS(fsmc_regs->fsmc_eccr0[1],
					FSMC_ECC2R_ERROR5_SHIFT,
					FSMC_ECC2R_ERROR5) |
					(FSMC_GET_BITS(fsmc_regs->fsmc_eccr0[1],
					FSMC_ECC3R_ERROR5_SHIFT,
					FSMC_ECC3R_ERROR5) << 12);
			break;
		case 6:
			err_pos = FSMC_GET_BITS(fsmc_regs->fsmc_eccr0[2],
					FSMC_ECC3R_ERROR6_SHIFT,
					FSMC_ECC3R_ERROR6);
			break;
		case 7:
			err_pos = FSMC_GET_BITS(fsmc_regs->fsmc_eccr0[2],
					FSMC_ECC3R_ERROR7_SHIFT,
					FSMC_ECC3R_ERROR7);
			break;
		case 8:
			err_pos = FSMC_GET_BITS(fsmc_regs->fsmc_eccr0[2],
					FSMC_ECC3R_ERROR8_SHIFT,
					FSMC_ECC3R_ERROR8);
			err_pos |= FSMC_GET_BITS(fsmc_regs->fsmc_psr0.reg,
					FSMC_PISR_ERROR8_SHIFT,
					FSMC_PISR_ERROR8) << 15;
			break;
		default:
			break;
		}

		/* Skip the inversion if the bit_error_SHIFTition is for ECC code */
		if (err_pos <= 4095) {
			/* Byte to change */
			byte2change = err_pos / 8;
			/* Bit into previous byte to invert */
			byte2invert = err_pos % 8;
			/* Check the bit value */
			if ((buf[byte2change] & BIT(flip[byte2invert])) == 0) {
				/* Inversion 0->1 */
				buf[byte2change] = buf[byte2change]
						| BIT(flip[byte2invert]);
			} else {
				/* Inversion 1->0 */
				buf[byte2change] = buf[byte2change]
						& ~BIT(flip[byte2invert]);
			}
		}
		err_pos = 0;
	}

	return 0;
}

/**
 * @brief	detect error and try to correct them if any
 * @param	buf: the data buffer pointer
 * @return	0 if no error, not 0 otherwise
 */
static int nand_check_and_correct_errors(uint8_t *buf)
{
	uint32_t reg = fsmc_regs->fsmc_psr0.reg;
	uint8_t err_cnt = (reg >> PSR_ERRFOUND) & PSR_ERRFOUND_MASK;

	/* no error, everything is ok */
	if (err_cnt == 0)
		return 0;

	/* Let's try to correct if possible up to 8 errors can be rectified */
	if (err_cnt > NAND_MAX_ERROR_TO_CORRECT)
		return -ENOENT;

	/* Try to correct */
	return nand_correct_data(buf, err_cnt);
}

static inline uint16_t get_ecc_column_address(uint32_t cnt)
{
	return (uint16_t)(ctx.data_bytes_per_sector + (cnt * 16) + 2);
}

static void nand_read_lp_cmd(uint16_t ca, uint32_t pa)
{
	mmio_write_8(NAND_CS0_BASE + NAND_CS0_CMD_OFFSET, NAND_CMD_READ);
	mmio_write_8(NAND_CS0_BASE + NAND_CS0_COMMON_AREA_OFFSET, ca & 0xFF);
	mmio_write_8(NAND_CS0_BASE + NAND_CS0_COMMON_AREA_OFFSET, ca >> 8);

	/* write the 3 bytes containing the page address */
	mmio_write_8(NAND_CS0_BASE + NAND_CS0_COMMON_AREA_OFFSET, pa & 0xFF);
	mmio_write_8(NAND_CS0_BASE + NAND_CS0_COMMON_AREA_OFFSET,
			(pa & 0xFF00) >> 8);
	mmio_write_8(NAND_CS0_BASE + NAND_CS0_COMMON_AREA_OFFSET,
			(pa & 0xFF0000) >> 16);
	mmio_write_8(NAND_CS0_BASE + NAND_CS0_CMD_OFFSET, NAND_CMD_READ_PAGE);
}

/**
 * @brief	execute READ PAGE command: sends the page address (on 3
 * bytes) using the 5 required cycles (2 dummy cycles first), then check the
 * ECC and try to correct errors if any.
 * @param	pa: the page address to be used
 * @param	buf: the buffer to put read data
 * @param	size: the buffer size
 * @param	total_byte: the total counter of bytes that have been read so far
 * @return	0 if no error, not 0 otherwise
 */
static int nand_cmd_exec_read_page(uint32_t pa,
		uint32_t *buf, uint32_t size, uint32_t *total_byte)
{
	bool err = 0;
	uint32_t cnt = 0;
	uint32_t cnt_max = ctx.data_bytes_per_sector / 512;
	uint32_t *tmpbuf = NULL;
	static uint32_t temp_512bytes_buf[512 / 4];

	if (ctx.has_read_cache) {
		nand_simple_cmd(NAND_CMD_READ_CACHE);
	} else {
		nand_read_lp_cmd(0, pa);
	}

	while (cnt < cnt_max && (*total_byte < size)) {
		tmpbuf = buf + (*total_byte >> 2);

		/* Clear CodeReady status bit */
		fsmc_regs->fsmc_psr0.bit.code_ready = 0;
		fsmc_regs->fsmc_pcr0.bit.ecc_enable = 0;
		fsmc_regs->fsmc_pcr0.bit.ecc_enable = 1;

		if (size - (*total_byte) < 512)
			tmpbuf = temp_512bytes_buf;

		/* Read 512 Bytes of data (128 WORDS) */
		/* Store the values into the array passed as parameter */
		nand_fast_read_512(tmpbuf, (uint32_t *)NAND_CS0_BASE);

		/* Jump ahead to correspondent spare area location,
		 * to get ECC code */
		nand_cmd_exec_data_read(get_ecc_column_address(cnt));

		/* Increase run counter (number of sub_block read within the block) */
		cnt++;

		/* Check for errors */
		err = nand_get_err_calc();

		/* Jump back to data */
		nand_cmd_exec_data_read(cnt * 512);

		/* Find out the number of errors, waiting for code ready */
		while (!fsmc_regs->fsmc_psr0.bit.code_ready)
			;

		if (err) {
			/* Look for errors and correct them,
			 * if any and if possible */
			err = nand_check_and_correct_errors((uint8_t *)tmpbuf);
			if (err)
				return err;
		}

		if (size - (*total_byte) < 512) {
			memcpy(buf + (*total_byte >> 2), tmpbuf,
					size - (*total_byte));
			*total_byte = size;
		}
		else {
			/* 512 bytes has been read, increase total_byte global counter */
			*total_byte += 512;
		}
	}

	return err;
}

/**
 * @brief	returns the block the sector belongs to
 * @param	sector number
 * @return	block number
 */
static inline uint32_t sector_to_block(uint32_t sector)
{
	return (sector >> ctx.block_to_sector_factor);
}

/**
 * @brief	Provide the NAND manufacturer identified
 * @param	none
 * @return	The manufacturer identified
 */
static inline uint8_t nand_get_manufacturer_id(void)
{
	return (uint8_t)nand_simple_cmd_ca(NAND_CMD_READ_ID, 0);
}

/**
 * @brief	Read ONFI parameters and update information accordingly
 * @param	none
 * @return	0 if no error, not 0 otherwise
 */
static int nand_read_onfi_params(void)
{
	unsigned int i;
	uint8_t onfi_params[96];

	/* ONFI supported? */
	if (nand_simple_cmd_ca(NAND_CMD_READ_ID, 0x20) == (('O' << 24) |
							   ('N' << 16) |
							   ('F' << 8) | 'I'))
		return -1;
	else
		VERBOSE("ONFI Nand\n");

	/* Write the specific command for reading O.N.F.I. params */
	mmio_write_8(NAND_CS0_BASE + NAND_CS0_CMD_OFFSET, NAND_CMD_READ_PARAM);
	mmio_write_8(NAND_CS0_BASE + NAND_CS0_COMMON_AREA_OFFSET, 0x0);

	/* Read ONFI parameters from offset 0 to 95 */
	for (i = 0; i < sizeof(onfi_params); i++)
		onfi_params[i] = mmio_read_8(NAND_CS0_BASE);

	/* Support read page cache ? */
	if ((onfi_params[8] | (onfi_params[9] << 8)) &
	    ONFI_OPT_CMD_READ_CACHE_CMDS) {
		INFO("Read page cache support\n");
		ctx.has_read_cache = true;
	}

	/* Get data bytes per sector (bytes 80 to 83) */
	ctx.data_bytes_per_sector = (onfi_params[83] << 24)
		| (onfi_params[82] << 16) | (onfi_params[81] << 8)
		| onfi_params[80];

	/* Get spare bytes per sector (bytes 84 to 85) */
	ctx.spare_bytes_per_sector = (onfi_params[85] << 8) | onfi_params[84];

	/* Get sectors per block (bytes 92 to 95) */
	ctx.sectors_per_block = (onfi_params[95] << 24)
		| (onfi_params[94] << 16) | (onfi_params[93] << 8)
		| onfi_params[92];

	return 0;
}

/**
 * @brief	Check if NAND device is present, if true, get the manufacturer ID
 * and set parameters accordingly
 * @param	None
 * @return	0 if no error, not 0 otherwise
 */
static int nand_scan(void)
{
	nand_simple_cmd(NAND_CMD_RESET);

	/* Get manufacturer id */
	ctx.man_id = nand_get_manufacturer_id();

	/* Test ONFI support */
	if (nand_read_onfi_params()) {
		/* ONFI not supported fallback to default values */
		ctx.data_bytes_per_sector = NAND_DATA_BYTES_PER_SECTOR;
		ctx.spare_bytes_per_sector = NAND_SPARE_BYTES_PER_SECTOR;
		ctx.sectors_per_block = NAND_SECTORS_PER_BLOCK;
	}

	NOTICE("Nand ID: %x, pg_sz=%d, oob_sz=%d, blk_sz=%d\n",
	       ctx.man_id, ctx.data_bytes_per_sector,
	       ctx.spare_bytes_per_sector,
	       NAND_BLOCK_SIZE);

	return 0;
}

/**
 * @brief	Init the NAND device for NAND Chip Select 0
 * @param	muxed_with_sqi: true if nand is muxed with sqi, false otherwise
 * @return	0 if no error, not 0 otherwise
 */
static int nand_init(void)
{
	int err;

	while (!hsem_trylock(HSEM_NAND_ID, 0))
		;

	/* Reset Status Register */
	fsmc_regs->fsmc_psr0.reg &= (~BIT(PSR_INTRRE)
			& ~BIT(PSR_INTRLVL) & ~BIT(PSR_INTRFE)
			& ~BIT(PSR_ENRE) & ~BIT(PSR_ENLVL)
			& ~BIT(PSR_ENFE) & ~BIT(PSR_CODE_RDY));

	/* Disable all the other CS that we do not use (NOR) */
	fsmc_regs->fsmc_bcr0.reg = BIT(FWPRLVL);
	fsmc_regs->fsmc_bcr1.reg = BIT(FWPRLVL);

	/* Configure for 8x NAND device usage */
	fsmc_regs->fsmc_pcr0.reg = BIT(PWAITEN) | BIT(PTYP);
	fsmc_regs->fsmc_pcr0.bit.enable = 1;

	/* Set NAND common area timings */
	fsmc_regs->fsmc_mem0.reg = (NAND_TIMING_HOLD << MEMXHOLD_SHIFT)
		| (NAND_TIMING_WAIT << MEMXWAIT_SHIFT) | NAND_TIMING_SET;

	err = nand_scan();
	if (err)
		goto end;

	ctx.block_to_sector_factor = ffs(ctx.sectors_per_block) - 1;

	nand_simple_cmd(NAND_CMD_RESET);

	/* Enable ECC logic */
	fsmc_regs->fsmc_pcr0.bit.ecc_enable = 1;

end:
	hsem_unlock(HSEM_NAND_ID);

	if (err)
		ERROR("%s: failed to intitialize\n", __func__);
	return err;
}

/**
 * @brief	get the status from one block
 * @param	ba - block to get status about
 * @return  true if block address points to a bad block, false otherwise
 */
static bool nand_is_bad_block(uint32_t ba)
{
	bool is_bad;
	uint32_t pa = (ba << ctx.block_to_sector_factor);

	nand_read_lp_cmd(ctx.data_bytes_per_sector, pa);

	is_bad = (mmio_read_8(NAND_CS0_BASE) != 0xFF);
	if (is_bad)
		NOTICE("%s: Warning: Block %d is BAD\n", __func__, ba);

	return is_bad;
}

/**
 * @brief	Read NAND pages depending on the input size parameters
 * @param	sector_address: starting nand page address
 * @param	buf: data buffer pointer
 * @param	size: size in bytes
 * @param	size_max: maximum size in bytes (partition size)
 * @return	0 if no error, not 0 otherwise
 */
static int nand_read_pages(uint32_t sector_address, uint32_t *buf,
		uint32_t size, uint32_t size_max)
{
	int err = 0;
	uint32_t total_byte = 0;
	uint32_t nb_sectors;
	uint32_t sector_max;
	bool restart_cache_reading;

	sector_max = sector_address +
		((size_max + ctx.data_bytes_per_sector - 1)
			/ ctx.data_bytes_per_sector);

	nb_sectors = (size + ctx.data_bytes_per_sector - 1)
				/ ctx.data_bytes_per_sector;

	for (; sector_address < sector_max && nb_sectors;
			sector_address++, nb_sectors--) {
		/* 1st sector to flash or 1st sector of the current block? */
		if ((sector_address % ctx.sectors_per_block == 0)
				|| (total_byte == 0)) {
			if (ctx.has_read_cache)
				restart_cache_reading = true;

			while (nand_is_bad_block(sector_to_block(sector_address))) {
				/*
				 * Every time we meet a bad block,
				 * jump to next one
				 */
				sector_address += ctx.sectors_per_block;
				if (sector_address > sector_max) {
					err = -ENOMEM;
					goto end;
				}
			}
		}

		if (restart_cache_reading) {
			nand_read_lp_cmd(0, sector_address);
			restart_cache_reading = false;
		}

		/* Send read command to required address */
		err = nand_cmd_exec_read_page(sector_address, buf, size, &total_byte);
		if (err)
			goto end;
	}

end:
	if (ctx.has_read_cache)
		nand_simple_cmd(NAND_CMD_READ_CACHE_END);

	if (err)
		ERROR("%s: failed to read (%d)\n", __func__, err);

	return err;
}

/*
 * Minimal IO block device interfaces
 */

/**
 * @brief  Read data
 * @param  pa: page address to read
 * @param  buf: output read buffer pointer
 * @param  length: buffer length
 * @return length if no error, < 0 otherwise
 */
static int sta_nand_read_pages(int pa, uintptr_t buf, size_t length)
{
	int ret;

	while (!hsem_trylock(HSEM_NAND_ID, 0))
		;
	ret = nand_read_pages(pa, (uint32_t *)buf, length, length
				+ NAND_BLOCK_SIZE);
	hsem_unlock(HSEM_NAND_ID);

	if (ret)
		return 0;
	else
		return length;
}

static const nand_flash_ops_t sta_nand_ops = {
	.init		= nand_init,
	.read		= sta_nand_read_pages,
};

static const nand_flash_ops_t *ops;

size_t nand_flash_read_blocks(int lba, uintptr_t buf, size_t size)
{
	assert((ops != 0) &&
	       (ops->read != 0) &&
	       ((buf & 3) == 0));

	inv_dcache_range(buf, size);

	return ops->read(lba, buf, size);
}

static int nand_flash_init(const nand_flash_ops_t *ops_ptr)
{
	assert((ops_ptr != 0) &&
	       (ops_ptr->init != 0) &&
	       (ops_ptr->read != 0));
	ops = ops_ptr;

	return ops_ptr->init();
}

int sta_nand_init(void)
{
	return nand_flash_init(&sta_nand_ops);
}

