#define DRV_VERSION		"1.0"
#define DRV_NAME		"sta-vip"

/*
 * SGA instructions
 */

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

/*
 * @brief	SGA pixels types
 */
#define SGA_PIX_FORMAT_MONO_1BPP		0x00
#define SGA_PIX_FORMAT_MONO_2BPP		0x01
#define SGA_PIX_FORMAT_MONO_4BPP		0x02
#define SGA_PIX_FORMAT_MONO_8BPP		0x03
#define SGA_PIX_FORMAT_RGB8			0x04
#define SGA_PIX_FORMAT_RGBA15			0x07   /*  5-5-5-1 */
#define SGA_PIX_FORMAT_RGB16			0x08   /*  5-6-5   */
#define SGA_PIX_FORMAT_ARGB15			0x09   /*  1-5-5-5 */
#define SGA_PIX_FORMAT_ARGB24			0x0A   /*  A888, can be packed
							*  as 888, see below
							*/
#define SGA_PIX_FORMAT_RGBA24			0x0B   /*  888A, actually
							*  rgb32
							*/
#define SGA_PIX_FORMAT_YUV422			0x0C
#define SGA_PIX_FORMAT_YUV420			0x40

#define SGA_PIX_FMT_MONO_1BPP			0x00
#define SGA_PIX_FMT_MONO_2BPP			0x01
#define SGA_PIX_FMT_MONO_4BPP			0x02
#define SGA_PIX_FMT_MONO_8BPP			0x03
#define SGA_PIX_FMT_RGB8			0x04
#define SGA_PIX_FMT_RGBA15			0x07   /* 5-5-5-1 */
#define SGA_PIX_FMT_RGB16			0x08   /* 5-6-5   */
#define SGA_PIX_FMT_ARGB15			0x09   /* 1-5-5-5 */
#define SGA_PIX_FMT_ARGB24			0x0A   /* A888, Can be packed
							* as 888, see below
							*/
#define SGA_PIX_FMT_RGBA24			0x0B   /* 888A, actually rgb32
							*/
#define SGA_PIX_FMT_YUV422			0x0C
#define SGA_PIX_FMT_YUV420			0x40
#define SGA_PIX_FMT_LITTLE_ENDIAN		0x0010
#define SGA_PIX_FMT_BGR				0x0020
#define SGA_PIX_FMT_STENCIL_MODE_OFF		0x0000
#define SGA_PIX_FMT_STENCIL_MODE_1BPP		0x0080
#define SGA_PIX_FMT_PACK_RGB24			0x4000

#define SGA_PIX_TYPE_BYPASS			0x00000400
#define SGA_PIX_TYPE_FREEZE_AHB			0x00000800
#define SGA_PIX_TYPE_ACTIVATE_FLOW		0x00001000
#define SGA_PIX_TYPE_ACTIVATE_BILIN		0x00002000
#define SGA_PIX_TYPE_DEPACK_RGB32		0x00004000
#define SGA_PIX_FMT_TRUNC_16_235		0x00008000 /*  Truncate to range
							    *  16 - 235
							    */
#define SGA_PIX_FMT_RGB_TO_YUV			0x00010000 /*  If set, convert
							    *  rgb to yuv422
							    *  format,
							    */
#define SGA_PIX_FMT_CONVRT			0x00020000 /* Do color space
							    * conversion
							    */
#define SGA_PIX_FMT_RGB_0			0x00040000 /*  Force all color
							    *  components to 0
							    */
#define SGA_PIX_FMT_ALPHA_255			0x00000100

#define SGA_TEX_SOURCE_0			0x00000000
#define SGA_TEX_SOURCE_1			0x00800000

#define SGA_TEX_IN1				0x00000000
#define SGA_TEX_IN2				0x00800000

#define SGA_ACTIVATE_SCISSOR_OPR		0x00000002
#define SGA_ACTIVATE_TIE_BRK_RULE		0x00000001

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
#define SGA_WAIT_PIPE_EMP_COMPLETE		0
#define SGA_WAIT_PIPE_EMP_OPERATIVE		1

#define SHFT_RAST_PRECISION			19
#define SGA_RASTER_INTEGER_COORDS		(SGA_RASTER_PRCS_LSB_NORMAL << \
						 SHFT_RAST_PRECISION)
#define SGA_RASTER_1DECBITS_COORDS		(SGA_RASTER_PRCS_LSB_1 << \
						 SHFT_RAST_PRECISION)
#define SGA_RASTER_2DECBITS_COORDS		(SGA_RASTER_PRCS_LSB_2 << \
						 SHFT_RAST_PRECISION)
#define SGA_RASTER_3DECBITS_COORDS		(SGA_RASTER_PRCS_LSB_3 << \
						 SHFT_RAST_PRECISION)

#define SGA_PIXEL_OPERATOR_ACTIVE		0x00000000
#define SGA_PIXEL_OPERATOR_BYPASS		0x00040000
#define SGA_PIXEL_FREEZE_INDEX			0x00020000

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

#define MASK_BIT0				BIT(0)
#define MASK_BIT1				BIT(1)
#define MASK_BIT2				BIT(2)
#define MASK_BIT3				BIT(3)
#define MASK_BIT4				BIT(4)
#define MASK_BIT5				BIT(5)
#define MASK_BIT6				BIT(6)
#define MASK_BIT7				BIT(7)
#define MASK_BIT8				BIT(8)
#define MASK_BIT9				BIT(9)
#define MASK_BIT10				BIT(10)
#define MASK_BIT11				BIT(11)
#define MASK_BIT12				BIT(12)
#define MASK_BIT13				BIT(13)
#define MASK_BIT14				BIT(14)
#define MASK_BIT15				BIT(15)
#define MASK_BIT16				BIT(16)
#define MASK_BIT17				BIT(17)
#define MASK_BIT18				BIT(18)
#define MASK_BIT19				BIT(19)
#define MASK_BIT20				BIT(20)
#define MASK_BIT21				BIT(21)
#define MASK_BIT22				BIT(22)
#define MASK_BIT23				BIT(23)
#define MASK_BIT24				BIT(24)
#define MASK_BIT25				BIT(25)
#define MASK_BIT26				BIT(26)
#define MASK_BIT27				BIT(27)
#define MASK_BIT28				BIT(28)
#define MASK_BIT29				BIT(29)
#define MASK_BIT30				BIT(30)
#define MASK_BIT31				BIT(31)

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
	SGA_TEX_ENV_REPLACE,		/* Replace arg0 */
	SGA_TEX_ENV_MODULATE,		/* Module arg0 x arg1 */
	SGA_TEX_ENV_ADD,		/* Add arg0 + arg1 */
	SGA_TEX_ENV_ADD_SIGNED,		/* Add signed arg0 + arg1 */
	SGA_TEX_ENV_INTERPOLATE,	/* Interpolate: arg0 x arg2 +
					 * arg1 x (1 - arg2)
					 */
	SGA_TEX_ENV_SUBTRACT,		/* Subtract arg0 - arg1 */
	SGA_TEX_ENV_DOT3RGB,		/* Dot3rgb: 4 x [(arg0 - 0.5) x
					 * (arg1 - 0.5) x Red + ...green +
					 * ...blue
					 */
	SGA_TEX_ENV_DOT4RGB		/* Dot4rgb: as above, but also on
					 * alpha component
					 */
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

/*
 * SGA Definitions
 */

#define SGA_FREQUENCY_MHZ 200
#define SGA_ODD_TST_BIT  31
#define SGA_EVEN_TST_BIT 30
#define SGA_START_TST_BIT 29
#define SGA_ERROR_TST_BIT 28

/* line duration is always 64 us (on PAL standard as well as on NTSC) */
#define LINE_DURATION_US 64

/* SGA Instruction Register */
#define SGA_INSTR 0x00

/**
 * SGA Global Configuration register fields description
 */
#define SGA_GCR 0x04

/* Shift values for Global configuration register */
#define  SGA_GCR_INTCMOD	BIT(0)   /* Interrupt clear mode select bit */
#define  SGA_GCR_FCLKGEN	BIT(1)   /* FCLK clock gating enable bit    */
#define  SGA_GCR_HCLKGEN	BIT(2)   /* Hclk Clock gating enable bit    */
#define  SGA_GCR_INTCMOD1	BIT(3)   /* Interrupt clear mode select     */

/**
 * SGA Controller command register fields description
 */
#define SGA_CTCMD 0x08

/* Shift values for Controller command register */
#define  SGA_CTCMD_GINIT	BIT(0)   /*  Global initialisation bit */
#define  SGA_CTCMD_GHALT	BIT(1)   /*  Global Halt bit           */
#define  SGA_CTCMD_GRESUME	BIT(2)   /*  Global Resume bit         */
#define  SGA_CTCMD_IHALT	BIT(3)   /*  Instruction halt bit      */
#define  SGA_CTCMD_IRESUME	BIT(4)   /*  Instruction resume bit    */
#define  SGA_CTCMD_IFLUSH	BIT(5)   /*  Instruction flush bit     */
#define  SGA_CTCMD_AFSTOP	BIT(6)   /*  AutoFetch Stopbit         */
#define  SGA_CTCMD_AFRUN	BIT(7)   /*  AutoFetchRun bit          */
#define  SGA_CTCMD_GRST		BIT(8)   /*  Global Reset bit          */

/*
 * SGA Controller status  register fields description
 */
#define SGA_CTSTAT 0x0C

/* Shift values for Controller status register */
#define  SGA_CTSTAT_GEN		BIT(0)   /* Global enable status */
#define  SGA_CTSTAT_IPEN	BIT(1)   /* Instruction processing enable
					  *  status
					  */
#define  SGA_CTSTAT_AFSTAT	BIT(2)   /* Auto fetch mode status */
#define  SGA_CTSTAT_IFEMPTY	BIT(3)   /* Instruction fifo empty */
#define  SGA_CTSTAT_PXPEMPTY	BIT(4)   /* Pixel pipe empty */
#define  SGA_CTSTAT_TPEMPTY	BIT(5)   /* Total Pipe empty */
#define  SGA_CTSTAT_TPCEMPTY	BIT(6)   /* Total Pipe and Cache Empty */
#define  SGA_CTSTAT_RESTARTCNT	BIT(16)  /* Restart Counter */

/*
 * SGA RAW Interrupt Status Register
 */
#define SGA_RIS 0x10

/*
 * SGA Masked Interrupt Status Register
 */
#define SGA_MIS 0x14

/*
 * SGA Interrupt Mask Set/Clear Register
 */
#define SGA_IMSC 0x18

/*
 * SGA Interrupt Clear Register
 */
#define SGA_ICR 0x1C

#define SGA_INT_GEN25_2		(0x00FFFFFF << 8) /* Instruction SendInt2~25 */
#define SGA_INT_IRESUM1MIS	BIT(7) /* Instruction Resume 1 Interrupt */
#define SGA_INT_IRESUM0MIS	BIT(6) /* Instruction Resume 0 Interrupt
					*   Status
					*/
#define SGA_INT_AHBEMIS		BIT(5) /* AHB Error Interrupt Status  */
#define SGA_INT_HANGMIS		BIT(4) /* Hanging Interrupt Status  */
#define SGA_INT_IFIFOOVMIS	BIT(3) /* Instruction FIFO Overrun Interrupt
					*   Status
					*/
#define SGA_INT_IFIFOEMIS	BIT(2) /* Instruction FIFO Empty Interrupt
					*   Status
					*/
#define SGA_INT_GEN1		BIT(1) /* Instruction SendInt1 */
#define SGA_INT_GEN0		BIT(0) /* Instruction SendInt0 */
#define SGA_INT_GEN1_0		(3 << 0) /* Instruction SendInt1~0 */
#define SGA_INT_GEN_ALL		(SGA_INT_GEN25_2      \
				 | SGA_INT_IRESUM1MIS \
				 | SGA_INT_IRESUM0MIS \
				 | SGA_INT_IFIFOEMIS  \
				 | SGA_INT_GEN1_0)

/*
 * SGA Set Instruction Test Register
 */
#define SGA_SITR 0x30

/*
 * SGA Clear Instruction Test Register
 */
#define SGA_CITR 0x34

/*
 * SGA Get Instruction Test Register
 */
#define SGA_GITR 0x38

/*  SGA Configuration */
#define SGA_MAIN_FIRMWARE_SIZE		0x400
#define SGA_WARMUP_BATCH_SIZE		0x1000
#define SGA_BATCH_DEFAULT_SIZE		0x4000
#define SGA_MAX_AVAIL_BATCHES		0x8

#define SGA_ADDRESS_ALIGN		0x3
#define SGA_MEMORY_ALIGN_MASK		0x7
#define SGA_MEMORY_ALIGN		8
#define SGA_LINES_TO_SKIP		8

/*
 * Cache Configuration
 *    [8:6]    = AutoFlushBank active (flushes after each Draw resp banks
 *               [2:Texture,1:Out,0:In0])
 *    [14:12]  = AutoInitCache: resets cache automatically after each Draw,
 *               for resp
 *               [2:Texture,1:Out,0:In0])
 */
#define SGA_CACHE_CONFIGURATION				0x71c0

/*
 * Auto Fetch Active
 *     [8]     = AutoFetchMode active (allows DmaFsm to fetch directly
 *               instructions).
 *     [5:4]   = BurstType generated when possible: 0: INCR, 1:INCR4, 2:INCR8,
 *               3:INCR16 (not possible with current Fifos of 2*8 words).
 */
#define SGA_AHB_CONFIGURATION				0x120

#define SGA_PACK_VECTOR(x, y) ((((x) & 0xFFF) << 12) | ((y) & 0xFFF))

#define SGA_WARMUP_INT_NB 0
#define SGA_RUNNING_INT_NB 1
#define SGA_WARMUP_INT  BIT(SGA_WARMUP_INT_NB)
#define SGA_RUNNING_INT BIT(SGA_RUNNING_INT_NB)
#define SGA_NB_WARMUP_FRAMES 4
#define SGA_HEARTBEAT 30

/**
 * struct sta_sga - All internal data for one instance of device
 * @base: hardware base address
 * @irq: SGA Interrupt number
 * @clk: device clock FIXME
 *
 * @main_fw : SGA Primary Firmware
 * @dma_main_fw : SGA Primary DMA Address
 *
 * @batches : Batch array
 * @dma_batches : DMA Batch Address Array
 * @avail_batches: list available batches
 * @num_batches : max batch number
 *
 * @warmup: Warmup batch
 * @dma_warmup: DMA Memory type Warmup batch
 * @warmup_done: Flag indication Warmup status
 *
 * @rlock: for SGA registers exclusive access
 *
 * @running: SGA currently processing frame
 * @heartbeat: max number of no SGA frames before raising an error to upper
 *             layer.
 *
 */
struct sta_sga {
	void __iomem *base;
	int irq;
	struct clk *clk;

	unsigned long *main_fw;
	dma_addr_t dma_main_fw;

	unsigned long *batches[SGA_MAX_AVAIL_BATCHES];
	dma_addr_t dma_batches[SGA_MAX_AVAIL_BATCHES];
	unsigned int avail_batches;
	unsigned int num_batches;

	unsigned long *warmup;
	dma_addr_t dma_warmup;
	bool warmup_done;

	spinlock_t rlock; /* For SGA registers exclusive access */

	bool running;
	int heartbeat;
};

/*
 * VIP Definitions
 */

#define MAX_FRAMES		SGA_MAX_AVAIL_BATCHES
#define NUM_MIN_BUFFERS		3

#define VIP_DMA_BURST_SIZE	4

enum vip_events {
	VIP_FRAME_START_EVEN,
	VIP_FRAME_END_EVEN,
	VIP_FRAME_START_ODD,
	VIP_FRAME_END_ODD,
};

enum vip_if_transformation {
	DISABLED,
	YUVCBYCRY,
	YUVCRYCBY,
	RGB888A,
	BGR888A,
	ABGR888,
	ARGB888,
	BGR565,
	RGB565,
	RGB888,
	JPEG,
};

enum vip_capture_modes {
	NO_CAPTURE,
	PHOTO_MODE_1,
	PHOTO_MODE_2,
	PHOTO_MODE_3,
	PHOTO_MODE_4,
	PHOTO_MODE_8,
	PHOTO_MODE_16,
	PHOTO_MODE_32,
	VIDEO_MODE_ALL_FRAMES,
	VIDEO_MODE_1_FRAME_ON_2,
	VIDEO_MODE_1_FRAME_ON_4,
	VIDEO_MODE_1_FRAME_ON_8,
	VIDEO_MODE_1_FRAME_ON_16,
	VIDEO_MODE_1_FRAME_ON_32,
	VIDEO_MODE_1_FRAME_ON_64,
	VIDEO_MODE_1_FRAME_ON_128,
};

/*
 * VIP register map (offsets)
 */
#define VIP_CTRL	0x00	/* Control Register                   */
#define VIP_EFECR	0x04	/* Even Field Embedded Codes Register */
#define VIP_CSTARTPR	0x08	/* Crop Start Point Register          */
#define VIP_CSTOPPR	0x0C	/* Crop Stop Point Register           */
#define VIP_MASK	0x10	/* Interrupt & DMA Mask Register      */
#define VIP_STATUS	0x14	/* Interrupt Status Register          */
#define VIP_DMAPR	0x18	/* DMA Pointer Register               */
#define VIP_OFECR	0x1C	/* Odd Field Embedded Codes Register  */

/*
 * VIP video buffer offset
 */
#define VIP_MEM_FIFO_BUFFER	0x3000

/* ITU656: Even Field Embedded Codes Register:
 * 31 - 24: Start of Active Video, Blanking Area (F=0, V=1, H=0).
 * 23 - 16: End of Active Video, Blanking Area (F=0, V=1, H=1).
 * 15 - 8: Start of Active Video, Active Area (F=0, V=0, H=0).
 * 7 - 0: End of Active Video, Active Area (F=0, V=0, H=1)
 */
#define ITU656_EMBEDDED_EVEN_CODE	0xABB6809D
/* ITU656: Odd Field Embedded Codes Register:
 * 31 - 24: Start of Active Video, Blanking Area (F=1, V=1, H=0).
 * 23 - 16: End of Active Video, Blanking Area (F=1, V=1, H=1).
 * 15 - 8: Start of Active Video, Active Area (F=1, V=0, H=0).
 * 7 - 0: End of Active Video, Active Area (F=1, V=0, H=1)
 */
#define ITU656_EMBEDDED_ODD_CODE	0xECF1C7DA

/*
 * VIP_CTRL Control register bits
 */
#define CTRL_RGB_ALPHA_VALUE(a) (((a) & 0xff) << 24)
#define CTRL_INTL_EN		BIT(23)
#define CTRL_SGA_TFR_EN		BIT(22)
#define CTRL_DMA_BURST_SIZE(a)	(((a) & 0x7) << 19)
#define CTRL_WAIT_STATE_EN	BIT(18)
#define CTRL_CROP_SELECT	BIT(17)
#define CTRL_EAV_SEL		BIT(16)
#define CTRL_IF_SYNC_TYPE	BIT(12)	/* ITU656 */
#define CTRL_IF_TRANS(a)	(((a) & 0xf) << 8)
#define CTRL_VS_POL		BIT(7)
#define CTRL_HS_POL		BIT(6)
#define CTRL_PCK_POL		BIT(5)
#define CTRL_EMB_CODES_EN	BIT(4)
#define CTRL_CAPTURE_MODES(a)	(((a) & 0xf) << 0)

#define CTRL_IF_TRANS_MASK    (0xf << 8)
#define CSTOP(w) (0xffff0000 | ((w) & 0xffff))

/*
 * Interrupt and Dma Mask Register
 */

#define IRQ_MASK_BREQ		BIT(15)
#define IRQ_MASK_SREQ		BIT(14)
#define IRQ_DUAL		BIT(12)
#define IRQ_DMAEN		BIT(11)
#define IRQ_FRAME_START		BIT(6)
#define IRQ_FRAME_END		BIT(5)
#define IRQ_LINE_END		BIT(4)

#define IRQ_STATUS_FRAME_START_RAW	BIT(18)
#define IRQ_STATUS_FRAME_END_RAW	BIT(17)
#define IRQ_STATUS_FRAME_TYPE		(3 << 13)
#define IRQ_STATUS_FRAME_ODD		BIT(13)
#define IRQ_STATUS_FRAME_TFR		BIT(12)
#define IRQ_STATUS_FRAME_EVEN		(2 << 13)
#define IRQ_STATUS_FRAME_START		BIT(6)
#define IRQ_STATUS_FRAME_END		BIT(5)
#define IRQ_STATUS_LINE_END		BIT(4)

#define IRQ_ENABLE (IRQ_FRAME_START | IRQ_FRAME_END  | IRQ_LINE_END)
#define IRQ_RESET 0x0

struct sta_vip_fmt {
	char *name;
	u32 fourcc;
	u32 sga_format;
	u8 bpp;
};

static struct sta_vip_fmt sta_cap_formats[] = {
	{
		.name		= "4:2:2, packed, UYVY",
		.fourcc		= V4L2_PIX_FMT_UYVY,
		.bpp		= 2,
		.sga_format	= SGA_PIX_FORMAT_YUV422
	},
	{
		.name		= "RGB565",
		.fourcc		= V4L2_PIX_FMT_RGB565,
		.bpp		= 2,
		.sga_format	= SGA_PIX_FORMAT_RGB16
	},
/*  TBD 32 bits support
 *	{
		.name		= "RGB-8-8-8-8",
		.fourcc		= V4L2_PIX_FMT_BGR32,
		.sga_format	= SGA_PIX_FORMAT_ARGB24,
		.bpp		= 4
	}
 */
};

#define VIP_NUM_FORMATS ARRAY_SIZE(sta_cap_formats)

/**
 * struct vip_control -	structure used to store information about VIP controls
 *			it is used to initialize the control framework.
 */
struct vip_control {
	__u32			id;
	enum v4l2_ctrl_type	type;
	__u8			name[32];  /* Whatever */
	__s32			minimum;   /* Note signedness */
	__s32			maximum;
	__s32			step;
	__u32			menu_skip_mask;
	__s32			default_value;
	__u32			flags;
	__u32			reserved[2];
	__u8			is_volatile;
};

static struct vip_control controls[] = {
	{
		.id = V4L2_CID_MIN_BUFFERS_FOR_CAPTURE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "Minimum number of cap bufs",
		.minimum = 1,
		.maximum = 32,
		.step = 1,
		.default_value = 1,
		.is_volatile = 1,
	},
};

#define NUM_CTRLS ARRAY_SIZE(controls)
#define VIP_MAX_CTRLS		5

#define STR_LENGTH 1400

/*
 * Buffer for one video frame
 */
struct vip_buffer {
	/* common v4l buffer stuff -- must be first */
	struct vb2_v4l2_buffer	vbuf;
	struct list_head	queue;

	unsigned int index;
	unsigned int size;

};

/* Video lines frame counter */
static atomic_t vip_lines_nb = ATOMIC_INIT(0);

static inline struct vip_buffer *to_vip_buffer(struct vb2_buffer *vb2)
{
	return container_of(to_vb2_v4l2_buffer(vb2), struct vip_buffer, vbuf);
}

/*
 * Performance measurements
 */
struct vip_perf {
	u64 start_streaming_ns;
	u64 end_streaming_ns;
	u64 max_duration;
	u64 min_duration;
};

/**
 * struct sta_vip - All internal data for one instance of device
 * @v4l2_dev: device registered in v4l layer
 * @video_dev: properties of our device
 * @pdev: Platform device
 * @notifier: V4L2 asynchronous subdevs notifier
 * @asd: sub-device descriptor for async framework
 * @decoder: contains information about video decoder
 * @ctrl_hdl: handler for control framework
 * @field: field format
 * @event: VIP frame event
 * @base: hardware base address
 * @line_irq: Video Interrupt number
 * @vsync_irq: Video Frame Interrupt number
 * @clk: device clock FIXME
 * @reset_pin: GPIO to reset video slave decoder device
 * @pwr_pin: GPIO to power on/off video slave decoder device
 * @format_out: V4l2 Output pixel format, fixed UYVY
 * @format_cap: V4l2 Capure pixel format, fixed UYVY
 * @std: video standard (e.g. PAL/NTSC)
 * @input: input line for video signal (0 => Ain1, 1 => Ain2, 2 => Ain3 ...)
 *		for sta1095 EVB this is 2
 *		see ADV7182 datasheet Input Control register (0x0)
 * @work: delayed work to start DMA at end of frame interruption
 * @rlock: for VIP registers exclusive access
 * @vq: queue maintained by videobuf2 layer
 * @capture: list of buffer in use
 * @sequence: sequence number of acquired buffer
 * @frame_err: number of frame error
 * @frame_drop: number of frame drop
 * @active: current active buffer
 * @lock: used in videobuf2 callback
 * @alloc_ctx: allocator-specific contexts for each plane
 *
 * @sga: SGA internal structure
 *
 * @instatus: VIP interrupt status backup
 * @pending: VIP pending SGA request
 * @timeout: Number of dropped frames before giving up
 *
 * @debugfs_dir:	debugfs directory
 * @debugfs_device:	debugfs device infos
 * @debugfs_hw:		debugfs hardware infos
 * @debugfs_last:	debugfs last instance infos
 * @str: centralized allocated string for debug
 *
 * All non-local data is accessed via this structure.
 */
struct sta_vip {
	struct v4l2_device v4l2_dev;
	struct video_device video_dev;
	struct platform_device *pdev;
	struct v4l2_async_notifier notifier;
	struct v4l2_async_subdev asd;
	struct v4l2_subdev *decoder;
	struct v4l2_ctrl_handler ctrl_hdl;
	struct v4l2_ctrl *ctrls[VIP_MAX_CTRLS];
	bool   v4l2_ctrls_is_configured;
	enum v4l2_field field;
	enum vip_events event;

	void __iomem *base;
	resource_size_t phys;
	int line_irq;
	int vsync_irq;
	struct clk *clk;
	int pwr_pin;
	int reset_pin;

	u8 bus_width;

	u32 ctrl; /* Current value of VIP CTRL register */

	struct v4l2_pix_format format_out;
	struct v4l2_pix_format format_cap;

	u32 max_height;
	v4l2_std_id std;
	unsigned int input;

	struct delayed_work work;
	spinlock_t rlock; /* for VIP registers exclusive access */

	struct vb2_queue vq;
	struct list_head capture;
	unsigned int sequence;
	unsigned int frame_err;
	unsigned int frame_drop;
	unsigned int incomplete_frame_nb;
	struct vip_buffer *active; /* current active buffer */
	spinlock_t lock;  /* Used in videobuf2 callback */

	/* used to access capture device */
	struct mutex mutex;

	/* allocator-specific contexts for each plane */
	struct vb2_alloc_ctx *alloc_ctx;

	struct sta_sga sga;

	unsigned int intstatus;
	bool pending;
	unsigned int timeout;

	/* debugfs */
	struct dentry *debugfs_dir;
	struct dentry *debugfs_device;
	struct dentry *debugfs_hw;
	struct dentry *debugfs_last;
	unsigned char str[STR_LENGTH];

	struct vip_perf perf;
};

#define VIP_TIMEOUT 5

enum vip_dma_burst_size {
	VIP_DMA_BSIZE_1 = 0,
	VIP_DMA_BSIZE_4,
	VIP_DMA_BSIZE_8,
	VIP_DMA_BSIZE_16,
	VIP_DMA_BSIZE_32,
	VIP_DMA_BSIZE_64,
	VIP_DMA_BSIZE_128,
	VIP_DMA_BSIZE_256,
};

/* PrimeCell DMA extension */
struct burst_table {
	u32 burstwords;
	u32 reg;
};

static const struct burst_table burst_sizes[] = {
	{ .burstwords = 256, .reg = VIP_DMA_BSIZE_256 },
	{ .burstwords = 128, .reg = VIP_DMA_BSIZE_128 },
	{ .burstwords = 64, .reg = VIP_DMA_BSIZE_64 },
	{ .burstwords = 32, .reg = VIP_DMA_BSIZE_32 },
	{ .burstwords = 16, .reg = VIP_DMA_BSIZE_16 },
	{ .burstwords = 8, .reg = VIP_DMA_BSIZE_8 },
	{ .burstwords = 4, .reg = VIP_DMA_BSIZE_4 },
	{ .burstwords = 0, .reg = VIP_DMA_BSIZE_1 },
};

#define FORMATS_WIDTH	   720	/* Common width for all composite video */
#define FORMATS_50_HEIGHT  576	/* For PAL/SECAM */
#define FORMATS_60_HEIGHT  480	/* For NTSC */
#define VIP_FIELD V4L2_FIELD_INTERLACED

static struct v4l2_pix_format formats_50[] = {
	{			/* PAL */
		.width = FORMATS_WIDTH,
		.height = FORMATS_50_HEIGHT,
		.pixelformat = V4L2_PIX_FMT_UYVY,
		.field = VIP_FIELD,
		.bytesperline = FORMATS_WIDTH * 2,
		.sizeimage = FORMATS_WIDTH * 2 * FORMATS_50_HEIGHT,
		.colorspace = V4L2_COLORSPACE_SMPTE170M
	},
};

static struct v4l2_pix_format formats_60[] = {
	{			/* NTSC */
		.width = FORMATS_WIDTH,
		.height = FORMATS_60_HEIGHT,
		.pixelformat = V4L2_PIX_FMT_UYVY,
		.field = VIP_FIELD,
		.bytesperline = FORMATS_WIDTH * 2,
		.sizeimage = FORMATS_WIDTH * 2 * FORMATS_60_HEIGHT,
		.colorspace = V4L2_COLORSPACE_SMPTE170M
	},
};

