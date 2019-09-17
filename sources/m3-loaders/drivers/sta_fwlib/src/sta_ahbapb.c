/**
 * @file sta_ahbapb.c
 * @brief AHB-APB bridge functions
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include "utils.h"
#include "sta_map.h"

#include "sta_ahbapb.h"
#include "sta_platform.h"

/* AHB-APB registers */
#define AHBAPB_BSR_OFFSET	0x00
#define AHBAPB_PAER_OFFSET	0x08
#define AHBAPB_PWAC_OFFSET	0x20
#define AHBAPB_PRAC_OFFSET	0x40
#define AHBAPB_PCG_OFFSET	0x60
#define AHBAPB_PUR_OFFSET	0x80
#define AHBAPB_EMUPCG_OFFSET	0xA0

/* AHB-APB Bridge 1 and 2 Status Register (BSR) */
#define BSR_ERROR		BIT(0)
#define BSR_OUTM		BIT(4)
#define BSR_PDA			BIT(6)
#define BSR_PCO			BIT(7)
#define BSR_PUR			BIT(8)

/* AHB-APB Bridge 1 and 2 Control Register (BCR) */
#define BCR_ERREN		BIT(8)
#define BCR_ISOF		BIT(12)
#define BCR_SPLITEN		BIT(24)
#define BCR_TOUTCNT(x)		((x) & 0x1f)		/* bit[0..4] */
#define BCR_SPLIT_CNT(x)	(((x) & 0x1f) << 16)	/* bit[16..20] */

/* AHB-APB Bridge 1 and 2 Peripherals Address Error Register (PAER) */
#define PAER_PERI_ADDR(x)	((x) & 0xffffff)
#define PAER_RW			BIT(24)

/*
 * AHB-APB Bridge 1 bits definitions applicable for:
 * - Peripheral Read/Write Access Control Register (PRAC/PWAC)
 * - Peripheral Clock Gating Register (PCG)
 * - Peripheral Under Reset Register (PUR)
 */
#define P0_APB_BRIDGE		BIT(0)
#define P0_M3_GPIO		BIT(1)
#define P0_M3_CAN		BIT(2)
#define P0_M3_MTU		BIT(3)
#define P0_M3_SRC		BIT(4)
#define P0_M3_WDG		BIT(5)
#define P0_CSS_GLUE		BIT(6)
#define P0_M3_IRQ		BIT(7)

/*
 * AHB-APB Bridge 2 bits definitions applicable for:
 * - Peripheral Read/Write Access Control Register (PRAC/PWAC)
 * - Peripheral Clock Gating Register (PCG)
 * - Peripheral Under Reset Register (PUR)
 */
#define P1_APB_BRIDGE		BIT(0)
#define P1_PMU			BIT(1)
#define P1_M3_WDG		BIT(2)

/**
  * @brief  Initialize AHB-APB bridges 1 and 2.
  * @param  None
  * @retval None
  */
void ahbapb_init(void)
{
	uint32_t val1, val2;

	if ((get_soc_id() == SOCID_STA1385) &&
		    (get_cut_rev() >= CUT_20)) {
		/* AHB-APB bridge 1 */
		val1 = P0_APB_BRIDGE | P0_M3_GPIO | P0_M3_CAN | P0_M3_MTU | P0_M3_SRC
		       | P0_CSS_GLUE | P0_M3_IRQ;

		/* AHB-APB bridge 2 */
		val2 = P1_APB_BRIDGE | P1_PMU | P1_M3_WDG;
	} else {
		/* AHB-APB bridge 1 */
		val1 = P0_APB_BRIDGE | P0_M3_GPIO | P0_M3_CAN | P0_M3_MTU | P0_M3_SRC
			| P0_M3_WDG | P0_CSS_GLUE | P0_M3_IRQ;

		/* AHB-APB bridge 2 */
		val2 = P1_APB_BRIDGE | P1_PMU;
	}

	write_reg(val1, AHBAPB_BRIDGE0_BASE + AHBAPB_PCG_OFFSET);
	write_reg(val1, AHBAPB_BRIDGE0_BASE + AHBAPB_PUR_OFFSET);
	write_reg(val1, AHBAPB_BRIDGE0_BASE + AHBAPB_PWAC_OFFSET);
	write_reg(val1, AHBAPB_BRIDGE0_BASE + AHBAPB_PRAC_OFFSET);
	write_reg(val2, AHBAPB_BRIDGE1_BASE + AHBAPB_PCG_OFFSET);
	write_reg(val2, AHBAPB_BRIDGE1_BASE + AHBAPB_PUR_OFFSET);
	write_reg(val2, AHBAPB_BRIDGE1_BASE + AHBAPB_PWAC_OFFSET);
	write_reg(val2, AHBAPB_BRIDGE1_BASE + AHBAPB_PRAC_OFFSET);
}

