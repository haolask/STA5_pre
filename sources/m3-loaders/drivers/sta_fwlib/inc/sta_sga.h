/**
 * @file sta_sga.h
 * @brief Smart Graphic Accelerator utilities
 *
 * Copyright (C) ST-Microelectronics SA 2016
 * @author: APG-MID team
 */

#ifndef _SGA_H_
#define _SGA_H_

/* Includes */
#include "sta_sga_p.h"
#include "utils.h"

/* Definitions */

#define SGA_FREQUENCY_MHZ 200

/**
 * @brief SGA Global Configuration register fields description
 */

/* Shift values for Golbal configuration register */
#define  SGA_GCR_INTCMOD_SHIFT	0  /*! Interrupt clear mode select bit */
#define  SGA_GCR_FCLKGEN_SHIFT	1  /*! FCLK clock gating enable bit    */
#define  SGA_GCR_HCLKGEN_SHIFT	2  /*! Hclk Clock gating enable bit    */
#define  SGA_GCR_INTCMOD1_SHIFT	3  /*! Interrupt clear mode select     */

/* Mask values for Golbal configuration register */
#define  SGA_GCR_INTCMOD_MASK	BIT(0)  /*! Interrupt clear mode select bit */
#define  SGA_GCR_FCLKGEN_MASK	BIT(1)  /*! FCLK clock gating enable bit    */
#define  SGA_GCR_HCLKGEN_MASK	BIT(2)  /*! Hclk Clock gating enable bit    */
#define  SGA_GCR_INTCMOD1_MASK	BIT(3)  /*! Interrupt clear mode select     */

/**
 * @brief	SGA Controller command  register fields description
 */

/* Shift values for Controller command register */
#define  SGA_CTCMD_GINIT_SHIFT		0  /*!  Global initialisation bit */
#define  SGA_CTCMD_GHALT_SHIFT		1  /*!  Global Halt bit           */
#define  SGA_CTCMD_GRESUME_SHIF		2  /*!  Global Resume bit         */
#define  SGA_CTCMD_IHALT_SHIFT		3  /*!  Instruction halt bit      */
#define  SGA_CTCMD_IRESUME_SHIFT	4  /*!  Instruction resume bit    */
#define  SGA_CTCMD_IFLUSH_SHIFT		5  /*!  Instruction flush bit     */
#define  SGA_CTCMD_AFSTOP_SHIFT		6  /*!  AutoFetch Stopbit         */
#define  SGA_CTCMD_AFRUN_SHIFT		7  /*!  AutoFetchRun bit          */
#define  SGA_CTCMD_GRST_SHIFT		8  /*!  Global Reset bit          */

/* Mask values for Controller command register */
#define  SGA_CTCMD_GINIT_MASK		BIT(0)  /*  Global initialisation bit */
#define  SGA_CTCMD_GHALT_MASK		BIT(1)  /*  Global Halt bit           */
#define  SGA_CTCMD_GRESUME_MASK		BIT(2)  /*  Global Resume bit         */
#define  SGA_CTCMD_IHALT_MASK		BIT(3)  /*  Instruction halt bit      */
#define  SGA_CTCMD_IRESUME_MASK		BIT(4)  /*  Instruction resume bit    */
#define  SGA_CTCMD_IFLUSH_MASK		BIT(5)  /*  Instruction flush bit     */
#define  SGA_CTCMD_AFSTOP_MASK		BIT(6)  /*  AutoFetch Stopbit         */
#define  SGA_CTCMD_AFRUN_MASK		BIT(7)  /*  AutoFetchRun bit          */
#define  SGA_CTCMD_GRST_MASK		BIT(8)  /*  Global Reset bit          */

/*
 * @brief	SGA Controller status  register fields description
 */

/* Shift values for Controller status register */
#define  SGA_CTSTAT_GEN_SHIFT		0  /*! Global enable status */
#define  SGA_CTSTAT_IPEN_SHIFT		1  /*! Instr processing enable status */
#define  SGA_CTSTAT_AFSTAT_SHIFT	2  /*! Auto fetch mode status */
#define  SGA_CTSTAT_IFEMPTY_SHIFT	3  /*! Instruction fifo empty */
#define  SGA_CTSTAT_PXPEMPTY_SHIFT	4  /*! Pixel pipe empty */
#define  SGA_CTSTAT_TPEMPTY_SHIFT	5  /*! Total Pipe empty */
#define  SGA_CTSTAT_TPCEMPTY_SHIFT	6  /*! Total Pipe and Cache Empty */
#define  SGA_CTSTAT_RESTARTCNT_SHIFT	16 /*! Restart Counter */

/* Mask values for Controller status register */
#define  SGA_CTSTAT_GEN_MASK		BIT(0) /*! Global enable status */
/*! Instruction processing enable status */
#define  SGA_CTSTAT_IPEN_MASK		BIT(1)
#define  SGA_CTSTAT_AFSTAT_MASK		BIT(2) /*! Auto fetch mode status */
#define  SGA_CTSTAT_IFEMPTY_MASK	BIT(3) /*! Instruction fifo empty */
#define  SGA_CTSTAT_PXPEMPTY_MASK	BIT(4) /*! Pixel pipe empty */
#define  SGA_CTSTAT_TPEMPTY_MASK	BIT(5) /*! Total Pipe empty */
#define  SGA_CTSTAT_TPCEMPTY_MASK	BIT(6) /*! Total Pipe and Cache Empty */
#define  SGA_CTSTAT_RESTARTCNT_MASK	0xFFFF0000  /*! Restart Counter */

#define SGA_INT_GEN25_2		(0x007FFFFF << 8) /* Instruction SendInt2~25 */
#define SGA_INT_IRESUM1MIS	BIT(7) /* Instruction Resume 1 Interrupt */
/* Instruction Resume 0 Interrupt Status */
#define SGA_INT_IRESUM0MIS	BIT(6)
#define SGA_INT_AHBEMIS		BIT(5) /* AHB Error Interrupt Status  */
#define SGA_INT_HANGMIS		BIT(4) /* Hanging Interrupt Status  */
/* Instruction FIFO Overrun Interrupt Status */
#define SGA_INT_IFIFOOVMIS	BIT(3)
/* Instruction FIFO Empty Interrupt Status */
#define SGA_INT_IFIFOEMIS	BIT(2)
#define SGA_INT_GEN1_0		(3 << 0) /* Instruction SendInt1~0 */
#define SGA_INT_GEN_ALL		(SGA_INT_GEN25_2      \
				 | SGA_INT_IRESUM1MIS \
				 | SGA_INT_IRESUM0MIS \
				 | SGA_INT_IFIFOEMIS  \
				 | SGA_INT_GEN1_0)

#define GUARDBAND_X		(0)
#define GUARDBAND_Y		(0)

#define SGA_RASTER_PRECISION			2
#define TRIG_PRECISION				12
#define MAX_PLIST_LENGTH			50
#define BATCH_SURFACE_NUM			(0x2)
#define BATCH_SURFACE_SIZE			(0x8000)
#define ACTIVATE_ALTERNATIVE_OPTIMIZATIONS
#define DO_DRAW_LINES
#define DO_DRAW_ENDCAPS

#define GUARDBAND_SET_ADDRESS(surf) \
	((surf)->guardband_addr = ((surf)->addr - \
				  ((GUARDBAND_Y * (surf)->width + GUARDBAND_X) \
				   * (surf)->format.color_depth)))

#define SGA_PACK_VECTOR_GUARDBAND(x, y, prec) \
			(((((x) + (GUARDBAND_X << (prec))) & 0xFFF) << 12) | \
			 (((y) + (GUARDBAND_Y << (prec))) & 0xFFF))
#define SGA_PACK_VECTOR_SCREEN(x, y, prec) \
			(((((x) + (GUARDBAND_X << (prec))) & 0xFFF) << 12) | \
			 (((y) + (GUARDBAND_X << prec)) & 0xFFF))

#define SGA_PACK_VECTOR_ONE(prec)	(((0x1 << 12) | 0x1) << (prec))
#define SGA_PACK_VECTOR(x, y)		((((x) & 0xFFF) << 12) | ((y) & 0xFFF))

/*
 * @brief	SGA pixels types
 */
#define SGA_PIX_FORMAT_MONO_1BPP	0x00
#define SGA_PIX_FORMAT_MONO_2BPP	0x01
#define SGA_PIX_FORMAT_MONO_4BPP	0x02
#define SGA_PIX_FORMAT_MONO_8BPP	0x03
#define SGA_PIX_FORMAT_RGB8		0x04
#define SGA_PIX_FORMAT_RGBA15		0x07   /* 5-5-5-1 */
#define SGA_PIX_FORMAT_RGB16		0x08   /* 5-6-5   */
#define SGA_PIX_FORMAT_ARGB15		0x09   /* 1-5-5-5 */
#define SGA_PIX_FORMAT_ARGB24		0x0A   /* A888, can be packed as 888 */
#define SGA_PIX_FORMAT_RGBA24		0x0B   /* 888A, actually rgb32 */
#define SGA_PIX_FORMAT_YUV422		0x0C
#define SGA_PIX_FORMAT_YUV420		0x40

#define SGA_PIX_FMT_MONO_1BPP		0x00
#define SGA_PIX_FMT_MONO_2BPP		0x01
#define SGA_PIX_FMT_MONO_4BPP		0x02
#define SGA_PIX_FMT_MONO_8BPP		0x03
#define SGA_PIX_FMT_RGB8		0x04
#define SGA_PIX_FMT_RGBA15		0x07   /* 5-5-5-1 */
#define SGA_PIX_FMT_RGB16		0x08   /* 5-6-5   */
#define SGA_PIX_FMT_ARGB15		0x09   /* 1-5-5-5 */
#define SGA_PIX_FMT_ARGB24		0x0A   /* A888, can be packed as 888 */
#define SGA_PIX_FMT_RGBA24		0x0B   /* 888A, actually rgb32 */
#define SGA_PIX_FMT_YUV422		0x0C
#define SGA_PIX_FMT_YUV420		0x40
#define SGA_PIX_FMT_LITTLE_ENDIAN	0x0010
#define SGA_PIX_FMT_BGR			0x0020
#define SGA_PIX_FMT_STENCIL_MODE_OFF	0x0000
#define SGA_PIX_FMT_STENCIL_MODE_1BPP	0x0080
#define SGA_PIX_FMT_PACK_RGB24		0x4000

#define SGA_PIX_TYPE_BYPASS		0x00000400
#define SGA_PIX_TYPE_FREEZE_AHB		0x00000800
#define SGA_PIX_TYPE_ACTIVATE_FLOW	0x00001000
#define SGA_PIX_TYPE_ACTIVATE_BILIN	0x00002000
#define SGA_PIX_TYPE_DEPACK_RGB32	0x00004000
/*  Truncate to range 16 - 235 */
#define SGA_PIX_FMT_TRUNC_16_235	0x00008000
/*  If set, convert rgb to yuv422 format, */
#define SGA_PIX_FMT_RGB_TO_YUV		0x00010000
/*  Do color space conversion */
#define SGA_PIX_FMT_CONVRT		0x00020000
/*  Force all color components to 0 */
#define SGA_PIX_FMT_RGB_0		0x00040000
#define SGA_PIX_FMT_ALPHA_255		0x00000100

#define SGA_TEX_SOURCE_0		0x00000000
#define SGA_TEX_SOURCE_1		0x00800000

#define SGA_TEX_IN1			0x00000000
#define SGA_TEX_IN2			0x00800000

#define SGA_ACTIVATE_SCISSOR_OPR	0x00000002
#define SGA_ACTIVATE_TIE_BRK_RULE	0x00000001

/**
 * @brief	BlendEnv stuff
 */
#define SGA_BLEND_OP_ADD			0x00000000
#define SGA_BLEND_OP_SUB			0x00010000
#define SGA_BLEND_OP_REV_SUB			0x00020000
#define SGA_BLEND_OP_MIN			0x00030000
#define SGA_BLEND_OP_MAX			0x00040000

#define SHFT_RGB_FRAG_SRC			12
#define SHFT_ALPHA_FRAG_SRC			8
#define SHFT_RGB_FRAME_SRC			4
#define SHFT_ALPHA_FRAME_SRC			0

#define SGA_XYMODE_SET_MIRROR_LOG_SIZEX(x)	(((x) & 0xF) << 9)
#define SGA_XYMODE_SET_MIRROR_LOG_SIZEY(y)	(((y) & 0xF) << 3)

/**
 * @brief	Fixed address FIFO register
 */
#define SGA_STOP_CONCAT				0x00010000
#define SGA_MASK_Z_BITS				0x00008000
#define SGA_MASK_S1_BITS			0x00004000
#define SGA_MASK_S0_BITS			0x00002000
#define SGA_MASK_A_BITS				0x00001000
#define SGA_MASK_R_BITS				0x00000800
#define SGA_MASK_G_BITS				0x00000400
#define SGA_MASK_B_BITS				0x00000200
#define SGA_DITHER_RGB				0x00000100
#define SGA_DITHER_ALPHA			0x00000080
#define SGA_ACTIVATE_ROP4			0x00000040
#define SHFT_ROP4_FUNCT				0x00000002
#define SGA_ACTIVATE_TBREAK			0x00000001

#define SHFT_RGB_FNCN				15
#define SHFT_ALPHA_FNCN				12

#define SHFT_RGB_SRC0				10
#define SHFT_RGB_SRC1				8
#define SHFT_RGB_SRC2				6
#define SHFT_ALPHA_SRC0				4
#define SHFT_ALPHA_SRC1				2
#define SHFT_ALPHA_SRC2				0

#define SHFT_OPR0_RGB				7
#define SHFT_OPR1_RGB				5
#define SHFT_OPR2_RGB				3

#define SHFT_OPR0_ALPHA				2
#define SHFT_OPR1_ALPHA				1
#define SHFT_OPR2_ALPHA				0

#define SCALE_MASK				0xFF
#define SHFT_RGB_SCALE				8
#define SHFT_ALPHA_SCALE			0

/**
 * @brief	Wait instruction stuff
 */
#define SGA_WAIT_PIPE_EMP_COMPLETE	0
#define SGA_WAIT_PIPE_EMP_OPERATIVE	1

#define SHFT_RAST_PRECISION		19
#define SGA_RASTER_INTEGER_COORDS \
			(SGA_RASTER_PRCS_LSB_NORMAL << SHFT_RAST_PRECISION)
#define SGA_RASTER_1DECBITS_COORDS \
			(SGA_RASTER_PRCS_LSB_1 << SHFT_RAST_PRECISION)
#define SGA_RASTER_2DECBITS_COORDS \
			(SGA_RASTER_PRCS_LSB_2 << SHFT_RAST_PRECISION)
#define SGA_RASTER_3DECBITS_COORDS \
			(SGA_RASTER_PRCS_LSB_3 << SHFT_RAST_PRECISION)

#define SGA_PIXEL_OPERATOR_ACTIVE	0x00000000
#define SGA_PIXEL_OPERATOR_BYPASS	0x00040000
#define SGA_PIXEL_FREEZE_INDEX		0x00020000

#define SGA_DISABLE_BLENDING(op)	(((op) | SGA_BLEND_OP_ADD)	| \
		(SGA_BLEND_SRC_COEF_1 << SHFT_RGB_FRAG_SRC)		| \
		(SGA_BLEND_SRC_COEF_1 << SHFT_ALPHA_FRAG_SRC)		| \
		(SGA_BLEND_SRC_COEF_0 << SHFT_RGB_FRAME_SRC)		| \
		(SGA_BLEND_SRC_COEF_0 << SHFT_ALPHA_FRAME_SRC))

/* Bit mask definition */
#define MASK_NULL8				0x00
#define MASK_NULL16				0x0000
#define MASK_NULL32				0x00000000
#define MASK_ALL8				0xFF
#define MASK_ALL16				0xFFFF
#define MASK_ALL32				0xFFFFFFFF

/* Enums */
typedef enum sga_error {
	SGA_OK						=  0, /*  No error */
	SGA_NO_PENDING_EVENT_ERROR			= -1,
	SGA_NO_MORE_FILTER_PENDING_EVENT		= -2,
	SGA_NO_MORE_PENDING_EVENT			= -3,
	SGA_REMAINING_FILTER_PENDING_EVENTS		= -4,
	SGA_REMAINING_PENDING_EVENTS			= -5,
	SGA_INTERNAL_EVENT				= -6,
	SGA_INTERNAL_ERROR				= -7,
	SGA_NOT_CONFIGURED				= -8,
	SGA_REQUEST_PENDING				= -9,
	SGA_REQUEST_NOT_APPLICABLE			= -10,
	SGA_INVALID_PARAMETER				= -11,
	SGA_UNSUPPORTED_FEATURE				= -12,
	SGA_UNSUPPORTED_HW				= -13,
	SGA_RESOURCE_NOT_AVAILABLE			= -14,
	SGA_MAIN_FIRMWARE_NOT_BUILD			= -15,
	SGA_GENERIC_FAILURE				= -16
} t_sga_error;

enum coeff_xy_mode {
	SGA_XYMODE_DISABLE				= 0x0,
	SGA_XYMODE_ENABLE_ROTATION			= (0x1 << 0),
	SGA_XYMODE_ENABLE_PERSPECTIVE_CORRECTION	= (0x1 << 13),
	SGA_XYMODE_CLAMPING_X				= (0x1 << 7),
	SGA_XYMODE_REPEAT_X				= (0x2 << 7),
	SGA_XYMODE_MIRROR_X				= (0x3 << 7),
	SGA_XYMODE_CLAMPING_Y				= (0x1 << 1),
	SGA_XYMODE_REPEAT_Y				= (0x2 << 1),
	SGA_XYMODE_MIRROR_Y				= (0x3 << 1)
};

enum coeff_src {
	SGA_BLEND_SRC_COEF_0,
	SGA_BLEND_SRC_COEF_1,
	SGA_BLEND_SRC_COEF_RGB_FRAGMENT,
	SGA_BLEND_SRC_COEF_1_RGB_FRAGMENT,
	SGA_BLEND_SRC_COEF_RGB_FRAME,
	SGA_BLEND_SRC_COEF_1_RGB_FRAME,
	SGA_BLEND_SRC_COEF_A_FRAGMENT,
	SGA_BLEND_SRC_COEF_1_A_FRAGMENT,
	SGA_BLEND_SRC_COEF_A_FRAME,
	SGA_BLEND_SRC_COEF_1_A_FRAME,
	SGA_BLEND_SRC_COEF_RGB_CST_CLR,
	SGA_BLEND_SRC_COEF_1_RGB_CST_CLR,
	SGA_BLEND_SRC_COEF_A_CST_CLR,
	SGA_BLEND_SRC_COEF_1_A_CST_CLR,
	SGA_BLEND_SRC_COEF_MIN
};

/**
 * @brief	Combine functions for RGB components
 */
enum text_funcs {
	SGA_TEX_ENV_REPLACE,	/* Replace arg0 */
	SGA_TEX_ENV_MODULATE,	/* Module arg0 * arg1 */
	SGA_TEX_ENV_ADD,	/* Add arg0 + arg1 */
	SGA_TEX_ENV_ADD_SIGNED,	/* Add signed arg0 + arg1 */
	/* Interpolate: arg0 * arg2 + arg1 * (1 - arg2) */
	SGA_TEX_ENV_INTERPOLATE,
	/* Subtract arg0 - arg1 */
	SGA_TEX_ENV_SUBTRACT,
	/* Dot3rgb: 4 * [(arg0 - 0.5) * (arg1 - 0.5) * Red + ..green + ..blue */
	SGA_TEX_ENV_DOT3RGB,
	/* Dot4rgb: as above, but also on aplpha component */
	SGA_TEX_ENV_DOT4RGB

};

enum component_src {
	SGA_TEX_ENV_SOURCE_TEXTURE,
	SGA_TEX_ENV_SOURCE_CST_COLOR,
	SGA_TEX_ENV_SOURCE_GOURAUD_COL,	/* Gouraud color   */
	SGA_TEX_ENV_SOURCE_PREVIOUS_COL	/* Previous color  */
};

enum rgb_operand {
	SGA_TEX_ENV_RGB_OPR_COLOR,
	SGA_TEX_ENV_RGB_OPR_1_COLOR,
	SGA_TEX_ENV_RGB_OPR_ALPHA,
	SGA_TEX_ENV_RGB_OPR_1_ALPHA
};

enum alpha_operand {
	SGA_TEX_ENV_A_OPR_ALPHA,
	SGA_TEX_ENV_A_OPR_1_ALPHA
};

/**
 * @brief	Pixel Operations
 */
enum rasterizer_precision {
	SGA_RASTER_PRCS_LSB_NORMAL,
	SGA_RASTER_PRCS_LSB_1,
	SGA_RASTER_PRCS_LSB_2,
	SGA_RASTER_PRCS_LSB_3
};

/* Structs */
typedef struct sga_color_format {
	sga_uint32		buffer_fmt;
	sga_uint8		color_depth;
} t_sga_color_format;

typedef struct sga_rectangle {
	int			m_x;
	int			m_y;
	int			m_w;
	int			m_h;
} t_sga_rectangle;

typedef struct sga_surface {
	sga_address		addr;
	sga_address		guardband_addr;
	sga_uint32		m_size_xy;
	sga_uint32		height;
	sga_uint32		width;
	t_sga_color_format	format;
	sga_uint32		swap_rb;
	sga_uint8		m_field;
	t_sga_rectangle		m_crop;
} t_sga_surface;

typedef struct sga_command_surface {
	t_sga_instr_word	*curr_addr;
	t_sga_instr_word	*batch_end;
} t_sga_command_surface;

typedef struct sga_memory_context {
	uint32_t		magic;
	uint32_t		memory_size;
	uint32_t		user_mem;
	uint32_t		kernel_mem;
	uint32_t		phys_mem;
	uint32_t		user_kernel_offset;
	uint32_t		heap;
} t_sga_memory_context;

typedef struct sga_batch_context {
	uint32_t		magic;
	uint32_t		curr_batch;
	uint32_t		num_batchs;
	/*  +1 for the sentinel batch */
	t_sga_instr_word	*batch_start_ptr[SGE_MAX_AVAIL_BATCHES + 1];
	t_sga_instr_word	*last_inst_addr[SGE_MAX_AVAIL_BATCHES];
	uint32_t		dst_surface_id[SGE_MAX_AVAIL_BATCHES];
	uint32_t		*addr_instr_ptr[SGE_MAX_AVAIL_BATCHES];
	uint32_t		priv[SGE_MAX_AVAIL_BATCHES];
	/*callbacks to be called on SGA interrupt*/
	void			(*int_callback[SGE_MAX_AVAIL_BATCHES])(void);
	t_sga_command_surface	sga_lib_surface;
	t_sga_command_surface	curr_batch_surface;
	uint8_t			int_id[SGE_MAX_AVAIL_BATCHES];
	uint8_t			batch_id[SGE_MAX_AVAIL_BATCHES];
} t_sga_batch_context;

/**
 * @brief	SGA display types
 */
static const t_sga_color_format SGA_DIS_FMT_MONO_8BPP	= {
				SGA_PIX_FORMAT_MONO_8BPP,	1 }; /* BpP */
static const t_sga_color_format SGA_DIS_FMT_RGB565	= {
				SGA_PIX_FORMAT_RGB16,		2 }; /* BpP */
static const t_sga_color_format SGA_DIS_FMT_RGBA15	= {
				SGA_PIX_FORMAT_RGBA15,		2 }; /* BpP */
static const t_sga_color_format SGA_DIS_FMT_ARGB15	= {
				SGA_PIX_FORMAT_ARGB15,		2 }; /* BpP */
static const t_sga_color_format SGA_DIS_FMT_RGB888	= {
				SGA_PIX_FORMAT_ARGB24 | 0x4000,	3 }; /* BpP */
static const t_sga_color_format SGA_DIS_FMT_ARGB8888	= {
				SGA_PIX_FORMAT_ARGB24,		4 }; /* BpP */
static const t_sga_color_format SGA_DIS_FMT_RGBA8888	= {
				SGA_PIX_FORMAT_RGBA24,		4 }; /* BpP */
static const t_sga_color_format SGA_DIS_FMT_YUV422	= {
				SGA_PIX_FORMAT_YUV422,		2 }; /* BpP */
static const t_sga_color_format SGA_DIS_FMT_YUV420	= {
				SGA_PIX_FORMAT_YUV420,		2 }; /* BpP */

#define SGA_BUFFER_ALIGNMENT 0x10

t_sga_error sga_build_main_batch_firmware
(
	uint32_t		*p_phy_main_add,
	uint32_t		*p_phy_default_batch_add,
	uint32_t		*p_no_cmd
);

/**
 * @brief	Functions & Routines declaration
 */
t_sga_error sge_init(void);

t_sga_error sge_deinit(void);

t_sga_error sge_do_init_batch(
		const uint32_t	num_batchs,
		const uint32_t	batch_size,
		const uint32_t	header_size,
		void		**ctx_mem,
		void		**ctx_batch);

t_sga_error sge_do_start_batch(
		void		*ctx_batch,
		unsigned long	batch_id);

t_sga_error sge_do_stop_batch(
		void		*ctx_batch,
		unsigned long	batch_id);

t_sga_error sga_pick_batch(
		uint8_t		*p_batch_id);

t_sga_error sga_release_batch(
		uint8_t		batch_id);

t_sga_error sga_register_int_cb(
		void		*ctx_batch,
		uint8_t		batch_id,
		void		(*func)(void));

t_sga_error sga_get_batch_addr(
		void		*ctx_batch,
		uint8_t		batch_id,
		t_sga_instr_word **addr);

t_sga_error sga_commit_batch(
		void		*ctx_batch,
		uint8_t		batch_id,
		t_sga_instr_word *addr);

t_sga_error sga_set_surface(
		void		*ctx_batch,
		uint8_t		batch_id,
		uint32_t	surface_addr);

t_sga_error sga_get_surface(
		void		*ctx_batch,
		uint8_t		batch_id,
		uint32_t	*surface_addr);

t_sga_error sga_set_fb_instr_ptr_addr(
		void		*ctx_batch,
		uint8_t		batch_id,
		uint32_t	*addr_instr_ptr);

t_sga_error sga_get_fb_instr_ptr_addr(
		void		*ctx_batch,
		uint8_t		batch_id,
		uint32_t	**addr_instr_ptr);

t_sga_error sga_set_priv_data(
		void		*ctx_batch,
		uint8_t		batch_id,
		uint32_t	priv);

t_sga_error sga_get_priv_data(
		void		*ctx_batch,
		uint8_t		batch_id,
		uint32_t	*priv);

t_sga_error sga_get_curr_batch(
		void		*ctx_batch,
		uint8_t		*batch_id);

t_sga_error sga_get_batch_id(
		uint8_t		*p_batch_id);

t_sga_error sga_run_main_batch_firmware(void);

t_sga_error sge_set_tst_register_bit(uint8_t bit_nb);

t_sga_error sge_clear_tst_register_bit(uint8_t bit_nb);

void sga_irq_handler(void);

#endif /*_SGA_H_ */
