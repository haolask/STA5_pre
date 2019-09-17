/**
 * @file sta_ddr.h
 * @brief This is the header file for DDR
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#ifndef __STA_DDR_H__
#define __STA_DDR_H__

#include "sta_type.h"
#include "sta_mem_map.h"
#include "trace.h"

/* uMCTL2 */

#define MSTR 0x0
#define STAT 0x4
#define MRCTRL0 0x10
#define MRCTRL1 0x14
#define MRSTAT 0x18
#define DERATEEN 0x20
#define DERATEINT 0x24
#define PWRCTL 0x30
#define PWRTMG 0x34
#define HWLPCTL 0x38
#define RFSHCTL0 0x50
#define RFSHCTL1 0x54
#define RFSHCTL3 0x60
#define RFSHTMG 0x64
#define CRCPARCTL0 0xc0
#define CRCPARSTAT 0xcc
#define INIT(x) 0xd0 + (4 * x)
#define DIMMCTL 0xf0
#define RANKCTL 0xf4
#define DRAMTMG(x) 0x100 + (4 * x)
#define ZQCTL0 0x180
#define ZQCTL1 0x184
#define ZQCTL2 0x188
#define DFITMG0 0x190
#define DFITMG1 0x194
#define DFILPCFG0 0x198
#define DFIUPD0 0x1a0
#define DFIUPD1 0x1a4
#define DFIUPD2 0x1a8
#define DFIMISC 0x1b0
#define ADDRMAP(x) 0x200 + (4 * x)
#define ODTCFG 0x240
#define ODTMAP 0x244
#define SCHED 0x250
#define SCHED1 0x254
#define PERFHPR1 0x25c
#define PERFLPR1 0x264
#define PERFWR1 0x26c
#define PERFVPR1 0x274
#define PERFVPW1 0x278
#define DBG0 0x300
#define DBG1 0x304
#define DBGCAM 0x308
#define DBGCMD 0x30c
#define DBGSTAT 0x310
#define SWCTL 0x320
#define SWSTAT 0x324
#define POISONCFG 0x36c
#define POISONSTAT 0x370
#define PSTAT 0x3Fc
#define PCCFG 0x400

#define PCFGR(x) 0x404 + (0xb0 * (x & 0xf))
#define PCFGW(x) 0x408 + (0xb0 * (x & 0xf))
#define PCTRL(x) 0x490 + (0xb0 * (x & 0xf))
#define PCFGQOS0(x) 0x494 + (0xb0 * (x & 0xf))
#define PCFGQOS1(x) 0x498 + (0xb0 * (x & 0xf))
#define PCFGWQOS0(x) 0x49c + (0xb0 * (x & 0xf))
#define PCFGWQOS1(x) 0x4a0 + (0xb0 * (x & 0xf))

/* STAT register */
#define STAT_SELFREF_TYPE_MASK 0x3
#define STAT_SELFREF_TYPE_OFFSET 4
#define STAT_OPMODE_MASK 0x3
#define STAT_OPMODE_INIT		0
#define STAT_OPMODE_NORMAL		1
#define STAT_OPMODE_POWERDOWN	2
#define STAT_OPMODE_SELFREF		3

/* PWRCTL register */
#define SELF_REFRESH_EN BIT(0)
#define POWERDOWN_EN BIT(1)
#define EN_DFI_DRAM_CLK_DIS BIT(3)

/* RFSHCTL3 register */
#define DIS_AUTO_REFRESH BIT(0)

#define DDR_BOOT_REASON_COLD_BOOT		0
#define DDR_BOOT_REASON_EXIT_SELF_REF	1

#define DDR3_MAX_PORT 4

#define SKIP_DRAM_INIT_NORMAL	BIT(30)
#define SKIP_DRAM_INIT_SELFREF	BIT(30) | BIT(31)

#define DDR3_ALL_PORT_ENABLED_MASK	0x000F000F
#define DDR3_ALL_PORT_DISABLED_MASK 0x00000000

#ifdef DDR_IP_LOG
#define DDR_SYST_INIT_TIMEOUT_US (1000 * 100)
static inline void ddr_write_reg(uint32_t value, uint32_t addr)
{
	*(volatile uint32_t *)addr = value;
	TRACE_NOTICE("W @0x%x:0x%08x\n", addr, value);
}

static inline uint32_t ddr_read_reg(uint32_t addr)
{
	TRACE_NOTICE("R @0x%x:0x%08x\n", (addr), *(volatile uint32_t *)addr);
	return *(volatile uint32_t *)addr;
}
#else
#define DDR_SYST_INIT_TIMEOUT_US 1000
#define ddr_write_reg write_reg
#define ddr_read_reg read_reg
#endif

#define enable_ddr_port(port) ddr_write_reg(BIT(0), DDR3_CTRL_BASE + PCTRL(port))
#define disable_ddr_port(port) ddr_write_reg(~BIT(0), DDR3_CTRL_BASE + PCTRL(port))

#define poll_reg(addr, exp) do {} while ((read_reg(addr) & exp) != exp)

/* PUB */

#define PUB_RIDR		0x000

/* PIR offset is the same for both PUB and PUBL */
#define PUB_PIR 0x004

#define PUB_PGSR0 0x010
#define PUBL_PGSR 0x00c

/* ZQ0CRn offset is the same for both PUB and PUBL */
#define PUB_ZQCR0(i) 0x180 + (4 * i)

#define PGSR0_IDONE	BIT(0)

#define DDR3_PUB_ID		0x0014A315
#define DDR3_PUBL_ID	0x00200200

#define PUB_FULL_TRAINING 0x0CA5CADE

#define ZQCR0_ZDEN BIT(28)
#define ZDATA_MASK (0x0FFFFFFF)

#define ZDATA_STORAGE (BACKUP_RAM_DDR_ZQ_CAL_BASE)

#define TRAINING_DATA_LOCATION (DDRAM_BASE)
#ifdef DDRAM_DDR_TRAINING_BASE
#define TRAINING_DATA_STORAGE (DDRAM_DDR_TRAINING_BASE)
#define TRAINING_DATA_SIZE (DDRAM_DDR_TRAINING_SIZE)
#else
#define TRAINING_DATA_STORAGE (BACKUP_RAM_DDR_TRAINING_BASE)
#define TRAINING_DATA_SIZE (BACKUP_RAM_DDR_TRAINING_SIZE)
#endif


#ifdef _SELF_REF_TEST_DDR_CHECKSUM
#define DDR_CHECKSUM_AREA_START	0xA0000000
#define DDR_CHECKSUM_AREA_SIZE	0x100000
#define CHECKSUM_STORAGE (BACKUP_RAM_DDR_CHECKSUM_BASE)
#endif

#define MAX_IDX 0xDEADDEAD

typedef struct {
	uint32_t value;
	uint32_t addr;
#ifdef VERBOSE_DDR
	char *label;
#endif
} t_sta_ddr_setting;

extern t_sta_ddr_setting sta_ddr_setting_table[];

#define DDRVAL(idx) sta_ddr_setting_table[idx ## _IDX].value
#define DDRADDR(idx) sta_ddr_setting_table[idx ## _IDX].addr
#ifdef VERBOSE_DDR
#define DDRLBL(idx) sta_ddr_setting_table[idx ## _IDX].label
#define DDRSETTING(val, addr, label) { val, addr, #label },
#else
#define DDRSETTING(val, addr, label) { val, addr },
#endif

int sta_ddr_init(uint32_t boot_reason);
void sta_ddr_axi_reset(void);
void sta_ddr_core_reset(void);

/* PUB(L) API */
int sta_ddr_pub_configure(uint32_t boot_reason);
int sta_ddr_pub_set_io_retention(bool set);
int sta_ddr_pub_do_training(uint32_t mask);

/* uMCTL2 API */

int sta_ddr_ctl_configure(uint32_t boot_reason);
int sta_ddr_ctl_power_up(uint32_t boot_reason);
int sta_ddr_suspend(void);
int sta_ddr_resume(void);
#ifdef _SELF_REF_TEST_DDR_CHECKSUM
void sta_ddr_std_check(bool resume_stage);
#else
#define sta_ddr_std_check(x) do {} while (0)
#endif

/* common API */
int ddr3_pub_do_pir(uint32_t pir_mask);
uint32_t ddr3_pub_has_errors(uint32_t shift, uint32_t mask);
void ddr3_pub_save_zq_cal(void);
void ddr3_pub_load_zq_cal(void);
void ddr3_save_data_written_by_training(void);
void ddr3_load_data_written_by_training(void);


/* helpers */
void sta_ddr_display_rate(void);
void sta_ddr_ctl_test_pattern(uint32_t max, bool display_temp, uint32_t pattern,
		uint32_t s, bool r, bool w, int type);

#ifdef VERBOSE_DDR
void sta_ddr_dump(void);
void sta_ddr_set_setting(char *label, uint32_t value);
void  sta_ddr_get_setting(char *label, uint32_t *value);
#else
#define sta_ddr_dump(x, ...) do {} while (0)
#define sta_ddr_set_setting(x, ...) do {} while (0)
#define  sta_ddr_get_setting(x,  ...) do {} while (0)
#endif

#endif /* __STA_DDR_H__ */

