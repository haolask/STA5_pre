/**
 * @file sta_sga_p.h
 * @brief    Smart Graphic Accelerator utilities, command batch *
 *
 * Copyright (C) ST-Microelectronics SA 2016
 * @author: APG-MID team
 */

#ifndef _STA_SGA_P_H_
#define _STA_SGA_P_H_

/* Includes */
#include "sta_map.h"
#include "sta_common.h"

/* Constants and new types */
#define STOP						0x00000000
#define GOTO						0x20000000
#define GOSUB						0x40000000
#define NO_OP						0x60000000
#define RETURN						0x61000000
#define WAIT_SYNCHRO					0x80000000
#define WAIT_NEW_SYNCHRO				0x81000000
#define WAIT_PIPE_EMPTY					0x82000000
#define WAIT_N_CYCLES					0x83000000
#define SEND_SYNCHRO					0x84000000
#define SEND_INTERRUPT					0x85000000
#define AHB						0x86000000
#define CACHE_CTRL					0x87000000
#define SET_INSTR_TEST_REG				0x88000000
#define CLR_INSTR_TEST_REG				0x89000000
#define TST_INSTR_TEST_REG				0x8A000000
#define WAIT_INSTR_TEST_REG				0x8B000000

#define IN0_BASE_ADD_MSB				0xA0000000
#define IN0_BASE_ADD					0xA1000000
#define IN0_SET_LINE_JUMP				0xA2000000
#define IN0_SET_SIZE_XY					0xA3000000
#define IN0_SET_DELTA_XY				0xA4000000
#define IN0_SET_PIXEL_TYPE				0xA5000000

#define IN1_BASE_ADD_MSB				0xA8000000
#define IN1_BASE_ADD					0xA9000000
#define IN1_SET_LINE_JUMP				0xAA000000
#define IN1_SET_SIZE_XY					0xAB000000
#define IN1_SET_DELTA_XY				0xAC000000
#define IN1_SET_PIXEL_TYPE				0xAD000000

#define IN2_BASE_ADD_MSB				0xB0000000
#define IN2_BASE_ADD					0xB1000000
#define IN2_SET_LINE_JUMP				0xB2000000
#define IN2_SET_SIZE_XY					0xB3000000
#define IN2_SET_DELTA_XY				0xB4000000
#define IN2_SET_PIXEL_TYPE				0xB5000000

#define OUT_BASE_ADD_MSB				0xB8000000
#define OUT_BASE_ADD					0xB9000000
#define OUT_SET_LINE_JUMP				0xBA000000
#define OUT_SET_SIZE_XY					0xBB000000
#define OUT_SET_BASE_XY					0xBC000000
#define OUT_SET_PIXEL_TYPE				0xBD000000

#define SET_POINT0					0xC0000000
#define SET_POINT1					0xC1000000
#define SET_POINT2					0xC2000000
#define SET_COLOR					0xC3000000
#define SET_BYPASS_ZS					0xC4000000
#define LINE_STIPPLING					0xC5000000
#define DRAW_RECTANGLE					0xC6000000
#define DRAW_TRIANGLE					0xC7000000
#define DRAW_LINE					0xC8000000
#define DRAW_POINT					0xC9000000
#define DRAW_TRIANGLE_SHIFT				0xCA000000
#define DRAW_LINE_SHIFT					0xCB000000

#define SET_ZX_COEF					0xCC000000
#define SET_ZY_COEF					0xCD000000
#define SET_Z_OFFSET					0xCE000000
#define SET_Z_DYN					0xCF000000

#define SET_XX_COEF					0xD0000000
#define SET_XY_COEF					0xD1000000
#define SET_YX_COEF					0xD2000000
#define SET_YY_COEF					0xD3000000
#define SET_WX_COEF					0xD4000000
#define SET_WY_COEF					0xD5000000
#define SET_X_OFFSET					0xD6000000
#define SET_Y_OFFSET					0xD7000000
#define SET_W_OFFSET					0xD8000000
#define SET_XY_MODE					0xD9000000
#define SET_XY_DYN					0xDA000000

#define TRANSP_COLORMSB					0xE0000000
#define TRANSP_IN_COLOR					0xE1000000
#define TRANSP_OUT_COLOR				0xE2000000
#define TRANSP_MODE					0xE3000000

#define FLASH_COLOR_MSB					0xE4000000
#define FLASH_COLOR_ID					0xE5000000
#define FLASH_COLOR_NEW					0xE6000000
#define FLASH_MODE					0xE7000000

#define SET_COEF_AXAY					0xE8000000
#define SET_COEF_A0					0xE9000000
#define SET_COEF_RXRY					0xEA000000
#define SET_COEF_R0					0xEB000000
#define SET_COEF_GXGY					0xEC000000
#define SET_COEF_G0					0xED000000
#define SET_COEF_BXBY					0xEE000000
#define SET_COEF_B0					0xEF000000
#define SET_COEF_DYN					0xF0000000

#define SET_TEX_COLORMSB				0xF1000000
#define SET_TEX_COLORLSB				0xF2000000
#define SET_TEX_ENV_MSB					0xF3000000
#define SET_TEX_ENV_LSB					0xF4000000
#define SET_TEX_SCALE					0xF5000000
#define SET_COEF_FXFY					0xF6000000
#define SET_COEF_F0					0xF7000000
#define SET_COLOR_F0					0xF8000000
#define SET_BLEND_COLORMSB				0xF9000000
#define SET_BLEND_ENV					0xFA000000

#define PIXEL_OP_MODE					0xFB000000

#define SET_ALPHA_TEST					0xFC000000
#define SET_STENCIL_TEST				0xFD000000
#define SET_DEPTH_TEST					0xFE000000

/*  SGE Configuration */

#define SGE_MAIN_FIRMWARE_SIZE		(1024 * 4)
#define SGE_FIRMWARE_OFFSET		0
#define SGE_DEFAULT_FIRMWARE_OFFSET	(SGE_MAIN_FIRMWARE_SIZE \
					 - SGE_FIRMWARE_OFFSET - \
					 2 * 4)

#define SGE_BATCH_DEFAULT_SIZE		0x8000
#define SGE_MAX_AVAIL_BATCHES		0x4
#define SGE_BATCHES_SIZE		(SGE_BATCH_DEFAULT_SIZE * \
					 SGE_MAX_AVAIL_BATCHES)

#define SGE_NULL_SURFACE		0x0

/*  SGA Configuration */
#define SGA_ADDRESS_ALIGN		0x3
#define SGA_MEMORY_ALIGN_MASK		0x7

/*
 * Cache Configuration
 *    [8:6]    = AutoFlushBank active (flushes after each Draw resp banks
 *              [2:Texture,1:Out,0:In0])
 *    [14:12]  = AutoInitCache: resets cache automatically after each Draw,
 *    for resp  [2:Texture,1:Out,0:In0])
 */
#define SGA_CACHE_CONFIGURATION				0x71c0

/*
 * Auto Fetch Active
 *     [8]     = AutoFetchMode active
 *               allows DmaFsm to fetch directly instructions
 *     [5:4]   = BurstType generated when possible: 0: INCR, 1:INCR4, 2:INCR8,
 *               3:INCR16 (not possible with current Fifos of 2*8 words).
 */
#define SGA_AHB_CONFIGURATION				0x120

typedef unsigned int		uint;
typedef unsigned long		t_sga_instr_word;
typedef short			sga_coordinate;
typedef unsigned long		sga_address;
typedef unsigned char		sga_bool;

typedef signed long		sga_long;
typedef unsigned long		sga_ulong;
/*
 *typedef signed __int64	sga_int64;
 *typedef unsigned __int64	sga_uint64;
 */
typedef signed int		sga_int32;
typedef unsigned int		sga_uint32;
typedef signed short		sga_int16;
typedef unsigned short		sga_uint16;
typedef signed char		sga_int8;
typedef unsigned char		sga_uint8;

/*
 * Macros
 */
#define SGA_SET_BIT(reg_name, mask)	((reg_name) |= (mask))
#define SGE_IS_ALIGN(v, mask)		(((v) & (mask)) == 0)
#define SGE_GET_BATCH_SIZE(v)		((((uint32_t)(v)) & 0x0FFFFFFF))
#define SGE_GET_BATCH_COUNT(v)		((((uint32_t)(v)) & 0xF0000000) >> 28)

#define SGE_TO_USER_SPACE(addrKernel, offset)	((uint32_t)(addrKernel) + \
						 (offset))
#define SGE_TO_KERNEL_SPACE(addrUser, offset)	((uint32_t)(addrUser) - \
						 (offset))
#define SGE_START_BATCH_OFFSET			0
#define SGE_END_BATCH_OFFSET			3

#define SGA_READ_FIELD(reg_name, mask, shift)	(((reg_name) & (mask)) >> \
						 (shift))

#define SGE_DO_ALIGN(v, mask)			(((v) + (mask)) & (~(mask)))
#define SGE_IS_ALIGN(v, mask)			(((v) & (mask)) == 0)
#define SGE_SET_MAGIC_WORD			(0xCAFE0000)
#define SGE_WRONG_MAGIC_WORD(data)		((data) != 0xCAFE0000)

/*
 * Structs
 */
typedef unsigned long		t_sga_instr_word;

typedef struct sga_system_context {
	/* Contain the SGA memory mapped registers address */
	t_sga_registers		*p_sga_registers;
	/* Store the all interrupt resource availability status */
	uint32_t		interrupt_id;
	/* Store the all batch and semaphore id resources availability status */
	uint32_t		batch_sem_id;
	/* Contain the main batch firmware memory physical address */
	uint32_t		*p_main_batch_add;
	/* Contain the default batch firmware memory address */
	uint32_t		*p_default_batch_add;
	/* Contain the main FW allocated buffer */
	uint32_t		*p_main_fw_addr;
	/* Contain the SGA batches allocated buffer */
	uint32_t		*p_sga_batches_addr;
} t_sga_system_context;

#endif /* _STA_SGA_P_H_ */

