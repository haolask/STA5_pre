/**
 * @file sta_ddr3_ctl.c
 * @brief This is the driver for the Synopsys universal DDR Memory Controller
 * (uMCTL2)
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "trace.h"

#include "sta_mem_map.h"

#include "sta_mtu.h"
#include "sta_platform.h"
#include "sta_ddr.h"
#include "sta_src.h"

#define PUB_CONF_MAX_TRIES 3

/**
 * @brief	configure PIR register according to given mask
 */
static inline void ddr3_pub_pir_configure(uint32_t pir_offset, uint32_t pir_mask)
{
	ddr_write_reg(pir_mask, DDR3_PUB_BASE + pir_offset);
}

/**
 * @brief	Polls PUB_PGSR0 for a given bit to be set
 */
static int ddr3_pub_poll_status(uint32_t pgsr_offset, uint32_t expected_pgsr0)
{
	uint32_t pgsr0;
	uint32_t start;

	start = mtu_get_timebase(TIME_IN_US);

	do {
		pgsr0 = ddr_read_reg(DDR3_PUB_BASE + pgsr_offset);
		if ((mtu_get_timebase(TIME_IN_US) - start) > DDR_SYST_INIT_TIMEOUT_US)
			break;
	} while ((pgsr0 & expected_pgsr0) != expected_pgsr0);

	return ((pgsr0 & expected_pgsr0) != expected_pgsr0);
}

/**
 * @brief	Simple function to perform PIR routine and poll result
 */
int ddr3_pub_do_pir(uint32_t pir_mask)
{
	uint32_t pgsr_offset;

	switch (get_soc_id()) {
		case SOCID_STA1195:
		case SOCID_STA1275:
		case SOCID_STA1295:
			pgsr_offset = PUB_PGSR0;
			break;
		case SOCID_STA1385:
			pgsr_offset = PUBL_PGSR;
			break;
		default:
			return -EINVAL;
	}

	ddr3_pub_pir_configure(PUB_PIR, pir_mask);
	udelay(20);

	if (ddr3_pub_poll_status(pgsr_offset, PGSR0_IDONE)) {
		TRACE_ERR("%s: DDR PHY TimeOut for DDR System Initialisation, General Status Register %x\n",
			  __func__, read_reg(DDR3_PUB_BASE + pgsr_offset));
		return -EINVAL;
	}

	return 0;
}

/**
 * @brief	Save ZQ calibration values into a specific storage location.
 */
void ddr3_pub_save_zq_cal(void)
{
	uint32_t reg;

	/* Store ZDATA of each byte lane in non-volatile memory */
	reg = ddr_read_reg(DDR3_PUB_BASE + PUB_ZQCR0(0));
	ddr_write_reg(reg & ZDATA_MASK, ZDATA_STORAGE);
}

/**
 * @brief	Restore ZQ calibration values from location
 */
void ddr3_pub_load_zq_cal(void)
{
	uint32_t zdata;
	uint32_t reg;

	/*
	 * Override ZQ calibration for each byte lane
	 * by writing 1 to ZDEN bit and then by loading from Backup RAM the
	 * previously stored values. Once done, ZDEN is set back to 0.
	 */
	reg = ddr_read_reg(DDR3_PUB_BASE +  + PUB_ZQCR0(0));
	reg |= ZQCR0_ZDEN;

	zdata = ddr_read_reg(ZDATA_STORAGE);
	reg |= zdata & ZDATA_MASK;
	ddr_write_reg(reg, DDR3_PUB_BASE +  + PUB_ZQCR0(0));

	/* remove IOs from retention mode */
	sta_ddr_pub_set_io_retention(false);

	reg = ddr_read_reg(DDR3_PUB_BASE +  + PUB_ZQCR0(0));
	reg &= ~ZQCR0_ZDEN;
	ddr_write_reg(reg, DDR3_PUB_BASE +  + PUB_ZQCR0(0));
}

/**
 * @brief	Backup DDR in BackupRAM data overwritten by data training during
 * suspend-resume procedure
 */
void ddr3_save_data_written_by_training(void)
{
	/* Backup DDR used for training at following SelfRefresh Exit */
	memcpy((void *)TRAINING_DATA_STORAGE,
	       (const void *)TRAINING_DATA_LOCATION, TRAINING_DATA_SIZE);
}

/**
 * @brief	Restore DDR data from BackupRAM which have been overwritten
 * by data training
 */
void ddr3_load_data_written_by_training(void)
{
	/* Backup DDR used for training at following SelfRefresh Exit */
	memcpy((void *)TRAINING_DATA_LOCATION,
	       (const void *)TRAINING_DATA_STORAGE, TRAINING_DATA_SIZE);
}


/**
 * @brief	Check for errors in all data lanes
 * @return	0 if no errors, not 0 otherwise
 */
uint32_t ddr3_pub_has_errors(uint32_t shift, uint32_t mask)
{
	uint32_t reg;
	uint32_t pgsr_offset;

	switch (get_soc_id()) {
		case SOCID_STA1195:
		case SOCID_STA1275:
		case SOCID_STA1295:
			pgsr_offset = PUB_PGSR0;
			break;
		case SOCID_STA1385:
			pgsr_offset = PUBL_PGSR;
			break;
		default:
			return -EINVAL;
	}

	reg = ddr_read_reg(DDR3_PUB_BASE + pgsr_offset);
	if (((reg >> shift) & mask) != 0)
		return ((reg >> shift) & mask);

	return 0;
}

void sta_ddr_display_rate(void)
{
	__maybe_unused uint32_t fxtal = m3_get_mxtal() / 1000000;
	__maybe_unused uint32_t vco = (2 * src_m3_regs->plldfctrl.bit.ndiv * fxtal);
	__maybe_unused uint32_t phi = vco * 1/2 * 1/(1 << src_a7_regs->scplldfctrl.bit.pll_odf);

	TRACE_NOTICE("PLLD.VCO   = %dMHz\n", vco);
	TRACE_NOTICE("PLLD.PHI   = %dMHz\n", phi);
	TRACE_NOTICE("DDR.CLKOUT = %dMHz\n", (get_soc_id() == SOCID_STA1385 ? 1 : 2) * phi);
}

void sta_ddr_ctl_test_pattern(uint32_t max, bool display_temp,
		uint32_t pat, uint32_t s, bool r, bool w, int type)
{
#define FIXED_PATTERN			0
#define INCREMENTAL_PATTERN		1
#define RANDOM_PATTERN			2

	uint32_t reg;
	uint32_t i, j, k;
	uint32_t memsize;
	uint32_t split = s; /* test the by chucks of 1/split ..*/
	uint32_t chunck;
	uint32_t address = 0xa0000000;
	uint32_t pattern = 0;

	uint32_t lfsr = pat;
	uint32_t bit;

	__maybe_unused uint32_t start = 0, end = 0;

	memsize = DDRAM_SIZE;
	chunck = memsize / split;

	TRACE_NOTICE("%s: Start of mem test %s memsize:0x%x\n", __func__,
		     (max == 0 ? "(infinite mode)" : ""), memsize);
	for (k = 0; (k < max) || (max == 0); k++) {
		TRACE_NOTICE("%s: run %d\n", __func__, (k+1));
		start = mtu_get_timebase(TIME_IN_SEC) * 1000000
				+ mtu_get_timebase(TIME_IN_US);

		for (j = 0; j < split; j++) {
			uint32_t start_offset = j * chunck;
			uint32_t end_offset = (j + 1) * chunck;

			pattern = 0;
			lfsr = pat;

			if (display_temp)
				TRACE_NOTICE("Temperature: %dC\n", m3_get_soc_temperature(100));

			if (w) {
				TRACE_NOTICE("%s: Writing %s pattern to memory...\n",
						__func__, (type == FIXED_PATTERN ? "fixed" :
							(type == INCREMENTAL_PATTERN ? "incremental" : "random")));

				for (i = start_offset; i < end_offset; i += 4) {
					switch (type) {
					case FIXED_PATTERN:
						write_reg(pat, address + i);
						break;
					case INCREMENTAL_PATTERN:
						write_reg(pattern++, address + i);
						break;
					case RANDOM_PATTERN:
						bit  = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5) ) & 1;
						lfsr =  (lfsr >> 1) | (bit << 31);
						write_reg(lfsr, address + i);
						break;
					}
				}
			}

			/* reset values for checking */
			pattern = 0;
			lfsr = pat;

			if (r) {
				TRACE_NOTICE("%s: Reading back %sthe memory... (0x%08x - 0x%08x)\n",
						__func__, (w ? "and checking " : ""), address + start_offset,
						address + end_offset);
				for (i = start_offset; i < end_offset; i += 4) {
					reg = read_reg(address + i);
					switch (type) {
					case FIXED_PATTERN:
						if (reg != pat)
							TRACE_ERR("%s:Error @: %08x: (%08x vs. %08x)\n",
									__func__, i, reg, pat);
						break;
					case INCREMENTAL_PATTERN:
						if (reg != pattern)
							TRACE_ERR("%s:Error @: %08x: (%08x vs. %08x)\n",
									__func__, i, reg, pattern);
						pattern++;
						break;
					case RANDOM_PATTERN:
						bit  = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5) ) & 1;
						lfsr =  (lfsr >> 1) | (bit << 31);
						if (reg != lfsr)
							TRACE_ERR("%s:Error @: %08x: (%08x vs. %08x)\n",
									__func__, i, reg, lfsr);
						break;
					}
				}
			}
		}
		end = mtu_get_timebase(TIME_IN_SEC) * 1000000
					+ mtu_get_timebase(TIME_IN_US);
		TRACE_NOTICE("%s: end of run %d (%d MiB/s)\n", __func__, (k+1),
				((memsize >> 10)*1000)/(end-start));
	}
	TRACE_NOTICE("%s: End of mem test\n", __func__);
}

static void sta_ddr_axi_unreset(void)
{
	src_a7_regs->scresctrl1.bit.ddr_axi_rst = 1;
}

void sta_ddr_axi_reset(void)
{
	src_a7_regs->scresctrl1.bit.ddr_axi_rst = 0;
}

static void sta_ddr_core_unreset(void)
{
	if (get_soc_id() == SOCID_STA1385)
		src_m3_regs->resstat.bit_sta1385.ddr_reset_status = 1;
	else
		src_m3_regs->resstat.bit_sta1x95.ddr_reset_status = 1;
}

void sta_ddr_core_reset(void)
{
	if (get_soc_id() == SOCID_STA1385)
		src_m3_regs->resstat.bit_sta1385.ddr_reset_status = 0;
	else
		src_m3_regs->resstat.bit_sta1x95.ddr_reset_status = 0;
}

int sta_ddr_init(uint32_t boot_reason)
{
	int err;
	unsigned int try;

	sta_ddr_core_reset();
	sta_ddr_axi_reset();

	/* DDR3 CTL initialization */
	err = sta_ddr_ctl_configure(boot_reason);
	if (err) {
		TRACE_ERR("%s: failed to configure umctl2\n", __func__);
		goto end;
	}

#ifdef LPDDR2
	sta_ddr_pub_set_io_retention(false);
#endif

	for (try = 0; try < PUB_CONF_MAX_TRIES; try++) {
		/*
		 * Unreset the DDR Phy
		 */
		sta_ddr_core_unreset();
		sta_ddr_axi_unreset();
		mdelay(10);

		/* Let's configure PLLD to it's nominal frequency after having released
		 * the reset
		 */
		if (get_soc_id() == SOCID_STA1295 ||
		    get_soc_id() == SOCID_STA1195 ||
		    get_soc_id() == SOCID_STA1275) {
			srcm3_pll_disable(src_m3_regs, PLLD);
			srcm3_plld_update_freq(src_m3_regs, 55, 2);
			srcm3_pll_enable(src_m3_regs, PLLD);
			while (!srcm3_is_pll_enabled(src_m3_regs, PLLD));
		}

		if (get_soc_id() == SOCID_STA1385 &&
		    get_cut_rev() >= CUT_20) {
			src_a7_regs->scclkdivcr.bit.clk_source = 1;
		}

		/* DDR3 PHY initialization */
		err = sta_ddr_pub_configure(boot_reason);
		if (!err)
			break;

		sta_ddr_core_reset();
		sta_ddr_axi_reset();

		/*
		 * For STA1195, STA1295 and STA1275 we configure PLLD to
		 * half its nominal frequency before lifting the reset.
		 */
		if (get_soc_id() == SOCID_STA1295 ||
		    get_soc_id() == SOCID_STA1195 ||
		    get_soc_id() == SOCID_STA1275) {
			srcm3_pll_disable(src_m3_regs, PLLD);
			srcm3_plld_update_freq(src_m3_regs, 28, 2);
			srcm3_pll_enable(src_m3_regs, PLLD);
			while (!srcm3_is_pll_enabled(src_m3_regs, PLLD));
		}

		mdelay(100);
	}

	if (err) {
		TRACE_ERR("%s: failed to configure pub\n", __func__);
		goto end;
	}

	/* DDR3 Controller final wait for PHY initilization and come out of debug
	 * and power up
	 */
	err = sta_ddr_ctl_power_up(boot_reason);
	if (err) {
		TRACE_ERR("%s: failed to power_up\n", __func__);
	}

end:
	if (err)
		TRACE_ERR("%s: failed to initialize DDR (code %d)\n", __func__, err);

	return err;
}

#ifdef VERBOSE_DDR

void sta_ddr_dump(void)
{
	uint32_t idx;

	TRACE_NOTICE("DDR setting\n");

	for (idx = 0;
	     strcmp(sta_ddr_setting_table[idx].label, "MAX_IDX");
	     idx++)
		TRACE_NOTICE("\tLabel:%-20s\tAddr:0x%08x\t\tValue:0x%08x\n",
			     sta_ddr_setting_table[idx].label,
			     sta_ddr_setting_table[idx].addr,
			     read_reg(sta_ddr_setting_table[idx].addr));
}

static int sta_ddr_find_idx_setting(char *label)
{
	uint32_t idx;

	if (!label)
		return -1;

	for (idx = 0;
	     strcmp(sta_ddr_setting_table[idx].label, "MAX_IDX") && strcmp(sta_ddr_setting_table[idx].label, label);
	     idx++)
		;

	return !strcmp(sta_ddr_setting_table[idx].label, "MAX_IDX") ? -1 : (int)idx;
}

void sta_ddr_set_setting(char *label, uint32_t value)
{
	int idx = sta_ddr_find_idx_setting(label);

	if (idx < 0) {
		TRACE_NOTICE("Label %s unknown\n");
	} else {
		sta_ddr_setting_table[idx].value = value;
		ddr_write_reg(value, sta_ddr_setting_table[idx].addr);
		TRACE_NOTICE("Register %-20s set to 0x%08x @0x%08x\n",
			     label,
			     value,
			     sta_ddr_setting_table[idx].addr);
	}
}

void  sta_ddr_get_setting(char *label, uint32_t *value)
{
	int idx = sta_ddr_find_idx_setting(label);

	if (idx < 0) {
		TRACE_NOTICE("Label %s unknown\n");
		return;
	}

	if (value) {
		*value = read_reg(sta_ddr_setting_table[idx].addr);

		TRACE_NOTICE("Register %-20s\n\tAddresse:0x%08x\n\t",
			     "setting value:0x%08x\n\tcurrent value: 0x%08x\n",
			     label,
			     sta_ddr_setting_table[idx].addr,
			     sta_ddr_setting_table[idx].value,
			     *value);
	} else {
		TRACE_NOTICE("Register %-20s\n\tAddresse:0x%08x\n\t",
			     "setting value:0x%08x\n\tcurrent value: 0x%08x\n",
			     label,
			     sta_ddr_setting_table[idx].addr,
			     sta_ddr_setting_table[idx].value,
			     read_reg(sta_ddr_setting_table[idx].addr));
	}
}

#endif
