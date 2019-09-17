/*
 * Clock driver for STMicroelectronics Automotive SoCs
 * Copyright (C) 2015 STMicroelectronics
 * License terms: GNU General Public License (GPL) version 2
 * Author: Jean-Nicolas Graux <jean-nicolas.graux@st.com>
 */

#define pr_fmt(fmt) "clk-sta: " fmt

#include <linux/module.h>
#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/clkdev.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/clk-provider.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/spinlock.h>
#include <linux/clk-provider.h>
#include <linux/regmap.h>
#include <linux/mfd/syscon.h>

#include <dt-bindings/clock/sta-clock.h>

/* SRC_M3 registers */

#define SRCM3_CR				0x00
#define SRCM3_CR_MODECR				(0x7 << 0)
#define SRCM3_CR_MODESTATUS			(0xF << 3)
#define SRCM3_CR_REMAPCLR			BIT(8)
#define SRCM3_CR_REMAPSTAT			BIT(9)
#define SRCM3_CR_RTTEN				BIT(10)
#define SRCM3_CR_Timerxsel_T0_MTU0		BIT(16)
#define SRCM3_CR_Timerxsel_T1_MTU0		BIT(17)
#define SRCM3_CR_Timerxsel_T2_MTU0		BIT(18)
#define SRCM3_CR_Timerxsel_T3_MTU0		BIT(19)
#define SRCM3_CR_Timerxsel_T0_MTU1		BIT(20)
#define SRCM3_CR_Timerxsel_T1_MTU1		BIT(21)
#define SRCM3_CR_Timerxsel_T2_MTU1		BIT(22)
#define SRCM3_CR_Timerxsel_T3_MTU1		BIT(23)
#define SRCM3_CR_Timerxsel_MTU0			(0xF << 16)
#define SRCM3_CR_Timerxsel_MTU1			(0xF << 20)

#define SRCM3_IMCTRL				0x04
#define SRCM3_IMCTRL_PLL2IM			BIT(0)
#define SRCM3_IMCTRL_PLL4IM			BIT(1)
#define SRCM3_IMCTRL_ECCERRIM			(0xFF << 8)
#define SRCM3_IMCTRL_DB_ECCERRIM		(0xFF << 16)

#define SRCM3_IMSTAT				0x08
#define SRCM3_IMSTAT_PLL2MIS			BIT(0)
#define SRCM3_IMSTAT_PLL4MIS			BIT(1)
#define SRCM3_IMSTAT_ECC_ERR			(0xFF << 8)
#define SRCM3_IMSTAT_DB_ECC_ERR			(0xFF << 16)

#define SRCM3_XCTRL				0x0C
#define SRCM3_XCTRL_MXTALOVER			BIT(0)
#define SRCM3_XCTRL_MXTALEN			BIT(1)
#define SRCM3_XCTRL_XTALTIMEN			BIT(2)
#define SRCM3_XCTRL_MXTALSTAT			BIT(3)
/*#define SRCM3_XCTRL_MXTALTEMPR_DIS		BIT(4) ???*/
#define SRCM3_XCTRL_MXTALTIME			(0xFFFF << 16)

#define SRCM3_PLLCTRL				0x10
#define SRCM3_PLLCTRL_PLL2OVER			BIT(0)
#define SRCM3_PLLCTRL_PLL2EN			BIT(1)
#define SRCM3_PLLCTRL_PLL2STAT			BIT(2)
#define SRCM3_PLLCTRL_PLL3EN			BIT(5)
#define SRCM3_PLLCTRL_PLL3STAT			BIT(6)
#define SRCM3_PLLCTRL_PLLTIMEEN			BIT(7)
#define SRCM3_PLLCTRL_PLLDEN			BIT(8)
#define SRCM3_PLLCTRL_PLLDSTAT			BIT(9)
#define SRCM3_PLLCTRL_PLL4EN			BIT(10)
#define SRCM3_PLLCTRL_PLL4STAT			BIT(11)
#define SRCM3_PLLCTRL_PLL2TIMEOUT_COUNT		(0xFFFF << 16)

#define SRCM3_PLL2FCTRL				0x18
#define SRCM3_PLL2FCTRL_ODF			(0xF << 0)
#define SRCM3_PLL2FCTRL_IDF			(0x7 << 4)
#define SRCM3_PLL2FCTRL_ODF_STROBE		BIT(7)
#define SRCM3_PLL2FCTRL_NDIV			(0xFF << 16)

#define SRCM3_PLL3FCTRL				0x1C
#define SRCM3_PLL3FCTRL_IDF			(0x7 << 4)
#define SRCM3_PLL3FCTRL_NDIV			(0xFF << 16)
#define SRCM3_PLL3FCTRL_CP			(0xF << 24)

#define SRCM3_PLL3FCTRL2			0x20
#define SRCM3_PLL3FCTRL2_FRC_INPUT		(0xFFFF << 0)

#define SRCM3_RESSTAT				0x24
#define SRCM3_RESSTAT_PORRST			BIT(0)
#define SRCM3_RESSTAT_PADRST			BIT(1)
#define SRCM3_RESSTAT_WDGRST			BIT(2)
#define SRCM3_RESSTAT_SOFTRST			BIT(3)
#define SRCM3_RESSTAT_M3_SOFTRESREQ		BIT(4)
#define SRCM3_RESSTAT_NCPUHALT			BIT(5)
#define SRCM3_RESSTAT_REMAP0			BIT(7)
#define SRCM3_RESSTAT_REMAP1			BIT(8)
#define SRCM3_RESSTAT_MXTAL_FREQ_SEL		BIT(9)
#define SRCM3_RESSTAT_MXTALFREQSEL_SHIFT	9
#define SRCM3_RESSTAT_MXTALFREQSEL_STA1385_MSK	GENMASK(10, 9)

#define SRCM3_PCKEN1				0x30
#define SRCM3_PCKDIS1				0x34
#define SRCM3_PCKENSR1				0x38
#define SRCM3_PCKSR1				0x3C

#define SRCM3_CLKOCR				0x40
#define SRCM3_CLKOCR_CLKODIV0			(0x3F << 0)
#define SRCM3_CLKOCR_CLKOSEL0			(0x7 << 6)
#define SRCM3_CLKOCR_CLKODIV1			(0x3F << 16)
#define SRCM3_CLKOCR_CLKOSEL1			(0x7 << 22)

#define SRCM3_CLKCR				0x44
#define SRCM3_CLKCR_CLCD			BIT(0)
#define SRCM3_CLKCR_SQI				(0x3 << 1)
#define SRCM3_CLKCR_SDIO0			(0x3 << 3)
#define SRCM3_CLKCR_AUDIOSSMCLK			BIT(5)
#define SRCM3_CLKCR_AUDIOFS512			BIT(6)
#define SRCM3_CLKCR_CANSS			BIT(7)
#define SRCM3_CLKCR_SDMMC1			(0x3 << 8)
#define SRCM3_CLKCR_AUDIO_CLK_RST		BIT(10)
#define SRCM3_CLKCR_CLCD_DIV			(0x3F << 11)
#define SRCM3_CLKCR_GFX				BIT(17)
#define SRCM3_CLKCR_VDO				BIT(18)
#define SRCM3_CLKCR_SDMMC2			(0x3 << 19)
#define SRCM3_CLKCR_PIXCLK_AVL			BIT(21)

#define SRCM3_DEBUG				0x48

#define SRCM3_PLLDFCTRL				0x4C
#define SRCM3_PLLDFCTRL_NDIV			(0x7F << 16)
#define SRCM3_PLLDFCTRL2			0x50
#define SRCM3_PLLDFCTRL2_FRC_INPUT		(0xFFFF << 0)

#define SRCM3_PLL4FCTRL				0x54
#define SRCM3_PLL4FCTRL_IDF			(0x7 << 4)
#define SRCM3_PLL4FCTRL_NDIV			(0xFF << 16)
#define SRCM3_PLL4FCTRL_CP			(0xF << 24)

#define SRCM3_PLL4SSCGCTRL			0x58
#define SRCM3_PLL4SSCGCTRL_MOD			(0x1FFF << 0)
#define SRCM3_PLL4SSCGCTRL_STROBE_CTRL		BIT(13)
#define SRCM3_PLL4SSCGCTRL_STROBE_BYPASS	BIT(14)
#define SRCM3_PLL4SSCGCTRL_SPREAD_CTRL		BIT(15)
#define SRCM3_PLL4SSCGCTRL_SSCG			BIT(16)
#define SRCM3_PLL4SSCGCTRL_INC_STEP		(0x7FFF << 17)

#define SRCM3_ECCFAILADD			0x5C
#define SRCM3_ECCFAILADD_CLEAR			BIT(0)
#define SRCM3_ECCFAILADD_FAILADD		(0xFFF << 1)

#define SRCM3_ECCFORCEADD			0x60
#define SRCM3_ECCFORCEADD_SINGLE_ERR		(0xF << 0)
#define SRCM3_ECCFORCEADD_DOUBLE_ERR		(0xF << 8)
#define SRCM3_ECCFORCEADD_ECCBYPASS		BIT(31)

#define SRCM3_PCKEN2				0x68
#define SRCM3_PCKDIS2				0x6C
#define SRCM3_PCKENSR2				0x70
#define SRCM3_PCKSR2				0x74

#define SRCM3_CLKOCR1				0x78
#define SRCM3_CLKOCR1_CLKODIV2			(0x3F << 0)
#define SRCM3_CLKOCR1_CLKOSEL2			(0x7 << 6)
#define SRCM3_CLKOCR1_CLKODIV3			(0x3F << 9)
#define SRCM3_CLKOCR1_CLKOSEL3			(0x7 << 15)
#define SRCM3_CLKOCR1_CLKODIV4			(0x3F << 18)

#define MODESTATUS_NORMAL			4

/* SRC_A7 registers */

#define SRCA7_IMCTRL_LO			0x00
#define SRCA7_IMCTRL_LO_PLL1_LOSSLOCKEN	BIT(0)
#define SRCA7_IMCTRL_LO_PLL3_LOSSLOCKEN	BIT(1)
#define SRCA7_IMCTRL_LO_PLLD_LOSSLOCKEN	BIT(3)
#define SRCA7_IMCTRL_LO_ECC_INTEN		(0xFFFF << 16)

#define SRCA7_IMSTAT_LO			0x04
#define SRCA7_IMSTAT_LO_PLL1_LOCKLOST		BIT(0)
#define SRCA7_IMSTAT_LO_PLL3_LOCKLOST		BIT(1)
#define SRCA7_IMSTAT_LO_PLLD_LOCKLOST		BIT(3)

#define SRCA7_PLL3FCTRL			0x18
#define SRCA7_PLL3FCTRL_ODF			(0xF << 0)
#define SRCA7_PLL3FCTRL_STRB_ODF		BIT(4)
#define SRCA7_PLL3FCTRL_STRB			BIT(5)
#define SRCA7_PLL3FCTRL_STRB_BYPS		BIT(6)
#define SRCA7_PLL3FCTRL_FRAC_CTRL		BIT(7)
#define SRCA7_PLL3FCTRL_DITHER_DIS		(0x3 << 8)

#define SRCA7_RESSTAT				0x1C
#define SRCA7_RESSTAT_SFTRST			BIT(0)
#define SRCA7_RESSTAT_WDGRST			BIT(1)
#define SRCA7_RESSTAT_NRST			BIT(2)
#define SRCA7_RESSTAT_REMAP0			BIT(3)
#define SRCA7_RESSTAT_REMAP1			BIT(4)
#define SRCA7_RESSTAT_MXTALFREQSEL		BIT(5)
#define SRCA7_RESSTAT_MXTALFREQSEL_SHIFT	5
#define SRCA7_RESSTAT_MXTALFREQSEL_STA1385_MSK	GENMASK(6, 5)

#define SRCA7_SCCLKDIVCR			0x20
#define SRCA7_SCCLKDIVCR_HCLK_DIV		(0x7 << 0)
#define SRCA7_SCCLKDIVCR_ETMCLK_DIV		(0x3 << 7)

#define SRCA7_PCKEN0				0x28
#define SRCA7_PCKDIS0				0x2C
#define SRCA7_PCKENSR0				0x30
#define SRCA7_PCKSR0				0x34

#define SRCA7_SCDEBUG				0x38
#define SRCA7_SCDEBUG_GPDBG			(0xFFFF << 0)

#define SRCA7_SCPLLDFCTRL			0x3C
#define SRCA7_SCPLLDFCTRL_ODF			(0x7 << 0)

#define SRCA7_SCRESCTRL1			0x40
#define SRCA7_SCRESCTRL1_GFC_RST		BIT(14)
#define SRCA7_SCRESCTRL1_AXI_DDR_RST		BIT(15)

#define SRCA7_PCKEN1				0x44
#define SRCA7_PCKDIS1				0x48
#define SRCA7_PCKENSR1				0x4C
#define SRCA7_PCKSR1				0x50

/* A7_SSC clock registers */

#define A7SSC_SET				0x4
#define A7SSC_CLEAR				0x8
#define A7SSC_TOGGLE				0xC

#define A7SSC_CHCLKREQ				0x00
#define A7SSC_CHCLKREQ_CHGCLKREQ		BIT(0)
#define A7SSC_CHCLKREQ_DIVSEL			BIT(16)

#define A7SSC_BRM				0x10
#define A7SSC_BRM_RATEVAL			BIT(0)
#define A7SSC_BRM_RATE				(0x3F << 8)
#define A7SSC_BRM_IDLEDIVEN			BIT(16)
#define A7SSC_BRM_IDLEDIV			(0x3 << 24)

#define A7SSC_PLLARMFREQ			0x80
#define A7SSC_PLLARMFREQ_NDIV			(0xFF << 0)
#define A7SSC_PLLARMFREQ_ODF			(0x3F << 10)
#define A7SSC_PLLARMFREQ_IDF			(0x7 << 16)
#define A7SSC_PLLARMFREQ_CP			(0x1F << 19)
#define A7SSC_PLLARMFREQ_STRB_ODF		BIT(24)

#define A7SSC_PLLARMCTRL			0x90
#define A7SSC_PLLARMCTRL_ENABLE			BIT(0)
#define A7SSC_PLLARMCTRL_STRB_BYPASS		BIT(1)
#define A7SSC_PLLARMCTRL_STRB			BIT(2)
#define A7SSC_PLLARMCTRL_SPREAD_CTRL		BIT(3)
#define A7SSC_PLLARMCTRL_FRAC_CTRL		BIT(4)
#define A7SSC_PLLARMCTRL_SSCG_CTRL		BIT(5)
#define A7SSC_PLLARMCTRL_DITHER_DISABLE		(0x3 << 6)
#define A7SSC_PLLARMCTRL_FRACINPUT		(0xFFFF << 16)

#define A7SSC_PLLARMLOCKP			0xA0
#define A7SSC_PLLARMLOCKP_PLLLOCKP3		BIT(1)
#define A7SSC_PLLARMLOCKP_PLLCLKGOOD		BIT(2)

/* the peripheral clock controllers */
enum sta_clock_ctrls {
	SRCM3,
	SRCA7,
	A7SSC,
	A7MSCR,
};

enum {
	PLL1 = 1,
	PLL2,
	PLL3,
	PLL4,
	PLLD,
};

static struct regmap_config regmap_configs[] = {
	[SRCM3] = {
		.reg_bits = 32,
		.val_bits = 32,
		.reg_stride = 4,
		.val_format_endian = REGMAP_ENDIAN_LITTLE,
		.max_register = 0x7C,
		.fast_io = true,
	},
	[SRCA7] = {
		.reg_bits = 32,
		.val_bits = 32,
		.reg_stride = 4,
		.val_format_endian = REGMAP_ENDIAN_LITTLE,
		.max_register = 0x54,
		.fast_io = true,
	},
	[A7SSC] = {
		.reg_bits = 32,
		.val_bits = 32,
		.reg_stride = 4,
		.val_format_endian = REGMAP_ENDIAN_LITTLE,
		.max_register = 0xB4,
		.fast_io = true,
	},
	[A7MSCR] = {
		.reg_bits = 32,
		.val_bits = 32,
		.reg_stride = 4,
		.val_format_endian = REGMAP_ENDIAN_LITTLE,
		.max_register = 0x110,
		.fast_io = true,
	}
};

static struct regmap *srcm3_regmap;
static struct regmap *srca7_regmap;
static struct regmap *a7ssc_regmap;
static struct regmap *a7mscr_regmap;

/**
 * STA1385 SoC clock tree is inherited from STA1295 one but with some few
 * differencies:
 * - MXTAL_FREQSEL is encoded with several bits instead of 1.
 * - There are some additional bus clock gates encoded in SRCM3_PCKEN2.
 * - There are some additional clock divider and clock mux for uart4, uart5
 *   defined in A7 MSCR register bank.
 */
static bool is_sta1385_compatible;

enum {
	/* Start from the last defined clock in dt bindings */
	STA_CLK_ETMSEL = STA_CLK_PLL3PHI + 1,
	STA_CLK_GFXSEL,
	STA_CLK_VDOSEL,
	STA_CLK_SDIOSEL,
	STA_CLK_SDMMC1SEL,
	STA_CLK_SDMMC2SEL,
	STA_CLK_SQIOSEL,
	STA_CLK_AUDIOSSMSEL,
	STA_CLK_AUDIOSSFS512SEL,
	STA_CLK_PADPIX,
	STA_CLK_CLCDPLLSEL,
	STA_CLK_CLCDDIV,
	STA_CLK_CLCDSEL,
	STA_CLK_CANSSSEL,
	STA_CLK_UART45DIV,
	STA_CLK_UART45SEL,

	STA_CLK_PLL3FVCO,

	STA_CLK_PLL4FVCOBY2,
	STA_CLK_PLL4FVCOBY2BY2,
	STA_CLK_PLL4FVCOBY2BY3,
	STA_CLK_PLL4FVCOBY2BY4,
	STA_CLK_PLL4FVCOBY2BY12,
	STA_CLK_PLL4FVCOBY3,
	STA_CLK_PLL4FVCOBY3BY6,
	STA_CLK_PLL4PHI,
	STA_CLK_PLL4FVCO,

	STA_CLK_PLLDPHI,
	STA_CLK_PLLDFVCO,

	STA_CLK_PLLUPHIBY6,
	STA_CLK_PLLUPHIBY12,
	STA_CLK_PLLUPHI,
	STA_CLK_PLLUFVCO,

	STA_CLK_PLL2FVCOBY2,
	STA_CLK_PLL2FVCOBY2BY3,
	STA_CLK_PLL2FVCOBY2BY4,
	STA_CLK_PLL2FVCOBY2BY5,
	STA_CLK_PLL2FVCOBY2BY6,
	STA_CLK_PLL2FVCOBY2BY7,
	STA_CLK_PLL2FVCOBY2BY12,
	STA_CLK_PLL2FVCOBY2BY25,
	STA_CLK_PLL2FVCOBY2BY26,
	STA_CLK_PLL2FVCOBY3,
	STA_CLK_PLL2PHI,
	STA_CLK_PLL2FVCO,

	STA_CLK_PLL1FVCOBY2,
	STA_CLK_PLL1FVCOBY3,
	STA_CLK_PLL1PHI,
	STA_CLK_PLL1FVCO,

	STA_CLK_RNG,
	STA_CLK_OTPCC,
	STA_CLK_TSENSEDIV,
	STA_CLK_CPUEXT,
	STA_CLK_RINGOSC,
	STA_CLK_RTCOSC,

	STA_CLK_MXTALBY2,
	STA_CLK_MXTAL,
	STA_CLK_MAX,
};

static struct clk_hw_onecell_data clk_data = {
	.num = STA_CLK_MAX,
	.hws = {
		[STA_CLK_MAX - 1] = NULL,
	},
};

#define STA_CLK_PARENTS_MAX 5

struct clk_proto {
	const char *name;
	const char *parents[STA_CLK_PARENTS_MAX];
	u8 num_parents;
	unsigned long flags;
};

#define CLK_PREFIX(LITERAL)		STA_CLK_ ## LITERAL
#define NUMARGS(...)	(sizeof((char *[]){__VA_ARGS__}) / sizeof(char *))

#define STA_CLK_DEFINE(_idx, _name, _flags, ...)		\
	[CLK_PREFIX(_idx)] = {					\
		.name = _name,					\
		.flags = _flags,				\
		.parents = { __VA_ARGS__ },			\
		.num_parents = NUMARGS(__VA_ARGS__),		\
	}

#define GATE_FLAGS \
	(CLK_SET_RATE_PARENT | CLK_SET_RATE_GATE | CLK_IGNORE_UNUSED)

static const struct clk_proto clk_protos[STA_CLK_MAX] __initconst = {
	STA_CLK_DEFINE(MXTAL, "mxtal", 0x0),
	STA_CLK_DEFINE(RTCOSC, "rtcosc", 0x0),
	STA_CLK_DEFINE(RINGOSC, "ringosc", 0x0),
	STA_CLK_DEFINE(MXTALBY2, "mxtalby2", 0x0, "mxtal"),
	STA_CLK_DEFINE(CPUEXT, "cpuext", 0x0, "mxtal"),
	STA_CLK_DEFINE(CPU, "cpu", CLK_SET_RATE_PARENT, "pll1phi", "cpuext"),
	STA_CLK_DEFINE(TSENSEDIV, "tsensediv", 0x0, "mxtal"),
	STA_CLK_DEFINE(TIMER0, "timer0", 0x0, "mxtal"),
	STA_CLK_DEFINE(TIMER1, "timer1", 0x0, "mxtal"),
	STA_CLK_DEFINE(WDT, "wdt", 0x0, "mxtal"),
	STA_CLK_DEFINE(RNG, "rng", 0x0, "ringosc"),
	STA_CLK_DEFINE(OTPCC, "otpcc", 0x0, "mxtal"),
	STA_CLK_DEFINE(CAN0, "can0", 0x0, "pll2fvcoby2by12"),

	/* plls */
	STA_CLK_DEFINE(PLL1FVCO, "pll1fvco", CLK_IGNORE_UNUSED, "mxtal"),
	STA_CLK_DEFINE(PLL1PHI, "pll1phi", 0x0, "pll1fvco"),
	STA_CLK_DEFINE(PLL1FVCOBY2, "pll1fvcoby2", 0x0, "pll1fvco"),
	STA_CLK_DEFINE(PLL1FVCOBY3, "pll1fvcoby3", 0x0, "pll1fvco"),

	STA_CLK_DEFINE(PLL2FVCO, "pll2fvco", CLK_IGNORE_UNUSED, "mxtal"),
	STA_CLK_DEFINE(PLL2PHI, "pll2phi", 0x0, "pll2fvco"),
	STA_CLK_DEFINE(PLL2FVCOBY2, "pll2fvcoby2", 0x0, "pll2fvco"),
	STA_CLK_DEFINE(PLL2FVCOBY3, "pll2fvcoby3", 0x0, "pll2fvco"),
	STA_CLK_DEFINE(PLL2FVCOBY2BY3, "pll2fvcoby2by3", 0x0, "pll2fvcoby2"),
	STA_CLK_DEFINE(PLL2FVCOBY2BY4, "pll2fvcoby2by4", 0x0, "pll2fvcoby2"),
	STA_CLK_DEFINE(PLL2FVCOBY2BY5, "pll2fvcoby2by5", 0x0, "pll2fvcoby2"),
	STA_CLK_DEFINE(PLL2FVCOBY2BY6, "pll2fvcoby2by6", 0x0, "pll2fvcoby2"),
	STA_CLK_DEFINE(PLL2FVCOBY2BY7, "pll2fvcoby2by7", 0x0, "pll2fvcoby2"),
	STA_CLK_DEFINE(PLL2FVCOBY2BY12, "pll2fvcoby2by12", 0x0, "pll2fvcoby2"),
	STA_CLK_DEFINE(PLL2FVCOBY2BY25, "pll2fvcoby2by25", 0x0, "pll2fvcoby2"),
	STA_CLK_DEFINE(PLL2FVCOBY2BY26, "pll2fvcoby2by26", 0x0, "pll2fvcoby2"),

	STA_CLK_DEFINE(PLL3FVCO, "pll3fvco", 0x0, "mxtal"),
	STA_CLK_DEFINE(PLL3PHI, "pll3phi", 0x0, "pll3fvco"),

	STA_CLK_DEFINE(PLL4FVCO, "pll4fvco", 0x0, "mxtal"),
	STA_CLK_DEFINE(PLL4PHI, "pll4phi", 0x0, "pll4fvco"),
	STA_CLK_DEFINE(PLL4FVCOBY2, "pll4fvcoby2", 0x0, "pll4fvco"),
	STA_CLK_DEFINE(PLL4FVCOBY2BY2, "pll4fvcoby2by2", 0x0, "pll4fvcoby2"),
	STA_CLK_DEFINE(PLL4FVCOBY2BY3, "pll4fvcoby2by3", 0x0, "pll4fvcoby2"),
	STA_CLK_DEFINE(PLL4FVCOBY2BY4, "pll4fvcoby2by4", 0x0, "pll4fvcoby2"),
	STA_CLK_DEFINE(PLL4FVCOBY2BY12, "pll4fvcoby2by12", 0x0, "pll4fvcoby2"),
	STA_CLK_DEFINE(PLL4FVCOBY3, "pll4fvcoby3", 0x0, "pll4fvco"),
	STA_CLK_DEFINE(PLL4FVCOBY3BY6, "pll4fvcoby3by6", 0x0, "pll4fvcoby3"),

	STA_CLK_DEFINE(PLLDFVCO, "plldfvco", CLK_IGNORE_UNUSED, "mxtal"),
	STA_CLK_DEFINE(PLLDPHI, "plldphi", 0x0, "plldfvco"),

	STA_CLK_DEFINE(PLLUFVCO, "pllufvco", 0x0, "mxtal"),
	STA_CLK_DEFINE(PLLUPHI, "plluphi", 0x0, "pllufvco"),
	STA_CLK_DEFINE(PLLUPHIBY6, "plluphiby6", 0x0, "plluphi"),
	STA_CLK_DEFINE(PLLUPHIBY12, "plluphiby12", 0x0, "plluphi"),

	/* hclk */
	STA_CLK_DEFINE(HCLKDIV, "hclkdiv", 0x0, "pll1fvcoby2"),

	/* dividers, muxes */
	STA_CLK_DEFINE(ETMSEL, "etmsel", CLK_SET_RATE_PARENT, "pll4fvcoby2by2",
		       "pll4fvcoby2by3",
		       "pll4fvcoby2by4", "pll4fvcoby2by4"),
	STA_CLK_DEFINE(GFXSEL, "gfxsel", CLK_SET_RATE_PARENT, "pll4fvcoby2",
		       "pll2fvcoby3"),
	STA_CLK_DEFINE(VDOSEL, "vdosel", CLK_SET_RATE_PARENT, "pll4phi",
		       "pll2fvcoby2by3"),
	STA_CLK_DEFINE(SDIOSEL, "sdiosel", 0x0, "pll2fvcoby2by7", "pll2phi",
		       "pll2fvcoby2by6"),
	STA_CLK_DEFINE(SDMMC1SEL, "sdmmc1sel", 0x0, "pll2fvcoby2by7",
		       "pll2fvcoby2by6", "pll2phi"),
	STA_CLK_DEFINE(SDMMC2SEL, "sdmmc2sel", 0x0, "pll2fvcoby2by7",
		       "pll2fvcoby2by6", "pll2phi"),
	STA_CLK_DEFINE(SQIOSEL, "sqiosel", 0x0, "pll2fvcoby2by4",
		       "pll2fvcoby2by3", "pll2phi"),
	STA_CLK_DEFINE(AUDIOSSMSEL, "audiossmsel", 0x0, "pll2fvcoby2by5",
		       "pll2fvcoby2by4"),
	STA_CLK_DEFINE(AUDIOSSFS512SEL, "audiossfs512sel", 0x0,
		       "pll2fvcoby2by25", "pll2fvcoby2by26"),
	STA_CLK_DEFINE(PADPIX, "padpix", 0x0),
	STA_CLK_DEFINE(CLCDPLLSEL, "clcdpllsel", CLK_SET_RATE_PARENT,
		       "pll2fvcoby2", "pll4fvcoby2"),
	STA_CLK_DEFINE(CLCDDIV, "clcddiv", CLK_SET_RATE_PARENT, "clcdpllsel"),
	STA_CLK_DEFINE(CLCDSEL, "clcdsel", CLK_SET_RATE_PARENT, "clcddiv",
		       "padpix"),
	STA_CLK_DEFINE(CANSSSEL, "cansssel", 0x0, "pll2fvcoby2by3",
		       "pll2fvcoby2by4"),
	STA_CLK_DEFINE(UART45SEL, "uart45sel", 0x0, "pll4fvcoby3",
		       "pll4fvcoby2", "pll1fvcoby3"),
	STA_CLK_DEFINE(UART45DIV, "uart45div", CLK_SET_RATE_PARENT, "uart45sel"),

	/* peripheral bus clock gates */
	STA_CLK_DEFINE(HDMA0, "hdma0", GATE_FLAGS, "hclkdiv"),
	STA_CLK_DEFINE(HDMA1, "hdma1", GATE_FLAGS, "hclkdiv"),
	STA_CLK_DEFINE(HFSMC, "hfsmc", GATE_FLAGS, "hclkdiv"),
	STA_CLK_DEFINE(PSSP0, "pssp0", GATE_FLAGS, "hclkdiv"),
	STA_CLK_DEFINE(HCLCD, "hclcd", GATE_FLAGS, "hclkdiv"),
	STA_CLK_DEFINE(PSSP1, "pssp1", GATE_FLAGS, "hclkdiv"),
	STA_CLK_DEFINE(PSSP2, "pssp2", GATE_FLAGS, "hclkdiv"),
	STA_CLK_DEFINE(PSDIO, "psdio", GATE_FLAGS, "hclkdiv"),
	STA_CLK_DEFINE(PSDMMC1, "psdmmc1", GATE_FLAGS, "hclkdiv"),
	STA_CLK_DEFINE(PI2C1, "pi2c1", GATE_FLAGS, "hclkdiv"),
	STA_CLK_DEFINE(PI2C2, "pi2c2", GATE_FLAGS, "hclkdiv"),
	STA_CLK_DEFINE(PUART1, "puart1", GATE_FLAGS, "hclkdiv"),
	STA_CLK_DEFINE(PUART2, "puart2", GATE_FLAGS, "hclkdiv"),
	STA_CLK_DEFINE(PUART3, "puart3", GATE_FLAGS, "hclkdiv"),
	STA_CLK_DEFINE(PHSEM, "phsem", GATE_FLAGS, "hclkdiv"),
	STA_CLK_DEFINE(HGAE, "hgae", GATE_FLAGS, "hclkdiv"),
	STA_CLK_DEFINE(HVIP, "hvip", GATE_FLAGS, "hclkdiv"),
	STA_CLK_DEFINE(HUSB, "husb", GATE_FLAGS, "hclkdiv"),
	STA_CLK_DEFINE(PAPBREG, "papbreg", GATE_FLAGS, "hclkdiv"),
	STA_CLK_DEFINE(PSARADC, "psaradc", GATE_FLAGS, "hclkdiv"),
	STA_CLK_DEFINE(PMSP0, "pmsp0", GATE_FLAGS, "hclkdiv"),
	STA_CLK_DEFINE(PMSP1, "pmsp1", GATE_FLAGS, "hclkdiv"),
	STA_CLK_DEFINE(PMSP2, "pmsp2", GATE_FLAGS, "hclkdiv"),
	STA_CLK_DEFINE(PCDSUBSYS, "pcdsubsys", GATE_FLAGS, "hclkdiv"),
	STA_CLK_DEFINE(HGFX, "hgfx", GATE_FLAGS, "hclkdiv"),
	STA_CLK_DEFINE(AETH, "aeth", GATE_FLAGS, "hclkdiv"),
	STA_CLK_DEFINE(HC3, "hc3", GATE_FLAGS, "hclkdiv"),
	STA_CLK_DEFINE(ADDRC, "addrc", GATE_FLAGS, "hclkdiv"),
	STA_CLK_DEFINE(AA7, "aa7", GATE_FLAGS, "hclkdiv"),
	STA_CLK_DEFINE(PI2C0, "pi2c0", GATE_FLAGS, "hclkdiv"),
	STA_CLK_DEFINE(PUART0, "puart0", GATE_FLAGS, "hclkdiv"),

	STA_CLK_DEFINE(PSDMMC2, "psdmmc2", GATE_FLAGS, "hclkdiv"),
	STA_CLK_DEFINE(TRACEM3, "tracem3", GATE_FLAGS, "etmsel"),
	STA_CLK_DEFINE(TRACEA7, "tracea7", GATE_FLAGS, "etmsel"),
	STA_CLK_DEFINE(ATA7, "ata7", GATE_FLAGS, "pll4fvcoby2by2"),
	STA_CLK_DEFINE(ATDEBUG, "atdebug", GATE_FLAGS, "pll4fvcoby2by12"),
	STA_CLK_DEFINE(PDDRCTRL, "pddrctrl", GATE_FLAGS, "hclkdiv"),
	STA_CLK_DEFINE(AGFX, "agfx", GATE_FLAGS, "hclkdiv"),

	/* peripheral functional clock gates */
	STA_CLK_DEFINE(CLCD, "clcd", GATE_FLAGS, "clcdsel"),
	STA_CLK_DEFINE(SSP0, "ssp0", GATE_FLAGS, "pll2fvcoby2by6"),
	STA_CLK_DEFINE(UART0, "uart0", GATE_FLAGS, "pll2fvcoby2by12"),
	STA_CLK_DEFINE(SDIO, "sdio", GATE_FLAGS, "sdiosel"),
	STA_CLK_DEFINE(I2C0, "i2c0", GATE_FLAGS, "pll2fvcoby2by12"),
	STA_CLK_DEFINE(I2C1, "i2c1", GATE_FLAGS, "pll2fvcoby2by12"),
	STA_CLK_DEFINE(UART1, "uart1", GATE_FLAGS, "pll2fvcoby2by12"),
	STA_CLK_DEFINE(SQIO, "sqio", GATE_FLAGS, "sqiosel"),
	STA_CLK_DEFINE(AUDIOSSFS512, "audiossfs512", GATE_FLAGS,
		       "audiossfs512sel"),
	STA_CLK_DEFINE(AUDIOSSM, "audiossm", GATE_FLAGS, "audiossmsel"),
	STA_CLK_DEFINE(EMR, "emr", GATE_FLAGS, "pll2fvcoby2by4"),
	STA_CLK_DEFINE(SSP2, "ssp2", GATE_FLAGS, "pll2fvcoby2by6"),
	STA_CLK_DEFINE(UART2, "uart2", GATE_FLAGS, "pll2fvcoby2by12"),
	STA_CLK_DEFINE(SDMMC1, "sdmmc1", GATE_FLAGS, "sdmmc1sel"),
	STA_CLK_DEFINE(SSP1, "ssp1", GATE_FLAGS, "pll2fvcoby2by6"),
	STA_CLK_DEFINE(I2C2, "i2c2", GATE_FLAGS, "pll2fvcoby2by12"),
	STA_CLK_DEFINE(UART3, "uart3", GATE_FLAGS, "pll2fvcoby2by12"),
	STA_CLK_DEFINE(MSP0, "msp0", GATE_FLAGS, "pll2fvcoby2by12"),
	STA_CLK_DEFINE(MSP1, "msp1", GATE_FLAGS, "pll2fvcoby2by12"),
	STA_CLK_DEFINE(MSP2, "msp2", GATE_FLAGS, "pll2fvcoby2by12"),
	STA_CLK_DEFINE(A7CNT, "a7cnt", GATE_FLAGS, "pll2fvcoby2by12"),
	STA_CLK_DEFINE(A7DBG, "a7dbg", GATE_FLAGS, "pll2fvcoby2by12"),
	STA_CLK_DEFINE(SDMMC2, "sdmmc2", GATE_FLAGS, "sdmmc2sel"),
	STA_CLK_DEFINE(C3RNG, "c3rng", GATE_FLAGS, "rng"),
	STA_CLK_DEFINE(SAFMEM, "safmem", GATE_FLAGS, "mxtal"),
	STA_CLK_DEFINE(DDR, "ddr", GATE_FLAGS, "plldphi"),
	STA_CLK_DEFINE(GFX2X, "gfx2x", GATE_FLAGS, "gfxsel"),
	STA_CLK_DEFINE(VDO, "vdo", GATE_FLAGS, "vdosel"),
	STA_CLK_DEFINE(A7P, "a7p", GATE_FLAGS, "pll2fvcoby2by12"),
	STA_CLK_DEFINE(A7EXT2F, "a7ext2f", GATE_FLAGS, "pll2fvcoby2by12"),
	STA_CLK_DEFINE(TSENSE, "tsense", GATE_FLAGS, "tsensediv"),
	STA_CLK_DEFINE(CANFD, "canfd", GATE_FLAGS, "plluphiby6"),

	STA_CLK_DEFINE(ETH0PTP, "eth0ptp", GATE_FLAGS, "pll2fvcoby2by12"),
	STA_CLK_DEFINE(FDCANP, "fdcanp", GATE_FLAGS, "pll2fvcoby2by12"),
	STA_CLK_DEFINE(FRAYPE, "fraype", GATE_FLAGS, "pll2fvcoby2by12"),
	STA_CLK_DEFINE(ETH1PTP, "eth1ptp", GATE_FLAGS, "pll2fvcoby2by12"),
	STA_CLK_DEFINE(WLAN, "wlan", GATE_FLAGS, "uart45div"),
	STA_CLK_DEFINE(ECULTRA, "ecultra", GATE_FLAGS, "pll2fvcoby2by12"),
	STA_CLK_DEFINE(AES, "aes", GATE_FLAGS, "pll2fvcoby2by12"),
	STA_CLK_DEFINE(SHA, "sha", GATE_FLAGS, "pll2fvcoby2by12"),
};

/**
 * quirk to enable compatibility with old fdcan clock tree
 * on older cut
 */
static bool quirk_canfd_old_cuts;
const char *old_cut_canfd_parents[] = { "plluphiby12" };

/**
 * struct clk_pll - STA SoC PLL clock
 * @hw: corresponding clock hardware entry
 * @id: PLL instance: 1,2 or 3
 * @idf: idf dummy value in case of no access to SRC-M3
 * @ndiv: ndiv dummy value in case of no access to SRC-M3
 */
struct sta_clk_pll {
	struct clk_hw hw;
	int id;
	u32 idf;
	u32 ndiv;
	u32 fract;
	int frac_ctrl : 1;
	int dither : 1;
};

#define STROBE_NA 32

struct sta_clk_div {
	struct clk_hw	hw;
	struct regmap *regmap;
	u8		ctrl;
	u32		reg;
	u8		shift;
	u8		width;
	const struct clk_div_table *table;
	u8		flags;
	u8		strobe_bit;
};

struct sta_clk_mux {
	struct clk_hw	hw;
	struct regmap *regmap;
	u8		ctrl;
	u32		reg;
	u8		shift;
	u32		width;
	u32		*table;
};

/**
 * struct sta_clk_gate - STA SoC clock gate
 * @hw: corresponding clock hardware entry
 * @id: the clock ID
 * @group: stating whether clock bit is part of group 0, 1 or 2:
 *	Let's group the SRC clock gates in the following way:
 *	group 0: [00-31]: SRC-A7-PCKEN0
 *	group 1: [32-63]: SRC-A7-PCKEN1
 *	group 2: [64-95]: SRC-M3-PCKEN1
 *	group 3: [96-127]: SRC-M3-PCKEN2
 * @clkbit: bit 0...31 corresponding to the clock bit inside clock register
 */
struct sta_clk_gate {
	struct clk_hw hw;
	int id;
	int group;
	u32 clkbit;
};

#define STA1385_NUM_GATES 105
#define STA1295_NUM_GATES 98

static const char * const clk_gate_names[STA1385_NUM_GATES] = {
	/* group0 (via SRC_A7) */
	"HCLKDMA0",
	"HCLKDMA1",
	"HCLKSMC",
	"PCLKSSP0",
	"HCLKCLCD",
	"PCLKSSP1",
	"PCLKSSP2",
	"PCLKSDIO",
	"PCLKSDMMC1",
	"PCLKI2C1",
	"PCLKI2C2",
	"PCLKUART1",
	"PCLKUART2",
	"PCLKUART3",
	"PCLKHSEM",
	"HCLKGAE",
	"HCLKVIP",
	"HCLKUSB",
	"PCLK_APB_REG",
	"PCLKSARADC",
	"PCLKMSP0",
	"PCLKMSP1",
	"PCLKMSP2",
	"PCLKCDSUBSYS",
	"HCLKGFX",
	"PCLKETH",
	"HCLKC3",
	"NA", /* Not used */
	"ACLKDDRC",
	"ACLKA7",
	"PCLKI2C0",
	"PCLKUART0",
	/* group1 (via SRC_A7) */
	"PCLKSDMMC2", /* = 32 */
	"TRACECLK_M3",
	"TRACECLK_A7",
	"ATCLK_A7",
	"ATCLK_DEBUG",
	"PCLKDDRCTRL",
	"ACLKGFX",
	"NA_39", /* Not used = [29, .., 63] */
	"NA_40",
	"NA_41",
	"NA_42",
	"NA_43",
	"NA_44",
	"NA_45",
	"NA_46",
	"NA_47",
	"NA_48",
	"NA_49",
	"NA_50",
	"NA_51",
	"NA_52",
	"NA_53",
	"NA_54",
	"NA_55",
	"NA_56",
	"NA_57",
	"NA_58",
	"NA_59",
	"NA_60",
	"NA_61",
	"NA_62",
	"NA_63",
	/* group2 (via SRC_M3) */
	"CLCDCLK", /* = 64 */
	"SSP0CLK",
	"UART0CLK",
	"SDI0CLK",
	"I2C0CLK",
	"I2C1CLK",
	"UART1CLK",
	"SQICLK",
	"AUDIOSSFS512",
	"AUDIOSSMCLK",
	"EMRCLK",
	"SSP2CLK",
	"UART2CLK",
	"SDI1CLK",
	"SSP1CLK",
	"I2C2CLK",
	"UART3CLK",
	"MSP0CLK",
	"MSP1CLK",
	"MSP2CLK",
	"A7TSCNTCLK",
	"A7DBGCLK",
	"SDI2CLK",
	"C3RNGCLK",
	"SAFMEMCLK",
	"DDRCLK",
	"GFXCLK2X",
	"VDOCLK",
	"NA_92",
	"A7CLKEXT2F",
	"TSENSECLK",
	"CANFDCLK",
	/* group3 (via SRC_M3) */
	"ETH0PTPCLK",
	"FDCANPCLK",
	"FRAYPECLK",
	"NA_99",
	"ETH1PTPCLK",
	"WLANCLK",
	"ECULTRACLK",
	"AESCLK",
	"SHACLK",
};

#define to_pll(_hw) container_of(_hw, struct sta_clk_pll, hw)
#define to_gate(_hw) container_of(_hw, struct sta_clk_gate, hw)
#define to_div(_hw) container_of(_hw, struct sta_clk_div, hw)
#define to_mux(_hw) container_of(_hw, struct sta_clk_mux, hw)

static int sta_common_clk_init(void);

static int sta_clk_pll_enable(struct clk_hw *hw)
{
	struct sta_clk_pll *pll = to_pll(hw);
	u32 val, stat;

	switch (pll->id) {
	case PLL1:
		regmap_write(a7ssc_regmap, A7SSC_PLLARMCTRL + A7SSC_SET,
			     A7SSC_PLLARMCTRL_ENABLE);

		regmap_read(a7ssc_regmap, A7SSC_PLLARMLOCKP, &val);
		/* spin until locked */
		while (!(val & A7SSC_PLLARMLOCKP_PLLCLKGOOD)) {
			regmap_read(a7ssc_regmap, A7SSC_PLLARMLOCKP, &val);
			cpu_relax();
		}
		return 0;
	case PLL2:
		regmap_update_bits(srcm3_regmap, SRCM3_PLLCTRL,
				   SRCM3_PLLCTRL_PLL2EN,
				   SRCM3_PLLCTRL_PLL2EN);
		stat = SRCM3_PLLCTRL_PLL2STAT;
		break;
	case PLL3:
		regmap_update_bits(srcm3_regmap, SRCM3_PLLCTRL,
				   SRCM3_PLLCTRL_PLL3EN,
				   SRCM3_PLLCTRL_PLL3EN);
		stat = SRCM3_PLLCTRL_PLL3STAT;
		break;
	case PLL4:
		regmap_update_bits(srcm3_regmap, SRCM3_PLLCTRL,
				   SRCM3_PLLCTRL_PLL4EN,
				   SRCM3_PLLCTRL_PLL4EN);
		stat = SRCM3_PLLCTRL_PLL4STAT;
		break;
	case PLLD:
		regmap_update_bits(srcm3_regmap, SRCM3_PLLCTRL,
				   SRCM3_PLLCTRL_PLLDEN,
				   SRCM3_PLLCTRL_PLLDEN);
		stat = SRCM3_PLLCTRL_PLLDSTAT;
		break;
	default:
		pr_err("%s: invalid id:%i\n", __func__, pll->id);
		return -ENODEV;
	}

	regmap_read(srcm3_regmap, SRCM3_PLLCTRL, &val);
	/* spin until locked */
	while (!(val & stat)) {
		regmap_read(srcm3_regmap, SRCM3_PLLCTRL, &val);
		cpu_relax();
	}
	return 0;
}

/*
static void sta_clk_pll_disable(struct clk_hw *hw)
{
	struct sta_clk_pll *pll = to_pll(hw);

	switch (pll->id) {
	case PLL1:
		regmap_write(a7ssc_regmap, A7SSC_PLLARMCTRL + A7SSC_CLEAR,
			     A7SSC_PLLARMCTRL_ENABLE);
		return;
	case PLL2:
		regmap_update_bits(srcm3_regmap, SRCM3_PLLCTRL,
				   SRCM3_PLLCTRL_PLL2EN,
				   0x0);
		break;
	case PLL3:
		regmap_update_bits(srcm3_regmap, SRCM3_PLLCTRL,
				   SRCM3_PLLCTRL_PLL3EN,
				   0x0);
		break;
	case PLL4:
		regmap_update_bits(srcm3_regmap, SRCM3_PLLCTRL,
				   SRCM3_PLLCTRL_PLL4EN,
				   0x0);
		break;
	case PLLD:
		regmap_update_bits(srcm3_regmap, SRCM3_PLLCTRL,
				   SRCM3_PLLCTRL_PLLDEN,
				   0x0);
		break;
	default:
		pr_err("%s: invalid id:%i\n", __func__, pll->id);
		return;
	}
}
*/

static int sta_clk_pll_is_enabled(struct clk_hw *hw)
{
	struct sta_clk_pll *pll = to_pll(hw);
	u32 val, stat;

	if (pll->id != PLL1)
		regmap_read(srcm3_regmap, SRCM3_PLLCTRL, &val);
	else
		regmap_read(a7ssc_regmap, A7SSC_PLLARMCTRL, &val);

	/* Check that pll is enabled and locked */
	switch (pll->id) {
	case PLL1:
		regmap_read(a7ssc_regmap, A7SSC_PLLARMLOCKP, &stat);
		return !!(val & A7SSC_PLLARMCTRL_ENABLE) &&
			!!(stat & A7SSC_PLLARMLOCKP_PLLCLKGOOD);
	case PLL2:
		if (val & SRCM3_PLLCTRL_PLL2OVER)
			return !!(val & SRCM3_PLLCTRL_PLL2EN) &&
				!!(val & SRCM3_PLLCTRL_PLL2STAT);
		else
			return !!(val & SRCM3_PLLCTRL_PLL2STAT);
	case PLL3:
		return !!(val & SRCM3_PLLCTRL_PLL3EN) &&
			!!(val & SRCM3_PLLCTRL_PLL3STAT);
	case PLL4:
		return !!(val & SRCM3_PLLCTRL_PLL4EN) &&
			!!(val & SRCM3_PLLCTRL_PLL4STAT);
	case PLLD:
		return !!(val & SRCM3_PLLCTRL_PLLDEN) &&
			!!(val & SRCM3_PLLCTRL_PLLDSTAT);
	default:
		pr_err("%s: invalid id:%i\n", __func__, pll->id);
	}
	return 0;
}

static void clk_pll_get_settings(struct sta_clk_pll *pll)
{
	u32 val;

	switch (pll->id) {
	case PLL1:
		regmap_read(a7ssc_regmap, A7SSC_PLLARMFREQ, &val);
		pll->idf = (val & A7SSC_PLLARMFREQ_IDF) >>
			(ffs(A7SSC_PLLARMFREQ_IDF) - 1);
		pll->ndiv = (val & A7SSC_PLLARMFREQ_NDIV) >>
			(ffs(A7SSC_PLLARMFREQ_NDIV) - 1);
		/* PLL1 has fractional input */
		regmap_read(a7ssc_regmap, A7SSC_PLLARMCTRL, &val);
		pll->fract = val & A7SSC_PLLARMCTRL_FRACINPUT;
		break;
	case PLL2:
		regmap_read(srcm3_regmap, SRCM3_PLL2FCTRL, &val);
		pll->idf = (val & SRCM3_PLL2FCTRL_IDF) >>
			(ffs(SRCM3_PLL2FCTRL_IDF) - 1);
		pll->ndiv = (val & SRCM3_PLL2FCTRL_NDIV) >>
			(ffs(SRCM3_PLL2FCTRL_NDIV) - 1);
		break;
	case PLL3:
		regmap_read(srcm3_regmap, SRCM3_PLL3FCTRL, &val);
		pll->idf = (val & SRCM3_PLL3FCTRL_IDF) >>
			(ffs(SRCM3_PLL3FCTRL_IDF) - 1);
		pll->ndiv = (val & SRCM3_PLL3FCTRL_NDIV) >>
			(ffs(SRCM3_PLL3FCTRL_NDIV) - 1);
		/* PLL3 has fractional input */
		regmap_read(srcm3_regmap, SRCM3_PLL3FCTRL2, &val);
		pll->fract = val & SRCM3_PLL3FCTRL2_FRC_INPUT;
		regmap_read(srca7_regmap, SRCA7_PLL3FCTRL, &val);
		pll->frac_ctrl = !!(val & SRCA7_PLL3FCTRL_FRAC_CTRL);
		pll->dither = !(val & SRCA7_PLL3FCTRL_DITHER_DIS);
		break;
	case PLL4:
		regmap_read(srcm3_regmap, SRCM3_PLL4FCTRL, &val);
		pll->idf = (val & SRCM3_PLL4FCTRL_IDF) >>
			(ffs(SRCM3_PLL4FCTRL_IDF) - 1);
		pll->ndiv = (val & SRCM3_PLL4FCTRL_NDIV) >>
			(ffs(SRCM3_PLL4FCTRL_NDIV) - 1);
		break;
	case PLLD:
		regmap_read(srcm3_regmap, SRCM3_PLLDFCTRL, &val);
		/* PLL that feed the DDR has idf forced to 1 */
		pll->idf = 1;
		pll->ndiv = (val & SRCM3_PLLDFCTRL_NDIV) >>
			(ffs(SRCM3_PLLDFCTRL_NDIV) - 1);
		/* PLLD has fractional input */
		regmap_read(srcm3_regmap, SRCM3_PLLDFCTRL2, &val);
		pll->fract = val & SRCM3_PLL3FCTRL2_FRC_INPUT;
		break;
	default:
		pr_err("%s: invalid id:%i\n", __func__, pll->id);
	}
}

static u32 _get_pll3_fvco(struct sta_clk_pll *pll, u16 fract, u32 parent_rate)
{
	u64 fvco;
	u64 fvcofract;

	fvco = (u64)2 * pll->ndiv * parent_rate;
	fvco =  DIV_ROUND_CLOSEST_ULL(fvco, pll->idf);

	if (!pll->frac_ctrl)
		return (u32)fvco;

	if (pll->dither)
		fvco += (parent_rate / pll->idf) >> 16;

	fvcofract = ((u64)fract * parent_rate) >> 15;
	fvcofract = DIV_ROUND_CLOSEST_ULL(fvcofract, pll->idf);

	return (u32)(fvco + fvcofract);
}

static u16 _get_pll3_fract(struct sta_clk_pll *pll, u32 fvcofract,
			   u32 parent_rate)
{
	u64 fract64;
	u16 fract;

	if (!pll->frac_ctrl)
		return 0;

	fract64 = ((u64)fvcofract * pll->idf) << 15;
	fract64 = DIV_ROUND_CLOSEST_ULL(fract64, parent_rate);
	fract = (u16)min_t(u64, fract64, U16_MAX);

	return fract;
}

static unsigned long sta_clk_pll_recalc_rate(struct clk_hw *hw,
					     unsigned long parent_rate)
{
	struct sta_clk_pll *pll = to_pll(hw);
	u32 fvco;
	u64 fvcofract = 0;

	clk_pll_get_settings(pll);
	pll->idf = (pll->idf == 0) ? 1 : pll->idf;

	if (pll->id == PLL1 || pll->id == PLLD) {
		fvcofract = ((u64)pll->fract * parent_rate) >> 15;
		fvcofract = DIV_ROUND_CLOSEST_ULL(fvcofract, pll->idf);
		fvco = parent_rate * pll->ndiv / pll->idf;
		fvco = fvco * 2;
		fvco += fvcofract;
	} else if (pll->id == PLL3) {
		/* pll3 fvco calculation may include dithering, if enabled */
		fvco = _get_pll3_fvco(pll, pll->fract, parent_rate);
	} else {
		fvco = parent_rate * pll->ndiv / pll->idf;
		fvco = fvco * 2;
	}

	pr_debug("%s: id=%i, idf=%u, ndiv=%u, fract=%u, fvco=%u\n", __func__,
		 pll->id, pll->idf, pll->ndiv, pll->fract, fvco);

	/* return current vco frequency */
	return fvco;
}

static int sta_clk_pll_set_rate(struct clk_hw *hw, unsigned long rate,
				unsigned long parent_rate)
{
	struct sta_clk_pll *pll = to_pll(hw);
	u32 fvco;
	u16 fract;

	switch (pll->id) {
	case PLL1:
	case PLL2:
	case PLL4:
	case PLLD:
		pr_debug("%s: changing rate of pll%i is not allowed\n",
			 __func__, pll->id);
		break;
	case PLL3:
		/**
		 * The way we program the PLL3 is particular.
		 * More in details, to match audio requirements,
		 * we only update the fractional input and we always keep
		 * ndiv, idf (and odf) settings untouched.
		 */
		if (!pll->frac_ctrl)
			return 0;

		fvco = _get_pll3_fvco(pll, 0, parent_rate);
		fract = _get_pll3_fract(pll, rate - fvco, parent_rate);

		regmap_update_bits(srcm3_regmap, SRCM3_PLL3FCTRL2,
				   SRCM3_PLL3FCTRL2_FRC_INPUT,
				   fract);

		/* trigger strobe bit to make changes applied */
		regmap_update_bits(srca7_regmap, SRCA7_PLL3FCTRL,
				   SRCA7_PLL3FCTRL_STRB,
				   SRCA7_PLL3FCTRL_STRB);
		regmap_update_bits(srca7_regmap, SRCA7_PLL3FCTRL,
				   SRCA7_PLL3FCTRL_STRB,
				   0x0);
		break;
	}
	return 0;
}

static long sta_clk_pll_round_rate(struct clk_hw *hw, unsigned long rate,
				   unsigned long *prate)
{
	struct sta_clk_pll *pll = to_pll(hw);
	u32 fvco;
	u16 fract;

	switch (pll->id) {
	case PLL3:
		/**
		 * calculates the closest PLL3 rate which can be set
		 * only updating the fractional input (see clk_pll_set_rate)
		 */
		fvco = _get_pll3_fvco(pll, 0, (*prate));

		if (pll->frac_ctrl && rate > fvco) {
			fract = _get_pll3_fract(pll, rate - fvco, (*prate));
			fvco = _get_pll3_fvco(pll, fract, (*prate));
		}
		return fvco;
	default:
		/* changing rate is not allowed */
		return sta_clk_pll_recalc_rate(hw, *prate);
	}
}

static const struct clk_ops sta_clk_pll_ops = {
	.enable = sta_clk_pll_enable,
	.disable = NULL,
	.is_enabled = sta_clk_pll_is_enabled,
	.recalc_rate = sta_clk_pll_recalc_rate,
	.set_rate = sta_clk_pll_set_rate,
	.round_rate = sta_clk_pll_round_rate,
};

static inline void sta_clk_gate_spin_until_disabled(struct regmap *regmap,
						    u32 reg, u32 clkbit)
{
	u32 val;

	regmap_read(regmap, reg, &val);
	while (val & clkbit) {
		regmap_read(regmap, reg, &val);
		cpu_relax();
	}
}

static inline void sta_clk_gate_spin_until_enabled(struct regmap *regmap,
						   u32 reg, u32 clkbit)
{
	u32 val;

	regmap_read(regmap, reg, &val);
	while (!(val & clkbit)) {
		regmap_read(regmap, reg, &val);
		cpu_relax();
	}
}

/*
 * The STA SoC SRC clocks are gated, but not in the sense that
 * you read-modify-write a register. Instead there are separate
 * clock enable and clock disable registers. Writing a '1' bit in
 * the enable register for a certain clock ungates that clock without
 * affecting the other clocks. The disable register works the opposite
 * way.
 */
static int sta_clk_gate_enable(struct clk_hw *hw)
{
	struct sta_clk_gate *gate = to_gate(hw);
	struct regmap *regmap;
	u32 enreg, sreg;

	pr_debug("%s: %s\n", __func__,
		 clk_gate_names[(gate->group * 32) + ffs(gate->clkbit) - 1]);

	switch (gate->group) {
	case 0:
		regmap = srca7_regmap;
		enreg = SRCA7_PCKEN0;
		sreg = SRCA7_PCKSR0;
		break;
	case 1:
		regmap = srca7_regmap;
		enreg = SRCA7_PCKEN1;
		sreg = SRCA7_PCKSR1;
		break;
	case 2:
		regmap = srcm3_regmap;
		enreg = SRCM3_PCKEN1;
		sreg = SRCM3_PCKSR1;
		break;
	case 3:
		regmap = srcm3_regmap;
		enreg = SRCM3_PCKEN2;
		sreg = SRCM3_PCKSR2;
		break;
	default:
		pr_err("%s: invalid group: %d\n", __func__, gate->group);
		return -EFAULT;
	}

	regmap_write(regmap, enreg, gate->clkbit);
	sta_clk_gate_spin_until_enabled(regmap, sreg, gate->clkbit);
	return 0;
}

static void sta_clk_gate_disable(struct clk_hw *hw)
{
	struct sta_clk_gate *gate = to_gate(hw);
	struct regmap *regmap;
	u32 disreg, sreg;

	pr_debug("%s: %s\n", __func__,
		 clk_gate_names[(gate->group * 32) + ffs(gate->clkbit) - 1]);

	switch (gate->group) {
	case 0:
		regmap = srca7_regmap;
		disreg = SRCA7_PCKDIS0;
		sreg = SRCA7_PCKSR0;
		break;
	case 1:
		regmap = srca7_regmap;
		disreg = SRCA7_PCKDIS1;
		sreg = SRCA7_PCKSR1;
		break;
	case 2:
		regmap = srcm3_regmap;
		disreg = SRCM3_PCKDIS1;
		sreg = SRCM3_PCKSR1;
		break;
	case 3:
		regmap = srcm3_regmap;
		disreg = SRCM3_PCKDIS2;
		sreg = SRCM3_PCKSR2;
		break;
	default:
		pr_err("%s: invalid group: %d\n", __func__, gate->group);
		return;
	}

	regmap_write(regmap, disreg, gate->clkbit);
	sta_clk_gate_spin_until_disabled(regmap, sreg, gate->clkbit);
}

static int sta_clk_gate_is_enabled(struct clk_hw *hw)
{
	struct sta_clk_gate *gate = to_gate(hw);
	u32 val;

	pr_debug("%s: %s\n", __func__,
		 clk_gate_names[(gate->group * 32) + ffs(gate->clkbit) - 1]);

	switch (gate->group) {
	case 0:
		regmap_read(srca7_regmap, SRCA7_PCKSR0, &val);
		break;
	case 1:
		regmap_read(srca7_regmap, SRCA7_PCKSR1, &val);
		break;
	case 2:
		regmap_read(srcm3_regmap, SRCM3_PCKSR1, &val);
		break;
	case 3:
		regmap_read(srcm3_regmap, SRCM3_PCKSR2, &val);
		break;
	default:
		pr_err("%s: invalid group: %d\n", __func__, gate->group);
		return 0;
	}
	return !!(val & gate->clkbit);
}

static unsigned long sta_clk_gate_recalc_rate(struct clk_hw *hw,
					      unsigned long parent_rate)
{
	struct sta_clk_gate *gate = to_gate(hw);

	pr_debug("%s: %s = %luHz\n", __func__,
		 clk_gate_names[(gate->group * 32) + ffs(gate->clkbit) - 1],
		 parent_rate);
	return parent_rate;
}

static const struct clk_ops sta_clk_gate_dummy_ops = {
	.enable = NULL,
	.disable = NULL,
	.is_enabled = sta_clk_gate_is_enabled,
	.recalc_rate = sta_clk_gate_recalc_rate,
};

static const struct clk_ops sta_clk_gate_ops = {
	.enable = sta_clk_gate_enable,
	.disable = sta_clk_gate_disable,
	.is_enabled = sta_clk_gate_is_enabled,
	.recalc_rate = sta_clk_gate_recalc_rate,
};

#ifdef CONFIG_DEBUG_FS

static u32 src_a7_pcksr0_boot;
static u32 src_a7_pcksr1_boot;
static u32 src_m3_pcksr1_boot;
static u32 src_m3_pcksr2_boot;

static int sta_clk_gate_show(struct seq_file *s, void *what)
{
	int i, array_size;
	u32 mask, pcksrb, pcksr, pckreq,
		src_a7_pcksr0, src_a7_pckensr0,
		src_a7_pcksr1, src_a7_pckensr1,
		src_m3_pcksr1 = 0, src_m3_pckensr1 = 0,
		src_m3_pcksr2 = 0, src_m3_pckensr2 = 0;

	regmap_read(srca7_regmap, SRCA7_PCKSR0, &src_a7_pcksr0);
	regmap_read(srca7_regmap, SRCA7_PCKENSR0, &src_a7_pckensr0);
	regmap_read(srca7_regmap, SRCA7_PCKSR1, &src_a7_pcksr1);
	regmap_read(srca7_regmap, SRCA7_PCKENSR1, &src_a7_pckensr1);

	if (is_sta1385_compatible)
		array_size = STA1385_NUM_GATES;
	else
		array_size = STA1295_NUM_GATES;

	regmap_read(srcm3_regmap, SRCM3_PCKSR1, &src_m3_pcksr1);
	regmap_read(srcm3_regmap, SRCM3_PCKENSR1, &src_m3_pckensr1);
	regmap_read(srcm3_regmap, SRCM3_PCKSR2, &src_m3_pcksr2);
	regmap_read(srcm3_regmap, SRCM3_PCKENSR2, &src_m3_pckensr2);

	seq_puts(s, "Clock:      Boot:   Now:    Request: ASKED:\n");
	for (i = 0; i < array_size; i++) {
		if (i < 32) {
			pcksrb = src_a7_pcksr0_boot;
			pcksr = src_a7_pcksr0;
			pckreq = src_a7_pckensr0;
		} else if (i < 64) {
			pcksrb = src_a7_pcksr1_boot;
			pcksr = src_a7_pcksr1;
			pckreq = src_a7_pckensr1;
		} else if (i < 96) {
			pcksrb = src_m3_pcksr1_boot;
			pcksr = src_m3_pcksr1;
			pckreq = src_m3_pckensr1;
		} else {
			pcksrb = src_m3_pcksr2_boot;
			pcksr = src_m3_pcksr2;
			pckreq = src_m3_pckensr2;
		}
		mask = BIT(i & 0x1f);
		seq_printf(s, "%12s  %s     %s     %s\n",
			   clk_gate_names[i],
			   (pcksrb & mask) ? "on " : "off",
			   (pcksr & mask) ? "on " : "off",
			   (pckreq & mask) ? "on " : "off");
	}
	return 0;
}

static int sta_clk_gate_open(struct inode *inode, struct file *file)
{
	return single_open(file, sta_clk_gate_show, NULL);
}

static const struct file_operations sta_clk_gate_debugfs_ops = {
	.open           = sta_clk_gate_open,
	.read           = seq_read,
	.llseek         = seq_lseek,
	.release        = single_release,
};

static int __init sta_clk_gate_init_debugfs(void)
{
	regmap_read(srca7_regmap, SRCA7_PCKSR0, &src_a7_pcksr0_boot);
	regmap_read(srca7_regmap, SRCA7_PCKSR1, &src_a7_pcksr1_boot);
	regmap_read(srcm3_regmap, SRCM3_PCKSR1, &src_m3_pcksr1_boot);
	regmap_read(srcm3_regmap, SRCM3_PCKSR2, &src_m3_pcksr2_boot);
	debugfs_create_file("sta-src-clk", S_IFREG | S_IRUGO,
			    NULL, NULL, &sta_clk_gate_debugfs_ops);
	return 0;
}

module_init(sta_clk_gate_init_debugfs);

#endif

#define width_to_mask(width)	((1 << (width)) - 1)

static unsigned int _get_table_div(const struct clk_div_table *table,
				   unsigned int val)
{
	const struct clk_div_table *clkt;

	for (clkt = table; clkt->div; clkt++)
		if (clkt->val == val)
			return clkt->div;
	return 0;
}

static unsigned int _get_div(const struct clk_div_table *table,
			     unsigned int val, unsigned long flags, u8 width)
{
	if (flags & CLK_DIVIDER_ONE_BASED)
		return val;
	if (table)
		return _get_table_div(table, val);
	return val + 1;
}

static unsigned long sta_clk_divider_recalc_rate(struct clk_hw *hw,
						 unsigned long parent_rate)
{
	struct sta_clk_div *divider = to_div(hw);
	unsigned int val;

	regmap_read(divider->regmap, divider->reg, &val);

	val >>= divider->shift;
	val &= width_to_mask(divider->width);

	return divider_recalc_rate(hw, parent_rate, val, divider->table,
				   divider->flags);
}

static long sta_clk_divider_round_rate(struct clk_hw *hw, unsigned long rate,
				       unsigned long *prate)
{
	struct sta_clk_div *divider = to_div(hw);
	unsigned int bestdiv;

	/* if read only, just return current value */
	if (divider->flags & CLK_DIVIDER_READ_ONLY) {
		regmap_read(divider->regmap, divider->reg, &bestdiv);
		bestdiv >>= divider->shift;
		bestdiv &= width_to_mask(divider->width);
		bestdiv = _get_div(divider->table, bestdiv, divider->flags,
				   divider->width);
		return DIV_ROUND_UP(*prate, bestdiv);
	}

	return divider_round_rate(hw, rate, prate, divider->table,
				  divider->width, divider->flags);
}

static int sta_clk_divider_set_rate(struct clk_hw *hw, unsigned long rate,
				    unsigned long parent_rate)
{
	struct sta_clk_div *divider = to_div(hw);
	unsigned int value;

	value = divider_get_val(rate, parent_rate, divider->table,
				divider->width, divider->flags);

	regmap_update_bits(divider->regmap, divider->reg,
			   width_to_mask(divider->width) << divider->shift,
			   value << divider->shift);
	if (divider->strobe_bit != -1) {
		/* trigger strobe bit to make changes applied */
		regmap_update_bits(divider->regmap, divider->reg,
				   BIT(divider->strobe_bit),
				   BIT(divider->strobe_bit));
		regmap_update_bits(divider->regmap, divider->reg,
				   BIT(divider->strobe_bit),
				   0);
	}
	return 0;
}

static const struct clk_ops sta_clk_divider_ops = {
	.recalc_rate = sta_clk_divider_recalc_rate,
	.round_rate = sta_clk_divider_round_rate,
	.set_rate = sta_clk_divider_set_rate,
};

static u8 sta_clk_mux_get_parent(struct clk_hw *hw)
{
	struct sta_clk_mux *mux = to_mux(hw);
	u32 num_parents = clk_hw_get_num_parents(hw);
	u32 val;

	regmap_read(mux->regmap, mux->reg, &val);

	val >>= mux->shift;
	val &= width_to_mask(mux->width);

	if (mux->table) {
		u32 i;

		for (i = 0; i < num_parents; i++)
			if (mux->table[i] == val)
				return i;
		return -EINVAL;
	}

	if (val >= num_parents)
		return -EINVAL;

	return val;
}

static int sta_clk_mux_set_parent(struct clk_hw *hw, u8 index)
{
	struct sta_clk_mux *mux = to_mux(hw);

	if (mux->table)
		index = mux->table[index];

	return regmap_update_bits(mux->regmap, mux->reg,
				  width_to_mask(mux->width) << mux->shift,
				  index << mux->shift);
}

static const struct clk_ops sta_clk_mux_ro_ops = {
	.get_parent = sta_clk_mux_get_parent,
};

static const struct clk_ops sta_clk_mux_ops = {
	.get_parent = sta_clk_mux_get_parent,
	.set_parent = sta_clk_mux_set_parent,
	.determine_rate = __clk_mux_determine_rate,
};

static int __init pll3phi_clk_init(void)
{
	const struct clk_proto *clk_p = &clk_protos[STA_CLK_PLL3PHI];
	struct clk_hw *hw;
	u32 odf;

	/**
	 * PLL3 PHI is only used by the audio sub-system and in a special way.
	 * More in details, the "odf" divider factor is meant to be kept
	 * fixed. And a fine tuning of the audio ref clock is expected, relying
	 * on the parent clock of PLL3 PHI clock, namely the PLL3 FCVO clock,
	 * that support FRACTIONAL input.
	 *
	 * Also as for the other pll settings, we expect odf setting to be
	 * set before Linux startup by the early boot stages.
	 */
	regmap_read(srca7_regmap, SRCA7_PLL3FCTRL, &odf);
	odf &= SRCA7_PLL3FCTRL_ODF;
	pr_debug("%s: read odf=%i from register\n",
		 __func__, odf);

	if (odf < 2) {
		pr_err("%s: odf=%i, should be in range [2,15]\n",
		       __func__, odf);
		WARN_ON(odf < 2);
		return -EINVAL;
	}

	hw = clk_hw_register_fixed_factor(NULL, clk_p->name, clk_p->parents[0],
					  CLK_SET_RATE_PARENT | CLK_IS_BASIC,
					  1, odf);
	if (IS_ERR(hw)) {
		pr_err("%s: registering failed\n", __func__);
		clk_data.hws[STA_CLK_PLL3PHI] = ERR_PTR(-ENOENT);
		return IS_ERR(hw);
	}
	clk_data.hws[STA_CLK_PLL3PHI] = hw;
	return 0;
}

static int __init mxtal_clk_init(void)
{
	struct clk_hw *hw;
	u32 val, rate;

	regmap_read(srca7_regmap, SRCA7_RESSTAT, &val);

	if (is_sta1385_compatible) {
		val = (val & SRCA7_RESSTAT_MXTALFREQSEL_STA1385_MSK) >>
		      SRCA7_RESSTAT_MXTALFREQSEL_SHIFT;
		switch (val) {
		case 0:
			rate = 24000000;
			break;
		case 1:
			rate = 26000000;
			break;
		case 2:
			rate = 40000000;
			break;
		default:
			return -EINVAL;
		}
	} else {
		rate = (val & SRCA7_RESSTAT_MXTALFREQSEL ? 26000000 : 24000000);
	}

	hw = clk_hw_register_fixed_rate(NULL, "mxtal", NULL, 0, rate);
	if (IS_ERR(hw)) {
		pr_err("%s: registering failed\n", __func__);
		clk_data.hws[STA_CLK_MXTAL] = ERR_PTR(-ENOENT);
		return IS_ERR(hw);
	}
	clk_data.hws[STA_CLK_MXTAL] = hw;

	pr_info("MXTAL frequency = %u Hz\n", rate);
	return 0;
}

static int __init timer_clk_init(void)
{
	struct clk_hw **hws;
	struct clk_hw *hw;
	u32 val, div;

	regmap_read(srcm3_regmap, SRCM3_CR, &val);

	div = (val & SRCM3_CR_Timerxsel_MTU0 ? 8 : 1);
	hw = clk_hw_register_fixed_factor(NULL, "tim0clk", "mxtal",
					  CLK_IS_BASIC, 1, div);
	hws = clk_data.hws;
	if (IS_ERR(hw)) {
		pr_err("%s: registering failed\n", __func__);
		hws[STA_CLK_TIMER0] = ERR_PTR(-ENOENT);
		return IS_ERR(hw);
	}
	hws[STA_CLK_TIMER0] = hw;

	div = (val & SRCM3_CR_Timerxsel_MTU1 ? 8 : 1);
	hw = clk_hw_register_fixed_factor(NULL, "tim1clk", "mxtal",
					  CLK_IS_BASIC, 1, div);
	if (IS_ERR(hw)) {
		pr_err("%s: registering failed\n", __func__);
		hws[STA_CLK_TIMER1] = ERR_PTR(-ENOENT);
		return IS_ERR(hw);
	}
	hws[STA_CLK_TIMER1] = hw;
	return 0;
}

enum sta_clk_type {
	STA_CLK_FIXED,
	STA_CLK_FIXED_FACTOR,
	STA_CLK_PLL,
	STA_CLK_MUX,
	STA_CLK_DIV,
	STA_CLK_GATE,
};

struct sta_clk_hw_proto {
	const struct clk_ops *ops;
	union {
		struct sta_clk_pll pll;
		struct sta_clk_mux mux;
		struct sta_clk_div div;
		struct sta_clk_gate gate;
	};
};

struct clk_hw_proto {
	enum sta_clk_type type;
	union {
		struct clk_fixed_rate fr;
		struct clk_fixed_factor ff;
		struct sta_clk_hw_proto sta_hw;
	};
};

#define STA_DEFINE_FIXED(_idx, _rate)					\
[CLK_PREFIX(_idx)] = {							\
	.type = STA_CLK_FIXED,						\
	{								\
		.fr = {							\
			.fixed_rate = (_rate),				\
		},							\
	},								\
}

#define STA_DEFINE_FIXED_FACTOR(_idx, _mul, _div)			\
[CLK_PREFIX(_idx)] = {							\
	.type = STA_CLK_FIXED_FACTOR,					\
	{								\
		.ff = {							\
			.div = (_div),					\
			.mult = (_mul),					\
		},							\
	},								\
}

#define STA_DEFINE_PLL(_idx, _id)					\
[CLK_PREFIX(_idx)] = {							\
	.type = STA_CLK_PLL,						\
	{								\
		.sta_hw = {						\
			.ops = &sta_clk_pll_ops,			\
			{						\
				.pll = {				\
					.id = _id,			\
				},					\
			},						\
		},							\
	},								\
}

#define STA_DEFINE_MUX(_idx, _ctrl, _reg, _shift, _width, _table)	\
[CLK_PREFIX(_idx)] = {							\
	.type = STA_CLK_MUX,						\
	{								\
		.sta_hw = {						\
			.ops = &sta_clk_mux_ops,			\
			{						\
				.mux = {				\
					.ctrl = _ctrl,			\
					.reg = _reg,			\
					.width = (_width),		\
					.shift = (_shift),		\
					.table = (_table),		\
				},					\
			},						\
		},							\
	},								\
}

#define STA_DEFINE_DIV(_idx, _ctrl, _reg, _shift, _width, _table,	\
		       _flags, _strobe_bit)				\
[CLK_PREFIX(_idx)] = {							\
	.type = STA_CLK_DIV,						\
	{								\
		.sta_hw = {						\
			.ops = &sta_clk_divider_ops,			\
			{						\
				.div = {				\
					.ctrl = _ctrl,			\
					.reg = _reg,			\
					.shift = (_shift),		\
					.width = (_width),		\
					.table = (_table),		\
					.flags = (_flags),		\
					.strobe_bit = (_strobe_bit)	\
				 },					\
			},						\
		 },							\
	},								\
}

#define STA_DEFINE_GATE(_idx, _id)					\
[CLK_PREFIX(_idx)] = {							\
	.type = STA_CLK_GATE,						\
	{								\
		.sta_hw = {						\
			.ops = &sta_clk_gate_ops,			\
			{						\
				.gate = {				\
					.id = (_id),			\
				},					\
			},						\
		},							\
	},								\
}

#define PHIDIV_FLAGS (CLK_DIVIDER_ONE_BASED | CLK_DIVIDER_ALLOW_ZERO)

static struct clk_hw_proto clk_hw_protos[STA_CLK_MAX] = {
	STA_DEFINE_FIXED(RTCOSC, 32768),
	STA_DEFINE_FIXED(RINGOSC, 6000000),
	STA_DEFINE_FIXED_FACTOR(MXTALBY2, 1, 2),
	STA_DEFINE_MUX(CPUEXT, A7SSC, 0x0, 16, 1, NULL),
	STA_DEFINE_MUX(CPU, A7SSC, 0x0, 0, 1, NULL),
	STA_DEFINE_FIXED_FACTOR(TSENSEDIV, 1, 128),
	STA_DEFINE_FIXED_FACTOR(WDT, 1, 1),
	STA_DEFINE_FIXED_FACTOR(RNG, 1, 1),
	STA_DEFINE_FIXED_FACTOR(OTPCC, 1, 1),
	STA_DEFINE_FIXED_FACTOR(CAN0, 1, 1),
	STA_DEFINE_FIXED(PADPIX, 1000000),

	STA_DEFINE_PLL(PLL1FVCO, 1),
	STA_DEFINE_DIV(PLL1PHI, A7SSC, 0x80, 10, 6, NULL, PHIDIV_FLAGS, 24),
	STA_DEFINE_FIXED_FACTOR(PLL1FVCOBY2, 1, 2),
	STA_DEFINE_FIXED_FACTOR(PLL1FVCOBY3, 1, 3),

	STA_DEFINE_PLL(PLL2FVCO, 2),
	STA_DEFINE_DIV(PLL2PHI, SRCM3, 0x18, 0, 4, NULL, PHIDIV_FLAGS, 7),
	STA_DEFINE_FIXED_FACTOR(PLL2FVCOBY2, 1, 2),
	STA_DEFINE_FIXED_FACTOR(PLL2FVCOBY3, 1, 3),
	STA_DEFINE_FIXED_FACTOR(PLL2FVCOBY2BY3, 1, 3),
	STA_DEFINE_FIXED_FACTOR(PLL2FVCOBY2BY4, 1, 4),
	STA_DEFINE_FIXED_FACTOR(PLL2FVCOBY2BY5, 1, 5),
	STA_DEFINE_FIXED_FACTOR(PLL2FVCOBY2BY6, 1, 6),
	STA_DEFINE_FIXED_FACTOR(PLL2FVCOBY2BY7, 1, 7),
	STA_DEFINE_FIXED_FACTOR(PLL2FVCOBY2BY12, 1, 12),
	STA_DEFINE_FIXED_FACTOR(PLL2FVCOBY2BY25, 1, 25),
	STA_DEFINE_FIXED_FACTOR(PLL2FVCOBY2BY26, 1, 26),

	STA_DEFINE_PLL(PLL3FVCO, 3),

	STA_DEFINE_PLL(PLL4FVCO, 4),
	STA_DEFINE_DIV(PLL4PHI, SRCM3, 0x54, 0, 4, NULL, PHIDIV_FLAGS, 7),
	STA_DEFINE_FIXED_FACTOR(PLL4FVCOBY2, 1, 2),
	STA_DEFINE_FIXED_FACTOR(PLL4FVCOBY2BY2, 1, 2),
	STA_DEFINE_FIXED_FACTOR(PLL4FVCOBY2BY3, 1, 3),
	STA_DEFINE_FIXED_FACTOR(PLL4FVCOBY2BY4, 1, 4),
	STA_DEFINE_FIXED_FACTOR(PLL4FVCOBY2BY12, 1, 12),
	STA_DEFINE_FIXED_FACTOR(PLL4FVCOBY3, 1, 3),
	STA_DEFINE_FIXED_FACTOR(PLL4FVCOBY3BY6, 1, 6),

	STA_DEFINE_PLL(PLLDFVCO, 5),
	STA_DEFINE_DIV(PLLDPHI, SRCA7, 0x3c, 0, 3, NULL, PHIDIV_FLAGS, 7),

	STA_DEFINE_FIXED(PLLUFVCO, 2880000000UL),
	STA_DEFINE_FIXED_FACTOR(PLLUPHI, 1, 6),
	STA_DEFINE_FIXED_FACTOR(PLLUPHIBY6, 1, 6),
	STA_DEFINE_FIXED_FACTOR(PLLUPHIBY12, 1, 12),

	STA_DEFINE_DIV(HCLKDIV, SRCA7, 0x20, 0, 3, NULL, 0x0, STROBE_NA),

	STA_DEFINE_MUX(ETMSEL, SRCA7, 0x20, 7, 2, NULL),
	STA_DEFINE_MUX(GFXSEL, SRCM3, 0x44, 17, 1, NULL),
	STA_DEFINE_MUX(VDOSEL, SRCM3, 0x44, 18, 1, NULL),
	STA_DEFINE_MUX(SDIOSEL, SRCM3, 0x44, 3, 2, NULL),
	STA_DEFINE_MUX(SDMMC1SEL, SRCM3, 0x44, 8, 2, NULL),
	STA_DEFINE_MUX(SDMMC2SEL, SRCM3, 0x44, 19, 2, NULL),
	STA_DEFINE_MUX(SQIOSEL, SRCM3, 0x44, 1, 2, NULL),
	STA_DEFINE_MUX(AUDIOSSMSEL, SRCM3, 0x44, 5, 1, NULL),
	STA_DEFINE_MUX(AUDIOSSFS512SEL, SRCM3, 0x44, 6, 1, NULL),
	STA_DEFINE_MUX(CLCDPLLSEL, SRCM3, 0x44, 22, 1, NULL),
	STA_DEFINE_DIV(CLCDDIV, SRCM3, 0x44, 11, 6, NULL, 0x0, STROBE_NA),
	STA_DEFINE_MUX(CLCDSEL, SRCM3, 0x44, 0, 1, NULL),
	STA_DEFINE_MUX(CANSSSEL, SRCM3, 0x44, 7, 1, NULL),

	STA_DEFINE_DIV(UART45DIV, A7MSCR, 0x10C, 4, 4, NULL, PHIDIV_FLAGS,
		       STROBE_NA),
	STA_DEFINE_MUX(UART45SEL, A7MSCR, 0x10C, 28, 2, NULL),

	/* SRCA7 gates group 0 <=> PCKEN0 */
	STA_DEFINE_GATE(HDMA0, 0),
	STA_DEFINE_GATE(HDMA1, 1),
	STA_DEFINE_GATE(HFSMC, 2),
	STA_DEFINE_GATE(PSSP0, 3),
	STA_DEFINE_GATE(HCLCD, 4),
	STA_DEFINE_GATE(PSSP1, 5),
	STA_DEFINE_GATE(PSSP2, 6),
	STA_DEFINE_GATE(PSDIO, 7),
	STA_DEFINE_GATE(PSDMMC1, 8),
	STA_DEFINE_GATE(PI2C1, 9),
	STA_DEFINE_GATE(PI2C2, 10),
	STA_DEFINE_GATE(PUART1, 11),
	STA_DEFINE_GATE(PUART2, 12),
	STA_DEFINE_GATE(PUART3, 13),
	STA_DEFINE_GATE(PHSEM, 14),
	STA_DEFINE_GATE(HGAE, 15),
	STA_DEFINE_GATE(HVIP, 16),
	STA_DEFINE_GATE(HUSB, 17),
	STA_DEFINE_GATE(PAPBREG, 18),
	STA_DEFINE_GATE(PSARADC, 19),
	STA_DEFINE_GATE(PMSP0, 20),
	STA_DEFINE_GATE(PMSP1, 21),
	STA_DEFINE_GATE(PMSP2, 22),
	STA_DEFINE_GATE(PCDSUBSYS, 23),
	STA_DEFINE_GATE(HGFX, 24),
	STA_DEFINE_GATE(AETH, 25),
	STA_DEFINE_GATE(HC3, 26),
	/* hole */
	STA_DEFINE_GATE(ADDRC, 28),
	STA_DEFINE_GATE(AA7, 29),
	STA_DEFINE_GATE(PI2C0, 30),
	STA_DEFINE_GATE(PUART0, 31),
	/* SRCA7 gates group 1 : <=> PCKEN1 */
	STA_DEFINE_GATE(PSDMMC2, 32 + 0),
	STA_DEFINE_GATE(TRACEM3, 32 + 1),
	STA_DEFINE_GATE(TRACEA7, 32 + 2),
	STA_DEFINE_GATE(ATA7, 32 + 3),
	STA_DEFINE_GATE(ATDEBUG, 32 + 4),
	STA_DEFINE_GATE(PDDRCTRL, 32 + 5),
	STA_DEFINE_GATE(AGFX, 32 + 6),
	/* SRCM3 gates group 0 : <=> PCKEN0 */
	STA_DEFINE_GATE(CLCD, 64 + 0),
	STA_DEFINE_GATE(SSP0, 64 + 1),
	STA_DEFINE_GATE(UART0, 64 + 2),
	STA_DEFINE_GATE(SDIO, 64 + 3),
	STA_DEFINE_GATE(I2C0, 64 + 4),
	STA_DEFINE_GATE(I2C1, 64 + 5),
	STA_DEFINE_GATE(UART1, 64 + 6),
	STA_DEFINE_GATE(SQIO, 64 + 7),
	STA_DEFINE_GATE(AUDIOSSFS512, 64 + 8),
	STA_DEFINE_GATE(AUDIOSSM, 64 + 9),
	STA_DEFINE_GATE(EMR, 64 + 10),
	STA_DEFINE_GATE(SSP2, 64 + 11),
	STA_DEFINE_GATE(UART2, 64 + 12),
	STA_DEFINE_GATE(SDMMC1, 64 + 13),
	STA_DEFINE_GATE(SSP1, 64 + 14),
	STA_DEFINE_GATE(I2C2, 64 + 15),
	STA_DEFINE_GATE(UART3, 64 + 16),
	STA_DEFINE_GATE(MSP0, 64 + 17),
	STA_DEFINE_GATE(MSP1, 64 + 18),
	STA_DEFINE_GATE(MSP2, 64 + 19),
	STA_DEFINE_GATE(A7CNT, 64 + 20),
	STA_DEFINE_GATE(A7DBG, 64 + 21),
	STA_DEFINE_GATE(SDMMC2, 64 + 22),
	STA_DEFINE_GATE(C3RNG, 64 + 23),
	STA_DEFINE_GATE(SAFMEM, 64 + 24),
	STA_DEFINE_GATE(DDR, 64 + 25),
	STA_DEFINE_GATE(GFX2X, 64 + 26),
	STA_DEFINE_GATE(VDO, 64 + 27),
	STA_DEFINE_GATE(A7P, 64 + 28),
	STA_DEFINE_GATE(A7EXT2F, 64 + 29),
	STA_DEFINE_GATE(TSENSE, 64 + 30),
	STA_DEFINE_GATE(CANFD, 64 + 31),
	/* SRCM3 gates group 1 : <=> PCKEN2 */
	STA_DEFINE_GATE(ETH0PTP, 96 + 0),
	STA_DEFINE_GATE(FDCANP, 96 + 1),
	STA_DEFINE_GATE(FRAYPE, 96 + 2),
	/* hole */
	STA_DEFINE_GATE(ETH1PTP, 96 + 4),
	STA_DEFINE_GATE(WLAN, 96 + 5),
	STA_DEFINE_GATE(ECULTRA, 96 + 6),
	STA_DEFINE_GATE(AES, 96 + 7),
	STA_DEFINE_GATE(SHA, 96 + 8),
};

static struct clk_hw * __init sta_clk_hw_register(u32 id)
{
	const struct clk_proto *clk_p = &clk_protos[id];
	struct clk_hw_proto *clk_hw_p = &clk_hw_protos[id];
	struct clk_hw *hw;
	int ret;

	pr_debug("%s: derived from '%s', clock type %d\n", clk_p->name,
		 clk_p->parents[0], clk_hw_p->type);

	switch (clk_hw_p->type) {
	case STA_CLK_PLL:
	case STA_CLK_MUX:
	case STA_CLK_DIV:
	case STA_CLK_GATE:
	{
		struct clk_init_data clk_init = {
			.name = clk_p->name,
			.parent_names = clk_p->parents,
			.num_parents = clk_p->num_parents,
			.flags = clk_p->flags,
			.ops = clk_hw_p->sta_hw.ops,
		};

		/* quirk for sta1385-cut1, where fdcan input clock is
		 * plluphiby12 instead of plluphiby6.
		 */
		if (id == STA_CLK_CANFD && quirk_canfd_old_cuts) {
			clk_init.parent_names = old_cut_canfd_parents;
			clk_init.num_parents = 1;
		}

		if (clk_hw_p->type == STA_CLK_PLL) {
			struct sta_clk_pll *pll = &clk_hw_p->sta_hw.pll;

			clk_pll_get_settings(pll);
			hw = &clk_hw_p->sta_hw.pll.hw;
		} else if (clk_hw_p->type == STA_CLK_MUX) {
			struct sta_clk_mux *mux = &clk_hw_p->sta_hw.mux;

			if (mux->ctrl == SRCM3)
				mux->regmap = srcm3_regmap;
			else if (mux->ctrl == SRCA7)
				mux->regmap = srca7_regmap;
			else if (mux->ctrl == A7SSC)
				mux->regmap = a7ssc_regmap;
			else if (mux->ctrl == A7MSCR)
				mux->regmap = a7mscr_regmap;
			hw = &clk_hw_p->sta_hw.mux.hw;
		} else if (clk_hw_p->type == STA_CLK_DIV) {
			struct sta_clk_div *div = &clk_hw_p->sta_hw.div;

			if (div->ctrl == SRCM3)
				div->regmap = srcm3_regmap;
			else if (div->ctrl == SRCA7)
				div->regmap = srca7_regmap;
			else if (div->ctrl == A7SSC)
				div->regmap = a7ssc_regmap;
			else if (div->ctrl == A7MSCR)
				div->regmap = a7mscr_regmap;
			hw = &clk_hw_p->sta_hw.div.hw;
		} else if (clk_hw_p->type == STA_CLK_GATE) {
			struct sta_clk_gate *gate = &clk_hw_p->sta_hw.gate;

			gate->group = gate->id / 32;
			gate->clkbit = BIT(gate->id & 0x1f);

			/*
			 * Some gates cannot be enabled or disabled:
			 * - buggy clocks in STA1295 CUT1:
			 *   - PCLKUART0	(31)
			 *   - PCLKI2C0		(30)
			 *   - PCLKSSP0		(3)
			 * - early boot console cannot be disabled:
			 *  - UART3CLK		(64 + 16)
			 * - keep enabled until proper driver handling:
			 *  - CLCDCLK		(64 + 0)
			 *  - HCLKCLCD		(4)
			 * - Devices belonging to Boot devices list and
			 *   whose clock gating is controlled by SRC M3,
			 *   to avoid hang in bootrom on a SoC reset.
			 *  - UART0CLK	(66)
			 *  - UART1CLK	(70)
			 *  - UART2CLK	(76)
			 *  - UART3CLK	(80)
			 *  - SQICLK	(71)
			 *  - SDI0CLK	(67)
			 *  - SDI1CLK	(77)
			 *  - SDI2CLK	(86)
			 */
			if (gate->id == 31 || gate->id == 30 || gate->id == 3 ||
			    gate->id == (64 + 16) || gate->id == (64 + 0) ||
			    gate->id == 4 ||
			    gate->id == 66 || gate->id == 70 ||
			    gate->id == 76 || gate->id == 80 ||
			    gate->id == 71 || gate->id == 67 ||
			    gate->id == 77 || gate->id == 86)
				clk_init.ops = &sta_clk_gate_dummy_ops;
#ifdef CONFIG_REGMAP_SMC
			/*
			 * For safety reason, A7 MSCR clock gate cannot be
			 * disabled in case hardware isolation of critical
			 * peripheral is enabled in the C-M3 microcontroller
			 * sub-system. To avoid a system dead-lock at the
			 * time Linux ask to disable that clock, we use
			 * dummy ops for this particular clock.
			 */
			if (gate->id == 18)
				clk_init.ops = &sta_clk_gate_dummy_ops;
#endif

			hw = &clk_hw_p->sta_hw.gate.hw;
		} else {
			return ERR_PTR(-EINVAL);
		}

		hw->init = &clk_init;
		ret = clk_hw_register(NULL, hw);
		if (ret)
			return ERR_PTR(ret);
		break;
	}
	case STA_CLK_FIXED:
	{
		struct clk_fixed_rate *fixed = &clk_hw_p->fr;

		hw = clk_hw_register_fixed_rate(NULL, clk_p->name,
						clk_p->parents[0], clk_p->flags,
						fixed->fixed_rate);
		break;
	}
	case STA_CLK_FIXED_FACTOR:
	{
		struct clk_fixed_factor *factor = &clk_hw_p->ff;

		hw = clk_hw_register_fixed_factor(NULL, clk_p->name,
						  clk_p->parents[0],
						  clk_p->flags, factor->mult,
						  factor->div);
		break;
	}
	default:
		hw = ERR_PTR(-EINVAL);
	}

	return hw;
}

#define SRCM3_COMPAT			"st,sta1295-src-m3-clk"
#define SRCM3_COMPAT_CUT1		"st,sta1295-cut1-src-m3-clk"
#define SRCM3_COMPAT_CUT2		"st,sta1295-cut2-src-m3-clk"
#define SRCM3_COMPAT_STA1385		"st,sta1385-src-m3-clk"
#define SRCM3_COMPAT_STA1385_CUT1	"st,sta1385-cut1-src-m3-clk"
#define SRCA7_COMPAT			"st,sta1295-src-a7-clk"
#define A7SSC_COMPAT			"st,sta1295-a7-ssc-clk"
#define A7MSCR_COMPAT			"st,sta1295-mscr-a7"

static const struct of_device_id srcm3_match[] __initconst = {
	{ .compatible = SRCM3_COMPAT },
	{ .compatible = SRCM3_COMPAT_CUT1 },
	{ .compatible = SRCM3_COMPAT_CUT2 },
	{ .compatible = SRCM3_COMPAT_STA1385 },
	{ .compatible = SRCM3_COMPAT_STA1385_CUT1 },
	{ /* sentinel */ }
};

static const struct of_device_id srca7_match[] __initconst = {
	{ .compatible = SRCA7_COMPAT },
	{ /* sentinel */ }
};

static const struct of_device_id a7ssc_match[] __initconst = {
	{ .compatible = A7SSC_COMPAT },
	{ /* sentinel */ }
};

static const struct of_device_id a7mscr_match[] __initconst = {
	{ .compatible = A7MSCR_COMPAT },
	{ /* sentinel */ }
};

static int sta_clk_init(int ctrl, const struct of_device_id *match)
{
	struct device_node *np;
	struct regmap *regmap;
#ifdef CONFIG_REGMAP_SMC
	struct resource res;
	int ret;
#else
	void __iomem *base = NULL;
#endif

	np = of_find_matching_node(NULL, match);
	if (!np || !of_device_is_available(np)) {
		pr_info("node not found or disabled\n");
		return -ENODEV;
	}

#ifdef CONFIG_REGMAP_SMC
	ret = of_address_to_resource(np, 0, &res);
	if (ret)
		return ret;
	regmap = regmap_init_smc(NULL, &res, &regmap_configs[ctrl]);
#else
	base = of_iomap(np, 0);
	if (!base) {
		pr_err("failed to map system control block registers\n");
		return -ENODEV;
	}

	regmap = regmap_init_mmio(NULL, base, &regmap_configs[ctrl]);
#endif
	if (IS_ERR(regmap)) {
		pr_err("failed to regmap system control block: %ld\n",
		       PTR_ERR(regmap));
#ifndef CONFIG_REGMAP_SMC
		iounmap(base);
#endif
		return IS_ERR(regmap);
	}

	switch (ctrl) {
	case SRCM3:

		if (of_device_is_compatible(np, SRCM3_COMPAT_STA1385) ||
		    of_device_is_compatible(np, SRCM3_COMPAT_STA1385_CUT1))
			is_sta1385_compatible = true;
		if (of_device_is_compatible(np, SRCM3_COMPAT_CUT1) ||
		    of_device_is_compatible(np, SRCM3_COMPAT_CUT2) ||
		    of_device_is_compatible(np, SRCM3_COMPAT_STA1385_CUT1))
			quirk_canfd_old_cuts = true;
		srcm3_regmap = regmap;
		break;
	case SRCA7:
		srca7_regmap = regmap;
		break;
	case A7SSC:
		a7ssc_regmap = regmap;
		break;
	case A7MSCR:
		a7mscr_regmap = regmap;
		break;
	default:
		pr_err("unknown controller id: %i\n", ctrl);
		return -ENODEV;
	}

	pr_debug("%s init done\n", np->name);
	return 0;
}

static int sta_common_clk_init(void)
{
	struct clk_hw **hws;
	struct clk_hw *hw;
	int i, ret;
	static bool initialized;

	if (initialized)
		return 0;

	ret = sta_clk_init(SRCM3, srcm3_match);
	if (ret)
		goto end;
	ret = sta_clk_init(SRCA7, srca7_match);
	if (ret)
		goto end;
	ret = sta_clk_init(A7SSC, a7ssc_match);
	if (ret)
		goto end;

	/* The sta1385 SoC variant has additional clock mux and clock
	 * dividers registers for uart4, uart5 in the A7 MSCR
	 * register bank.
	 */
	if (is_sta1385_compatible) {
		ret = sta_clk_init(A7MSCR, a7mscr_match);
		if (ret)
			goto end;
	}

	ret = mxtal_clk_init();
	if (ret)
		goto end;
	ret = timer_clk_init();
	if (ret)
		goto end;

	hws = clk_data.hws;
	for (i = STA_CLK_MAX - 1; i >= 0; i--) {
		if (i == STA_CLK_MXTAL || i == STA_CLK_TIMER0 ||
		    i == STA_CLK_TIMER1 || i == STA_CLK_PLL3PHI)
			continue;
		if (is_sta1385_compatible && i == STA_CLK_PLL3FVCO)
			continue;
		if (!is_sta1385_compatible && (i == STA_CLK_UART45SEL ||
					       i == STA_CLK_UART45DIV ||
					       i == STA_CLK_WLAN))
			continue;
		hw = sta_clk_hw_register(i);
		if (IS_ERR(hw)) {
			pr_err("failed to register %s clock: %ld\n",
			       clk_protos[i].name, PTR_ERR(hw));
			hws[i] = ERR_PTR(-ENOENT);
			goto end;
		}
		hws[i] = hw;
	}
	if (!is_sta1385_compatible)
		ret = pll3phi_clk_init();
end:
	initialized = true;
	return ret;
}

static void __init srcm3_clk_init(struct device_node *np)
{
	if (!sta_common_clk_init())
		of_clk_add_hw_provider(np, of_clk_hw_onecell_get,
				       &clk_data);
}

CLK_OF_DECLARE(srcm3_clk, SRCM3_COMPAT, srcm3_clk_init);
CLK_OF_DECLARE(srcm3_clk_cut1, SRCM3_COMPAT_CUT1, srcm3_clk_init);
CLK_OF_DECLARE(srcm3_clk_cut2, SRCM3_COMPAT_CUT2, srcm3_clk_init);
CLK_OF_DECLARE(srcm3_clk_sta1385, SRCM3_COMPAT_STA1385, srcm3_clk_init);
CLK_OF_DECLARE(srcm3_clk_sta1385_cut1, SRCM3_COMPAT_STA1385_CUT1,
	       srcm3_clk_init);

static void __init srca7_clk_init(struct device_node *np)
{
	if (!sta_common_clk_init())
		of_clk_add_hw_provider(np, of_clk_hw_onecell_get,
				       &clk_data);
}

CLK_OF_DECLARE(srca7_clk, SRCA7_COMPAT, srca7_clk_init);

static void __init a7ssc_clk_init(struct device_node *np)
{
	if (!sta_common_clk_init())
		of_clk_add_hw_provider(np, of_clk_hw_onecell_get,
				       &clk_data);
}

CLK_OF_DECLARE(a7ssc_clk, A7SSC_COMPAT, a7ssc_clk_init);
