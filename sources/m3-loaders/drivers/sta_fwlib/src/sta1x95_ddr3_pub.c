/**
 * @file sta1x95_ddr3_pub.c
 * @brief This is the driver for the Synopsys physical utility block (PUB)
 * controller
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include <errno.h>


#include "utils.h"
#include "trace.h"

#include "sta_mtu.h"
#include "sta_gpio.h"
#include "sta_common.h"
#include "sta_ddr.h"
#include "sta1x95_ddr3_setting.h"
#include "sta_pinmux.h"

/* BIST registers */
#define BISTRR (0x40 * 4)
#define BISTWCR (0x41 * 4)
#define BISTMSKR0 (0x42 * 4)
#define BISTMSKR1 (0x43 * 4)
#define BISTMSKR2 (0x44 * 4)
#define BISTLSR (0x45 * 4)
#define BISTAR0 (0x46 * 4)
#define BISTAR1 (0x47 * 4)
#define BISTAR2 (0x48 * 4)
#define BISTUDPR (0x49 * 4)
#define BISTGSR (0x4A * 4)
#define BISTWER (0x4B * 4)
#define BISTBER0 (0x4C * 4)
#define BISTBER1 (0x4D * 4)
#define BISTBER2 (0x4E * 4)
#define BISTBER3 (0x4F * 4)
#define BISTWCSR (0x50 * 4)
#define BISTWR0 (0x51 * 4)
#define BISTWR1 (0x52 * 4)
#define BISTWR2 (0x53 * 4)

/* Data lanes */
#define PUB_DX0GSR2 (0x7D * 4)
#define PUB_DX1GSR2 (0x8D * 4)
#define PUB_DX2GSR2 (0x9D * 4)
#define PUB_DX3GSR2 (0xAD * 4)
#define PUB_DX4GSR2 (0xBD * 4)
#define PUB_DX5GSR2 (0xCD * 4)
#define PUB_DX6GSR2 (0xDD * 4)
#define PUB_DX7GSR2 (0xED * 4)
#define PUB_DX8GSR2 (0xFD * 4)

/* PIR register */
#define PIR_INIT BIT(0)
#define PIR_ZCAL BIT(1)
#define PIR_PLLINIT BIT(4)
#define PIR_DCAL BIT(5)
#define PIR_PHYRST BIT(6)
#define PIR_DRAMRST BIT(7)
#define PIR_DRAMINIT BIT(8)
#define PIR_WL BIT(9)
#define PIR_QSGATE BIT(10)
#define PIR_WLADJ BIT(11)
#define PIR_RDDSKW BIT(12)
#define PIR_WRDSKW BIT(13)
#define PIR_RDEYE BIT(14)
#define PIR_WREYE BIT(15)
#define PIR_ICPC BIT(16)
#define PIR_PLLBYP BIT(17)
#define PIR_CTLDINIT BIT(18)
#define PIR_RDIMMINIT BIT(19)
#define PIR_CLRSR BIT(27)
#define PIR_LOCKBYP BIT(28)
#define PIR_DCALBYP BIT(29)
#define PIR_ZCALBYP BIT(30)
#define PIR_INITBYP BIT(31)

/* PGSR0 register */
#define PGSR0_PLDONE	BIT(1)
#define PGSR0_DCDONE	BIT(2)
#define PGSR0_ZCDONE	BIT(3)
#define PGSR0_QSGDONE	BIT(6)
#define PGSR0_ZCERR BIT(20)
#define PGSR0_WLERR BIT(21)
#define PGSR0_QSGERR BIT(22)
#define PGSR0_WLAERR BIT(23)
#define PGSR0_RRERR	BIT(24)
#define PGSR0_WDERR	BIT(25)
#define PGSR0_REERR	BIT(26)
#define PGSR0_WEERR	BIT(27)

#define PGSR0_ZCERR_MASK	BIT(0)
#define PGSR0_WLERR_MASK	BIT(1)
#define PGSR0_QSGERR_MASK	BIT(2)
#define PGSR0_WLAERR_MASK	BIT(3)
#define PGSR0_RDERR_MASK	BIT(4)
#define PGSR0_WDERR_MASK	BIT(5)
#define PGSR0_REERR_MASK	BIT(6)
#define PGSR0_WEERR_MASK	BIT(7)
#define PGSR0_ERR_MASK		(PGSR0_ZCERR_MASK | PGSR0_WLERR_MASK | \
	PGSR0_QSGERR_MASK | PGSR0_WLAERR_MASK | PGSR0_RDERR_MASK | \
	PGSR0_WDERR_MASK | PGSR0_REERR_MASK | PGSR0_WEERR_MASK)

#define PGSR0_ERR_SHIFT 20

#define RESETn M3_GPIO(2)

#if defined(CFG_DDR_ALT1) || defined(CFG_DDR_ALT2)
#define MAX_BYTE_LANES 2
#else
#define MAX_BYTE_LANES 4
#endif

/* Built-in Self Test option, enable USB_BIST to use it.
 * Below, the set of settings that can be modified in order to run different
 * kind of BIST sequences (DRAM or Loopback mode), with different kind of
 * patterns (walking bit 0 or 1, random or user defined).
 * User can also modify the number of rows, columns, banks to test.
 */
#ifdef USE_BIST

#define BIST_MAX_LANES MAX_BYTE_LANES

#define BIST_DRAM_MODE 0
#define BIST_LOOP_MODE 1

#define BIST_START_COL 0 /* col address, must be BL aligned */
#define BIST_START_ROW 0 /* row address */
#define BIST_START_BANK 0 /* bank address */

#define BIST_START_BANK 0 /* bank address */
#define BIST_RANK1 0

#define BIST_WALKING_BIT0 0
#define BIST_WALKING_BIT1 1
#define BIST_LFSR 2
#define BIST_USER_DEFINED 3

#define BIST_USER_PATTERN 0xA5A5A5A5

#define BIST_WORD_COUNT 0xFFFF /* words to be generated */

/* constants below are proper to a Micron MT41K256M16 - 32 Meg x 16 x 8 banks */
#define BIST_MCOL (1024 - 8) /* 1K (max - Burst length) */
#define BIST_MROW 32767 /* 32K */
#define BIST_MBANK 7 /* 8 */

/* values to be adapted according to the BIST to be done */

/* Setting this to true will cause BIST never ending */
static bool bist_infinite = false;

static int bist_mode = BIST_DRAM_MODE;
static int bist_pattern = BIST_LFSR;
static int bist_user_pattern = BIST_USER_PATTERN;
static int bist_wc = BIST_WORD_COUNT;

#endif /* USE_BIST */

/**
 * @brief	Check for warnings in all data lanes
 * @return	The number of warnings
 */
static void ddr3_pub_has_warnings(void)
{
	int i, n;
	uint32_t reg, wrn = 0;

	/* Looping from DX0GSR2 to DX8GSR2 */
	for (i = PUB_DX0GSR2, n = 0; i < PUB_DX8GSR2; i += (0x10 * 4), n++) {
		reg = ddr_read_reg(DDR3_PUB_BASE + i);
		if (reg & 0xaa) {
			TRACE_INFO("%s: warning on DATX8-#%d (PUB_DX%dGSR2 = 0x%08x)\n",
					__func__, n, n, reg);
			wrn |= BIT(n);
		}
	}

	return;
}

#ifdef USE_BIST
static void ddr3_bist(uint32_t sc, uint32_t sr, uint32_t sb, uint32_t mc,
		uint32_t mr, uint32_t mb, uint32_t rk, bool bist_dump)
{
	uint32_t r;
	int l;

	/* Extend the DQS gate for the BIST */
	ddr_write_reg(4, DDR3_PUB_BASE + BISTWCR);

	/* Enable extended gate for BIST */
	r = ddr_read_reg(DDR3_PUB_BASE + PUB_DSGCR);
	ddr_write_reg(r | BIT(6), DDR3_PUB_BASE + PUB_DSGCR);

	/* Enable extended gate for BIST */
	r = ddr_read_reg(DDR3_PUB_BASE + PUB_DTPR2);
	ddr_write_reg(r | BIT(30), DDR3_PUB_BASE + PUB_DTPR2);

	/* set the number of patterns to be generated */
	ddr_write_reg(bist_wc, DDR3_PUB_BASE + BISTWCR);

	r = sc; /* col address */
	r |= sr; /* row address */
	r |= sb; /* bank address */

	ddr_write_reg(r, DDR3_PUB_BASE + BISTAR0);

	r = rk; /* only on rank 1, address increment is 000 for BL8 */

	if (bist_mode == BIST_DRAM_MODE)
		r |= BIT(7); /* set the BAINC to b1000 */

	ddr_write_reg(r, DDR3_PUB_BASE + BISTAR1);

	/* configure max values */
	r = mc;
	r |= mr << 12;
	r |= mb << 28;
	ddr_write_reg(r, DDR3_PUB_BASE + BISTAR2);

	if (bist_pattern == BIST_USER_DEFINED)
		ddr_write_reg(bist_user_pattern, DDR3_PUB_BASE + BISTUDPR);

	for (l = 0; l < BIST_MAX_LANES; l++) {
		TRACE_NOTICE("%s: BIST (%s) started on lane %d\n", __func__,
				(bist_mode == BIST_DRAM_MODE ? "DRAM mode" : "Loopback mode"), l);
		/* trigger BIST running */
		r = ddr_read_reg(DDR3_PUB_BASE + BISTRR);
		r = (r & 0xFF81FFF0) | BIT(0) | BIT(1); /* reset BIST */

		if (bist_mode == BIST_DRAM_MODE) {
			r |= BIT(3);
			r |= BIT(14); /* for DatX8 */
			/* r |= BIT(15); for A/C */
		}

		switch (bist_pattern) {
		case BIST_WALKING_BIT0:
			/* do nothing */
			break;
		case BIST_WALKING_BIT1:
			r |= BIT(17);
			break;
		case BIST_USER_DEFINED:
			r |= BIT(17);
			r |= BIT(18);
			break;
		case BIST_LFSR:
		default:
			r |= BIT(18);
			break;
		}

		r |= (l << 19); /* test on current lane */

		if (bist_infinite)
			r |= BIT(4);

		ddr_write_reg(r, DDR3_PUB_BASE + BISTRR);

		/* let some delay to reset BIST */
		mdelay(1);
		r = ddr_read_reg(DDR3_PUB_BASE + BISTRR);
		ddr_write_reg(r & ~BIT(1), DDR3_PUB_BASE + BISTRR); /* set BINST to 01 to start */

		do {
			r = ddr_read_reg(DDR3_PUB_BASE + BISTGSR);
		} while ((r & BIT(0)) != 1);
		TRACE_NOTICE("%s: BIST is over 0x%08x\n", __func__, ddr_read_reg(DDR3_PUB_BASE + BISTWER));

		if (bist_dump) {
			TRACE_NOTICE("BISTRR = 0x%08x\n", read_reg(DDR3_PUB_BASE + BISTRR));
			TRACE_NOTICE("BISTWCR = 0x%08x\n", read_reg(DDR3_PUB_BASE + BISTWCR));
			TRACE_NOTICE("BISTMSKR0 = 0x%08x\n", read_reg(DDR3_PUB_BASE + BISTMSKR0));
			TRACE_NOTICE("BISTMSKR1 = 0x%08x\n", read_reg(DDR3_PUB_BASE + BISTMSKR1));
			TRACE_NOTICE("BISTMSKR2 = 0x%08x\n", read_reg(DDR3_PUB_BASE + BISTMSKR2));
			TRACE_NOTICE("BISTLSR = 0x%08x\n", read_reg(DDR3_PUB_BASE + BISTLSR));
			TRACE_NOTICE("BISTAR0 = 0x%08x\n", read_reg(DDR3_PUB_BASE + BISTAR0));
			TRACE_NOTICE("BISTAR1 = 0x%08x\n", read_reg(DDR3_PUB_BASE + BISTAR1));
			TRACE_NOTICE("BISTAR2 = 0x%08x\n", read_reg(DDR3_PUB_BASE + BISTAR2));
			TRACE_NOTICE("BISTUDPR = 0x%08x\n", read_reg(DDR3_PUB_BASE + BISTUDPR));
			TRACE_NOTICE("BISTGSR = 0x%08x\n", read_reg(DDR3_PUB_BASE + BISTGSR));
			TRACE_NOTICE("BISTWER = 0x%08x\n", read_reg(DDR3_PUB_BASE + BISTWER));
			TRACE_NOTICE("BISTBER0 = 0x%08x\n", read_reg(DDR3_PUB_BASE + BISTBER0));
			TRACE_NOTICE("BISTBER1 = 0x%08x\n", read_reg(DDR3_PUB_BASE + BISTBER1));
			TRACE_NOTICE("BISTBER2 = 0x%08x\n", read_reg(DDR3_PUB_BASE + BISTBER2));
			TRACE_NOTICE("BISTBER3 = 0x%08x\n", read_reg(DDR3_PUB_BASE + BISTBER3));
			TRACE_NOTICE("BISTWCSR = 0x%08x\n", read_reg(DDR3_PUB_BASE + BISTWCSR));
			TRACE_NOTICE("BISTWR0 = 0x%08x\n", read_reg(DDR3_PUB_BASE + BISTWR0));
			TRACE_NOTICE("BISTWR1 = 0x%08x\n", read_reg(DDR3_PUB_BASE + BISTWR1));
			TRACE_NOTICE("BISTWR2 = 0x%08x\n", read_reg(DDR3_PUB_BASE + BISTWR2));
		}

	}
	r = ddr_read_reg(DDR3_PUB_BASE + PUB_PGCR1);
	ddr_write_reg(r & ~BIT(26), DDR3_PUB_BASE + PUB_PGCR1); /* De-inhibit VT calc for mission mode */

	/* Roll-back to previous state */
	r = ddr_read_reg(DDR3_PUB_BASE + PUB_DSGCR);
	ddr_write_reg(r & ~BIT(6), DDR3_PUB_BASE + PUB_DSGCR); /* Disable extended gate for BIST */

	r = ddr_read_reg(DDR3_PUB_BASE + PUB_DTPR2);
	ddr_write_reg(r & ~BIT(30), DDR3_PUB_BASE + PUB_DTPR2); /* Disable extended gate for BIST */
}
#endif /* USE_BIST */

/**
 * @brief	Place IO in retention mode or not depending on the need.
 * This is done thanks to a GPIO because on SoC sta1x95 thos lines are not
 * implemented. Note that only RESETn is managed here, CKE remains low thanks
 * to a resistor bridge.
 */
int sta_ddr_pub_set_io_retention(bool set)
{
	struct gpio_config pin;

	pin.direction = GPIO_DIR_OUTPUT;
	pin.mode = GPIO_MODE_SOFTWARE;
	pin.level = GPIO_LEVEL_LEAVE_UNCHANGED;
	pin.trig = GPIO_TRIG_LEAVE_UNCHANGED;

	gpio_set_pin_config(RESETn, &pin);

	if (set)
		gpio_set_gpio_pin(RESETn);
	else
		gpio_clear_gpio_pin(RESETn);

	return 0;
}

int sta_ddr_pub_do_training(uint32_t mask)
{
	uint32_t ret;

#if defined(USE_BIST)
	/* bist in DRAM mode needs training to be done,
	 * this is not useful for LOOPBACK mode
	 */
	if (bist_mode == BIST_LOOP_MODE)
		return 0;
#endif

	if (mask == PUB_FULL_TRAINING)
		ddr3_pub_do_pir(PIR_WL | PIR_QSGATE | PIR_WLADJ | PIR_RDDSKW |
				PIR_WRDSKW | PIR_RDEYE | PIR_WREYE | PIR_INIT);
	else
		ddr3_pub_do_pir(mask);

	/* PUB block integrates some general status registers for the DATX8.
	 * Among other things they inform some warnings occurring during
	 * the calibration/training stages: Read Bit Deskew /
	 * Write Bit Deskew / Read Data Eye / Write Data Eye
	 */
	ddr3_pub_has_warnings();

	ret = ddr3_pub_has_errors(PGSR0_ERR_SHIFT, PGSR0_ERR_MASK);
	if (ret) {
		TRACE_ERR("%s: DDR PHY training ended with errors (error mask:0x%x)\n", __func__, ret);
		return ret;
	}

	return ret;
}

/**
 * @brief DDR Phy utility block configuration
 * Configure first all relevant settings, DDR type, timings, cycles, refresh,
 * etc.
 * Then, the PUB has a built-in SDRAM initialization routine that may be
 * triggered by software or memory controller by writing to the PHY
 * Initialization Register (PIR). Once this initialization is done, the
 * PGSR0.IDONE bit is set.
 */
int sta_ddr_pub_configure(uint32_t boot_reason)
{
	int i = 0;
	uint32_t ret;
	uint32_t reg;

	mdelay(10);

	/* By default, right after the reset is lifted the PHY init phase is
	 * automatically started with default PUB registers values. It seems that
	 * default values are not fitting with our system, as a consequence, DCAL
	 * phase never ends and PGSR0.IDONE bit is never set.
	 * the workaround suggested by Synopsys is to bypass the entire procedure,
	 * then trigger it again later on.
	 */
	ret = ddr3_pub_do_pir(PIR_INITBYP);
	if (ret) {
		TRACE_ERR("%s:%d\n", __func__, __LINE__);
		return -EIO;
	}

	/* presetn is already lifted up at boot, so they cannot be overlaped
	 * with ctl_rst_n.
	 * Not having the overlap could cause issues at the clock crossing between
	 * the ctl_clk domain and pclk domain.  The overlap in resets is to ensure
	 * those clock crossing paths are properly flushed out.
	 * Issuing 16 write in RIDR register solves the problem. At least performing
	 * the 16 writes to the RIDR registers should ensure that the clock crossings
	 * are in a predictable state after that's done.
	 */
	for (i = 0; i < 16; i++)
		ddr_write_reg(i, DDR3_PUB_BASE + PUB_RIDR);

	/* initialize controller */
	/* Data Training register configuration */
	ddr_write_reg(DDRVAL(DTCR) | BIT(7), DDRADDR(DTCR));

	reg = ddr_read_reg(DDRADDR(DSGCR));
	ddr_write_reg(reg | BIT(18), DDRADDR(DSGCR));

	ddr_write_reg(DDRVAL(ODTCR), DDRADDR(ODTCR)); /* enable RD and WR ODT */
	ddr_write_reg(DDRVAL(DCR), DDRADDR(DCR));

	ddr_write_reg(DDRVAL(PTR0), DDRADDR(PTR0));
	ddr_write_reg(DDRVAL(PTR1), DDRADDR(PTR1));
	ddr_write_reg(DDRVAL(PTR2), DDRADDR(PTR2));
	ddr_write_reg(DDRVAL(PTR3), DDRADDR(PTR3));
	ddr_write_reg(DDRVAL(PTR4), DDRADDR(PTR4));
	ddr_write_reg(DDRVAL(DTPR0), DDRADDR(DTPR0));
	ddr_write_reg(DDRVAL(DTPR1), DDRADDR(DTPR1));
	ddr_write_reg(DDRVAL(DTPR2), DDRADDR(DTPR2));
	ddr_write_reg(DDRVAL(MR0), DDRADDR(MR0));
	ddr_write_reg(DDRVAL(MR1), DDRADDR(MR1));
	ddr_write_reg(DDRVAL(MR2), DDRADDR(MR2));
	ddr_write_reg(DDRVAL(MR3), DDRADDR(MR3));

	/* DATX8 General configuration register */
#if defined(CFG_DDR_ALT1) || defined(CFG_DDR_ALT2)
	ddr_write_reg(0x44000e81, DDRADDR(DX0GCR));
	ddr_write_reg(0x44000e81, DDRADDR(DX1GCR));
	ddr_write_reg(0x80028070, DDRADDR(DX2GCR));
	ddr_write_reg(0x80028070, DDRADDR(DX3GCR));
#else
	ddr_write_reg(0x44000e81, DDRADDR(DX0GCR));
	ddr_write_reg(0x44000e81, DDRADDR(DX1GCR));
	ddr_write_reg(0x44000e81, DDRADDR(DX2GCR));
	ddr_write_reg(0x44000e81, DDRADDR(DX3GCR));
#endif

	ddr_write_reg(DDRVAL(PGCR2), DDRADDR(PGCR2));

#ifdef USE_BIST
	reg = ddr_read_reg(DDRADDR(PGCR1));
	reg |= BIT(26); /* Inhibit VT calc for BIST */

	/* Only applicable for loopback mode
	 * Loopback is before buffer, output is "don't care"
	 */
	if (bist_mode == BIST_LOOP_MODE)
		reg |= BIT(27);

	ddr_write_reg(reg, DDRADDR(PGCR1));
#endif /* USE_BIST */

	/* DXnBDLR3/4 (read delay) are set to default values */
	ddr_write_reg(DDRVAL(DX0BDLR0), DDRADDR(DX0BDLR0));
	ddr_write_reg(DDRVAL(DX0BDLR1), DDRADDR(DX0BDLR1));
	ddr_write_reg(DDRVAL(DX0BDLR2), DDRADDR(DX0BDLR2));
	ddr_write_reg(DDRVAL(DX1BDLR0), DDRADDR(DX1BDLR0));
	ddr_write_reg(DDRVAL(DX1BDLR1), DDRADDR(DX1BDLR1));
	ddr_write_reg(DDRVAL(DX1BDLR2), DDRADDR(DX1BDLR2));
	ddr_write_reg(DDRVAL(DX2BDLR0), DDRADDR(DX2BDLR0));
	ddr_write_reg(DDRVAL(DX2BDLR1), DDRADDR(DX2BDLR1));
	ddr_write_reg(DDRVAL(DX2BDLR2), DDRADDR(DX2BDLR2));
	ddr_write_reg(DDRVAL(DX3BDLR0), DDRADDR(DX3BDLR0));
	ddr_write_reg(DDRVAL(DX3BDLR1), DDRADDR(DX3BDLR1));
	ddr_write_reg(DDRVAL(DX3BDLR2), DDRADDR(DX3BDLR2));


	/* initialize PHY */
	switch (boot_reason) {
	case DDR_BOOT_REASON_COLD_BOOT:
		/* trigger the full init */
		ret = ddr3_pub_do_pir(PIR_DCAL | PIR_ZCAL | PIR_PLLINIT | PIR_PHYRST | PIR_INIT);
		if (ret) {
			TRACE_ERR("%s:%d\n", __func__, __LINE__);
			return -EIO;
		}

		ret = ddr3_pub_has_errors(PGSR0_ERR_SHIFT, PGSR0_ZCERR_MASK);
		if (ret) {
			TRACE_ERR("%s: failed to initialize DDR PUB (ZCAL error)\n", __func__);
			return -EIO;
		}

		reg = PIR_DRAMRST | PIR_DRAMINIT | PIR_INIT;

#if defined (USE_BIST)
		/* BIST loopback mode, skip DRAM init */
		if (bist_mode == BIST_LOOP_MODE)
			reg = PIR_CTLDINIT;
#endif

		/* initialize SDRAM */
		ret = ddr3_pub_do_pir(reg);
		if (ret) {
			TRACE_ERR("%s:%d\n", __func__, __LINE__);
			return -EIO;
		}

		/* initiate training without test */
		ret = ddr3_pub_do_pir(PIR_WL | PIR_QSGATE | PIR_WLADJ | PIR_RDDSKW |
				PIR_WRDSKW | PIR_RDEYE | PIR_WREYE | PIR_INIT);
		if (ret) {
			TRACE_ERR("%s:%d\n", __func__, __LINE__);
			return -EIO;
		}

		/* initiate training */
		ret = sta_ddr_pub_do_training(PIR_WL | PIR_QSGATE | PIR_WLADJ | PIR_RDDSKW |
				PIR_WRDSKW | PIR_RDEYE | PIR_WREYE | PIR_INIT);
		if (ret)
			return -EIO;

		break;
	case DDR_BOOT_REASON_EXIT_SELF_REF:
		/* bypass ZQ calibration and get back
		 * stored values from non-volatile memory
		 */
		ret = ddr3_pub_do_pir(PIR_DCAL | PIR_ZCALBYP | PIR_PLLINIT | PIR_PHYRST | PIR_INIT);
		if (ret) {
			TRACE_ERR("%s:%d\n", __func__, __LINE__);
			return -EIO;
		}

		/* restore and apply ZQ calibration values */
		ddr3_pub_load_zq_cal();

		/* trigger ZQ calibration */
		ret = ddr3_pub_do_pir(PIR_ZCAL | PIR_INIT);
		if (ret) {
			TRACE_ERR("%s:%d\n", __func__, __LINE__);
			return -EIO;
		}

		ret = ddr3_pub_has_errors(PGSR0_ERR_SHIFT, PGSR0_ZCERR_MASK);
		if (ret) {
			TRACE_ERR("%s: failed to initialize DDR PUB (ZCAL error)\n", __func__);
			return -EIO;
		}

		/* indicate to the PUB that SDRAM init will not be triggered */
		ret = ddr3_pub_do_pir(PIR_CTLDINIT | PIR_INIT);
		if (ret) {
			TRACE_ERR("%s:%d\n", __func__, __LINE__);
			return -EIO;
		}
		break;
	default:
		return -EINVAL;
	}

#if defined(USE_BIST)
	if (boot_reason == DDR_BOOT_REASON_COLD_BOOT)
		ddr3_bist(BIST_START_COL, BIST_START_ROW, BIST_START_BANK, BIST_MCOL, BIST_MROW,
			BIST_MBANK, BIST_RANK1, false);
#endif

	return 0;
}
