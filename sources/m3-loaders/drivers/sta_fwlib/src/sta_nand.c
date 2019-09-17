/**
 * @file sta_nand.c
 * @brief This file provides all the NAND firmware functions
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include <errno.h>
#include <string.h>

#include "sta_map.h"
#include "sta_nand.h"
#include "sta_common.h"
#include "sta_nvic.h"
#include "trace.h"
#include "sta_src.h"
#include "sta_pinmux.h"
#include "sta_hsem.h"

extern void nand_fast_read_512(uint32_t *buf, uint32_t *nand_base);

/* CSS M3 Registers offsets */
#define CSS_MISC_M3			0xC0

#define CSS_MISC_SQI_SEL		0
#define CSS_MISC_FSMC_SEL		1

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
#define MEMXSET_MASK			0xFFFFFF00
#define MEMXWAIT_MASK			0xFFFF00FF
#define MEMXHOLD_MASK			0xFF00FFFF
#define MEMXHIZ_MASK			0x00FFFFFF
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

/* NAND Registers Offsets */
#define NAND_CS0_CMD_OFFSET		0x10000
#define NAND_CS0_COMMON_AREA_OFFSET	0x20000

#define NAND_MANUF_ID_MASK		0xFF

#define NAND_ONFI_PARAM_LENGTH		(256 / sizeof(uint32_t))
/* ONFI optional Read Cache commands supported? */
#define ONFI_OPT_CMD_READ_CACHE_CMDS	(1 << 1)

#define NAND_MAX_ERROR_TO_CORRECT	8

/**
 * struct nand_context - define current NAND context
 * @man_id: manufacturer id
 * @data_bytes_per_sector: number of data bytes per sector
 * @spare_bytes_per_sector: number of spare bytes per sector
 * @sectors_per_block: number of sectors per block
 * @block_to_sector_factor: block to sector factor
 * @has_read_cache: has read page cache support
 * @bad_blocks: number of bad blocks
 * @hsem: Hardware semaphore
 */
struct nand_context {
	uint8_t man_id;
	uint32_t data_bytes_per_sector;
	uint32_t spare_bytes_per_sector;
	uint32_t sectors_per_block;
	uint8_t block_to_sector_factor;
	bool has_read_cache;
	int bad_blocks;
	struct hsem_lock *hsem;
};

static struct nand_context ctx;

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
 * @param	ca: column byte address
 * @return	uint32_t result
 */
static inline uint32_t nand_simple_cmd_ca(uint8_t cmd_id, uint8_t ca)
{
	write_byte_reg(cmd_id, NAND_CS0_BASE + NAND_CS0_CMD_OFFSET);
	write_byte_reg(ca, NAND_CS0_BASE + NAND_CS0_COMMON_AREA_OFFSET);

	return read_reg(NAND_CS0_BASE);
}

/**
 * @brief	Initialize and execute a NAND command with a given command id.
 * @param	cmd_id: the NAND command  identifier to be initialized and then executed.
 * @return	None
 */
static inline void nand_simple_cmd(uint8_t cmd_id)
{
	write_byte_reg(cmd_id, NAND_CS0_BASE + NAND_CS0_CMD_OFFSET);
}

/**
 * @brief	execute READ DATA command: sends the column address (on 2
 * bytes) using the 2 required cycles.
 * @param	ca: the column address to be used
 * @return	NA
 */
static void nand_cmd_exec_data_read(uint16_t ca)
{
	write_byte_reg(NAND_CMD_RANDOM_DATA_READ_START,
			NAND_CS0_BASE + NAND_CS0_CMD_OFFSET);

	/* write the 2 bytes containing the column address */
	write_byte_reg(ca & 0xFF, NAND_CS0_BASE + NAND_CS0_COMMON_AREA_OFFSET);
	write_byte_reg((ca & 0xFF00) >> 8,
		       NAND_CS0_BASE + NAND_CS0_COMMON_AREA_OFFSET);

	write_byte_reg(NAND_CMD_RANDOM_DATA_READ_END,
			NAND_CS0_BASE + NAND_CS0_CMD_OFFSET);
}

/**
 * @brief read the entire ECC code (13 bytes) and check if all bits are '1'.
 *	  an erased page will contain '1' in all ECC bits.
 * @return 1 if page is erased, 0 otherwise
 * errors
 */
static int nand_get_erased_page_indication(void)
{
	int i;
	int res = 0;

	/* Read first 12 bytes of ECC code (3 words) */
	for (i = 0; i < 3; i++)
		if (read_reg(NAND_CS0_BASE) != 0xFFFFFFFF)
			res++;

	/* Read last byte */
	if (read_byte_reg(NAND_CS0_BASE) != 0xFF)
		res++;

	/* Page is erased if res was not incremented */
	return !res;
}

/**
 * @brief	Read the syndrome (location of errors) and try to correct data on a
 * given buffer.
 * @param	buff: data buffer pointer
 * @param	err_cnt: error counter
 * @return	0 if no error, not 0 otherwise
 */
static int nand_correct_data(uint8_t *buff, uint32_t err_cnt)
{
	unsigned int	i;
	uint16_t err_pos = 0;
	uint16_t byte_to_change = 0;
	uint16_t bit_to_invert = 0;

	static const uint8_t flip[8] = {3,2,1,0,7,6,5,4};

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
						FSMC_ECCR_ERROR3);
			err_pos |= FSMC_GET_BITS(fsmc_regs->fsmc_eccr0[1],
						 FSMC_ECC2R_ERROR3_SHIFT,
						 FSMC_ECC2R_ERROR3) << 6;
			break;
		case 4:
			err_pos = FSMC_GET_BITS(fsmc_regs->fsmc_eccr0[1],
						FSMC_ECC2R_ERROR4_SHIFT,
						FSMC_ECC2R_ERROR4);
			break;
		case 5:
			err_pos = FSMC_GET_BITS(fsmc_regs->fsmc_eccr0[1],
						FSMC_ECC2R_ERROR5_SHIFT,
						FSMC_ECC2R_ERROR5);
			err_pos |= FSMC_GET_BITS(fsmc_regs->fsmc_eccr0[1],
						 FSMC_ECC3R_ERROR5_SHIFT,
						 FSMC_ECC3R_ERROR5) << 12;
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
						 FSMC_PISR_ERROR8) << 5;
			break;
		default:
			break;
		}

		/* Skip the inversion if the bit_error_position is for ECC code */
		if (err_pos <= 4095) {
			/* Byte to change */
			byte_to_change = err_pos / 8;
			/* Bit into previous byte to invert */
			bit_to_invert = err_pos % 8;
			/* Check the bit value: Inversion 1->0 ? */
			if (buff[byte_to_change] & BIT(flip[bit_to_invert]))
				buff[byte_to_change] &= ~BIT(flip[bit_to_invert]);
			else /* Inversion 0->1 */
				buff[byte_to_change] |= BIT(flip[bit_to_invert]);
		}
		err_pos = 0;
	}

	return 0;
}

/**
 * @brief	detect error and try to correct them if any
 * @param	buff: the data buffer pointer
 * @return	0 if no error, not 0 otherwise
 */
static int nand_check_and_correct_errors(uint8_t *buff)
{
	int ret;
	uint32_t reg = fsmc_regs->fsmc_psr0.reg;
	uint8_t err_cnt = (reg >> PSR_ERRFOUND) & PSR_ERRFOUND_MASK;

	/* no error, everything is ok */
	if (err_cnt == 0)
		return 0;

	/* Let's try to correct, if possible, up to 8 errors can be rectified */
	if (err_cnt > NAND_MAX_ERROR_TO_CORRECT) {
		TRACE_ERR("ECC error # is larger than maximum err_cnt:%u\n",
			  err_cnt);
		return -ENOENT;
	}

	/* Try to correct */
	ret = nand_correct_data(buff, err_cnt);
	TRACE_INFO("%u ECC errors has been corrected\n", err_cnt);

	return ret;
}

static inline uint16_t get_ecc_column_address(uint32_t cnt)
{
	return (uint16_t)(ctx.data_bytes_per_sector + (cnt * 16) + 2);
}

static void nand_read_lp_cmd(uint16_t ca, uint32_t pa)
{
	write_byte_reg(NAND_CMD_READ, NAND_CS0_BASE + NAND_CS0_CMD_OFFSET);
	/* Write column offset */
	write_byte_reg(ca & 0xFF, NAND_CS0_BASE + NAND_CS0_COMMON_AREA_OFFSET);
	write_byte_reg(ca >> 8, NAND_CS0_BASE + NAND_CS0_COMMON_AREA_OFFSET);

	/* write the 3 bytes containing the page address */
	write_byte_reg(pa & 0xFF, NAND_CS0_BASE + NAND_CS0_COMMON_AREA_OFFSET);
	write_byte_reg((pa & 0xFF00) >> 8,
			NAND_CS0_BASE + NAND_CS0_COMMON_AREA_OFFSET);
	write_byte_reg((pa & 0xFF0000) >> 16,
			NAND_CS0_BASE + NAND_CS0_COMMON_AREA_OFFSET);

	write_byte_reg(NAND_CMD_READ_PAGE, NAND_CS0_BASE + NAND_CS0_CMD_OFFSET);
}

/**
 * @brief	execute READ PAGE command: sends the page address (on 3
 * bytes) using the 5 required cycles (2 dummy cycles first), then check the
 * ECC and try to correct errors if any.
 * @param	pa: the page address to be used
 * @param	buff: the buffer to put read data
 * @param	size: the buffer size
 * @param	total_byte: the total counter of bytes that have been read so far
 * @return	0 if no error, not 0 otherwise
 */
static int nand_cmd_exec_read_page(uint32_t pa,
		uint32_t *buff, uint32_t size, uint32_t *total_byte)
{
	bool ret = 0;
	bool is_erased_page = 0;
	bool tmpalloc = 0;
	uint32_t cnt = 0;
	uint32_t cnt_max = ctx.data_bytes_per_sector / 512;
	uint32_t *tmpbuff = NULL;

	if (ctx.has_read_cache)
		nand_simple_cmd(NAND_CMD_READ_CACHE);
	else
		nand_read_lp_cmd(0, pa);

	while (cnt < cnt_max && (*total_byte < size)) {
		tmpbuff = buff + (*total_byte >> 2);

		/* Clear CodeReady status bit */
		fsmc_regs->fsmc_psr0.bit.code_ready = 0;
		fsmc_regs->fsmc_pcr0.bit.ecc_enable = 0;
		fsmc_regs->fsmc_pcr0.bit.ecc_enable = 1;

		if (size - (*total_byte) < 512) {
			tmpbuff = pvPortMalloc(512);
			tmpalloc = 1;
		}

		/* Read 512 Bytes of data (128 WORDS) */
		/* Store the values into the array passed as parameter */
		nand_fast_read_512(tmpbuff, (uint32_t *)NAND_CS0_BASE);

		/* Jump ahead to correspondent spare area location, to get ECC code */
		nand_cmd_exec_data_read(get_ecc_column_address(cnt));

		/* Increase run counter (number of sub_block read within the block) */
		cnt++;

		/* Check if page is erased */
		is_erased_page = nand_get_erased_page_indication();

		/* Jump back to data */
		nand_cmd_exec_data_read(cnt * 512);

		/* Find out the number of errors, waiting for code ready */
		while (!fsmc_regs->fsmc_psr0.bit.code_ready)
			;

		/* In case page is erased, no need to check and correct errors */
		if (!is_erased_page) {
			/* Look for errors and correct them, if any and if possible */
			ret = nand_check_and_correct_errors((uint8_t *)tmpbuff);
			if (ret)
				goto end;
		}

		if (size - (*total_byte) < 512) {
			memcpy(buff + (*total_byte >> 2), tmpbuff,
			       size - (*total_byte));
			*total_byte = size;
			goto end;
		} else {
			/* 512 bytes has been read, increase total_byte global counter */
			*total_byte += 512;
		}
	}

end:
	if (tmpalloc)
		vPortFree(tmpbuff);
	return ret;
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
		TRACE_INFO("ONFI Nand\n");

	/* Write the specific command for reading O.N.F.I. params */
	write_byte_reg(NAND_CMD_READ_PARAM, NAND_CS0_BASE + NAND_CS0_CMD_OFFSET);
	write_byte_reg(0x0, NAND_CS0_BASE + NAND_CS0_COMMON_AREA_OFFSET);

	/* Read ONFI parameters from offset 0 to 95 */
	for (i = 0; i < sizeof(onfi_params); i++)
		onfi_params[i] = read_byte_reg(NAND_CS0_BASE);

	/* Support read page cache ? */
	if ((onfi_params[8] | (onfi_params[9] << 8)) &
	    ONFI_OPT_CMD_READ_CACHE_CMDS) {
		TRACE_INFO("Read page cache support\n");
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

	trace_printf("Nand ID: %x, pg_sz=%d, oob_sz=%d, blk_sz=%d\n",
		     ctx.man_id, ctx.data_bytes_per_sector,
		     ctx.spare_bytes_per_sector,
		     ctx.sectors_per_block * ctx.data_bytes_per_sector);

	return 0;
}

/**
 * @brief	Return the NAND status
 * @param	None
 * @return	the NAND status
 */
static inline uint32_t nand_read_status(void)
{
	nand_simple_cmd(NAND_CMD_STATUS);
	return read_reg(NAND_CS0_BASE);
}

/**
 * @brief	Init the NAND device for NAND Chip Select 0
 * @param	None
 * @return	0 if no error, not 0 otherwise
 */
int nand_init(void)
{
	int err = -1;
	struct hsem_lock_req lock;

	pinmux_request("nand_mux");

	lock.lock_cb = NULL;
	lock.id = HSEM_NAND_ID;
	ctx.hsem = hsem_lock_request(&lock);
	if (!ctx.hsem)
		goto end;

	hsem_lock(ctx.hsem);

	/* Reset Status Register */
	fsmc_regs->fsmc_psr0.reg &= ~BIT(PSR_INTRRE) & ~BIT(PSR_INTRLVL)
		& ~BIT(PSR_INTRFE) & ~BIT(PSR_ENRE)
		& ~BIT(PSR_ENLVL) & ~BIT(PSR_ENFE)
		& ~BIT(PSR_CODE_RDY);

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
		goto release_hsem;

	ctx.block_to_sector_factor = ffs(ctx.sectors_per_block) - 1;

	nand_simple_cmd(NAND_CMD_RESET);

	/* Enable ECC logic */
	fsmc_regs->fsmc_pcr0.bit.ecc_enable = 1;

release_hsem:
	hsem_unlock(ctx.hsem);
end:
	if (err)
		TRACE_ERR("%s: failed to intitialize (%d)\n", __func__, err);
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

	is_bad = (read_byte_reg(NAND_CS0_BASE) != 0xFF);
	if (is_bad)
		TRACE_INFO("%s: Warn: Block %d is BAD\n", __func__, ba);

	return is_bad;
}

/**
 * @brief	Read NAND pages depending on the input size parameters
 * @param	sector_address: starting nand page address
 * @param	buff: data buffer pointer
 * @param	size: size in bytes
 * @param	size_max: maximum size in bytes (partition size)
 * @return	0 if no error, not 0 otherwise
 */
int nand_read_pages(uint32_t sector_address, uint32_t *buff,
		uint32_t size, uint32_t size_max)
{
	int err = 0;
	uint32_t total_byte = 0;
	uint32_t nb_sectors;
	uint32_t sector_max;
	bool restart_cache_reading;

	sector_max = sector_address + ((size_max + ctx.data_bytes_per_sector - 1)
				       / ctx.data_bytes_per_sector);

	nb_sectors = (size + ctx.data_bytes_per_sector - 1)
		/ ctx.data_bytes_per_sector;

	hsem_lock(ctx.hsem);

	for (; sector_address < sector_max && nb_sectors;
	     sector_address++, nb_sectors--) {

		/* 1st sector to flash or 1st sector of the current block? */
		if ((sector_address % ctx.sectors_per_block == 0)
		    || (total_byte == 0)) {
			if (ctx.has_read_cache)
				restart_cache_reading = true;

			while (nand_is_bad_block(sector_to_block(sector_address))) {
				/* Every time we meet a bad block, jump to next one */
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
		err = nand_cmd_exec_read_page(sector_address, buff, size, &total_byte);
		if (err)
			goto end;
	}

end:
	if (ctx.has_read_cache)
		nand_simple_cmd(NAND_CMD_READ_CACHE_END);

	hsem_unlock(ctx.hsem);
	if (err)
		TRACE_ERR("%s: failed to read (%d)\n", __func__, err);

	return err;
}

/**
 * @brief	erase NAND block by calling ERASE BLOCK command: sends the block
 * address (on 3 bytes) data in the common area
 * @param	ba: the block address to be erased
 * @return	0 if no error, not 0 otherwise
 */
int nand_erase_block(uint32_t ba)
{
	uint32_t pa = (ba << ctx.block_to_sector_factor);

	hsem_lock(ctx.hsem);

	write_byte_reg(NAND_CMD_ERASE_BLOCK_START,
		       NAND_CS0_BASE + NAND_CS0_CMD_OFFSET);
	write_byte_reg((uint8_t)pa,
		       NAND_CS0_BASE + NAND_CS0_COMMON_AREA_OFFSET);
	write_byte_reg((pa & 0xFF00) >> 8,
		       NAND_CS0_BASE + NAND_CS0_COMMON_AREA_OFFSET);
	write_byte_reg((pa & 0xFF0000) >> 16,
		       NAND_CS0_BASE + NAND_CS0_COMMON_AREA_OFFSET);
	write_byte_reg(NAND_CMD_ERASE_BLOCK_END,
		       NAND_CS0_BASE + NAND_CS0_CMD_OFFSET);

	/* Check NAND status */
	while ((nand_read_status() & NAND_STATUS_READY) != NAND_STATUS_READY)
		;
	hsem_unlock(ctx.hsem);

	return 0;
}

/**
 * @brief	execute PROGRAM PAGE command: sends the page address (on 3
 * bytes) using the 5 required cycles (2 dummy cycles first). Then write the
 * data in the common area, and set the ECC value (on 13 bytes) for the given
 * page.
 * @param	pa: the page address to be used
 * @param	buff: data buffer pointer to be written
 * @param	counter: current data counter
 * @return	0 if no error, not 0 otherwise
 */
static int nand_cmd_exec_program_page(uint32_t pa, uint8_t *buff,
		uint32_t *counter)
{
	int i;
	unsigned int data_cnt = ctx.data_bytes_per_sector;
	uint32_t ecc_reg;
	uint8_t ecc[13];

	write_byte_reg(NAND_CMD_PROGRAM_PAGE_START,
		       NAND_CS0_BASE + NAND_CS0_CMD_OFFSET);

	write_byte_reg(0x00, NAND_CS0_BASE + NAND_CS0_COMMON_AREA_OFFSET);
	write_byte_reg(0x00, NAND_CS0_BASE + NAND_CS0_COMMON_AREA_OFFSET);

	/* write the 3 bytes containing the page address */
	write_byte_reg(pa & 0xFF, NAND_CS0_BASE + NAND_CS0_COMMON_AREA_OFFSET);
	write_byte_reg((pa & 0xFF00) >> 8,
		       NAND_CS0_BASE + NAND_CS0_COMMON_AREA_OFFSET);
	write_byte_reg((pa & 0xFF0000) >> 16,
		       NAND_CS0_BASE + NAND_CS0_COMMON_AREA_OFFSET);

	while (data_cnt > 0) {
		fsmc_regs->fsmc_pcr0.bit.ecc_enable = 0;
		fsmc_regs->fsmc_pcr0.bit.ecc_enable = 1;

		/* Write 512 Bytes of data, that is to say 128 words */
		for (i = 0; i < (512 / 4); i++) {
			write_reg(*(uint32_t *)(buff + *counter++), NAND_CS0_BASE);
			data_cnt--;
		}

		/* Wait for the code ready */
		while (!fsmc_regs->fsmc_psr0.bit.code_ready)
			;

		/* Fill in the buffer
		 *  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |  10  |  11  |  12  |  13  |  14
		 *----------------------------------------------------------------------------------------------
		 * reserved  |    ECC register #1    |    ECC register #2    |       ECC register #3     | BCH8
		 */

		ecc_reg = fsmc_regs->fsmc_eccr0[0];
		ecc[0] =  (ecc_reg >> 0) & 0xFF;
		ecc[1] =  (ecc_reg >> 8) & 0xFF;
		ecc[2] =  (ecc_reg >> 16) & 0xFF;
		ecc[3] =  (ecc_reg >> 24) & 0xFF;

		ecc_reg = fsmc_regs->fsmc_eccr0[1];
		ecc[4] =  (ecc_reg >> 0) & 0xFF;
		ecc[5] =  (ecc_reg >> 8) & 0xFF;
		ecc[6] =  (ecc_reg >> 16) & 0xFF;
		ecc[7] =  (ecc_reg >> 24) & 0xFF;

		ecc_reg = fsmc_regs->fsmc_eccr0[2];
		ecc[8] =  (ecc_reg >> 0) & 0xFF;
		ecc[9] =  (ecc_reg >> 8) & 0xFF;
		ecc[10] =  (ecc_reg >> 16) & 0xFF;
		ecc[11] =  (ecc_reg >> 24) & 0xFF;

		ecc_reg = fsmc_regs->fsmc_psr0.reg;
		ecc[12] =  (ecc_reg >> 16) & 0xFF;

		/* Clear Code Ready status bit */
		fsmc_regs->fsmc_psr0.bit.code_ready = 0;
	}

	/* Write the ECC code */
	write_byte_reg(NAND_CMD_RANDOM_DATA_INPUT, NAND_CS0_BASE + NAND_CS0_CMD_OFFSET);

	/*
	 * 1 page = 2Kb + 224 bytes
	 * bytes 0 and 1 are reserved for bad blocks, so start at 2Kb + 2 bytes
	 */
	write_byte_reg((ctx.data_bytes_per_sector + 2) & 0xFF,
		       NAND_CS0_BASE + NAND_CS0_COMMON_AREA_OFFSET);
	write_byte_reg((ctx.data_bytes_per_sector & 0xFF00) >> 8,
		       NAND_CS0_BASE + NAND_CS0_COMMON_AREA_OFFSET);

	for (i = 0; i < 4; i++)
		write_reg(*(uint32_t *)(ecc + i), NAND_CS0_BASE);

	write_byte_reg(NAND_CMD_PROGRAM_PAGE_END,
		       NAND_CS0_BASE + NAND_CS0_CMD_OFFSET);

	return 0;
}

/**
 * @brief	Write NAND pages depending on the input size parameters
 * @param	sector_address: starting sector address
 * @param	buff: data buffer pointer
 * @param	size: size in bytes
 * @param	size_max: maximum size in bytes (partition size)
 * @return	0 if no error, not 0 otherwise
 */
int nand_write_pages(uint32_t sector_address, uint8_t *buff,
		uint32_t size, uint32_t size_max)
{
	int err = 0;
	uint32_t cnt = 0;
	uint32_t nb_sectors = 0;
	uint32_t current_block = 0xFFFFFFFF;
	uint32_t sector_max;

	hsem_lock(ctx.hsem);

	nb_sectors = (size + ctx.data_bytes_per_sector - 1)
		/ ctx.data_bytes_per_sector;

	sector_max = sector_address + ((size_max + ctx.data_bytes_per_sector - 1)
				       / ctx.data_bytes_per_sector);

	for (; sector_address < sector_max && nb_sectors;) {
		if (((sector_address % ctx.sectors_per_block) == 0)
		    || (current_block == 0xFFFFFFFF)) {
			current_block = sector_to_block(sector_address);
			/* Bad block skipping */
			if (nand_is_bad_block(current_block)) {
				ctx.bad_blocks++;
				sector_address += ctx.sectors_per_block;
				continue;
			} else {
				nand_erase_block(current_block);
			}
		}

		/* Wait for the ECC FIFO empty */
		while (!fsmc_regs->fsmc_psr0.bit.fifo_empty)
			;

		/* Write buffer and ECC codes */
		nand_cmd_exec_program_page(sector_address, buff, &cnt);

		/* Check NAND status */
		while ((nand_read_status() & NAND_STATUS_READY) != NAND_STATUS_READY)
			;

		/* next sector */
		sector_address++;
		nb_sectors--;
	}

	if (nb_sectors)
		TRACE_ERR("%s: failed to write (%d)\n", __func__, err);

	hsem_unlock(ctx.hsem);
	return err;
}

