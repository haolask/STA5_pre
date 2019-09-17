/**
 * @file sta_pmu.h
 * @brief Provide all the sta PMU driver definitions.
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: Jean-Nicolas GRAUX <jean-nicolas.graux@st.com>
 */

#ifndef _STA_PMU_H_
#define _STA_PMU_H_

/**
 * Common definitions between back-end and front-end driver.
 * Content must be updated in case remote front-end driver definitions
 * are updated and vice-versa. Make sure that both match.
 */

#define	PMU_LVI BIT(0)
#define	PMU_ONOFF BIT(1)
#define	PMU_IGNKEY BIT(2)
#define	PMU_VDDOK BIT(3) /* STAND-BY source only */

#define PMU_WAKES (0xFF << 4)
#define PMU_WAKE0 BIT(4)
#define PMU_WAKE1 BIT(5)
#define PMU_WAKE2 BIT(6)
#define PMU_WAKE3 BIT(7)
#define PMU_WAKE4 BIT(8)
#define PMU_WAKE5 BIT(9)
#define PMU_WAKE6 BIT(10)
#define PMU_WAKE7 BIT(11)

#define	PMU_RTC BIT(12)
#define	PMU_SBY2NCTR_DONE BIT(19)
#define	PMU_PORLVD BIT(20)

#define PMU_WKUP_SRC_MASK (PMU_LVI|PMU_ONOFF|PMU_IGNKEY|PMU_WAKES)
#define PMU_SBY_SRC_MASK (PMU_LVI|PMU_ONOFF|PMU_IGNKEY|PMU_VDDOK)

#define PMU_WKUP_STAT_MASK (PMU_LVI|PMU_ONOFF|PMU_IGNKEY|PMU_WAKES|\
	PMU_RTC|PMU_PORLVD)

#define	PMU_N2SBYCTR_TO_MAX 3

/**
 * PMU API that can be used by any M3 xloader tasks & top drivers
 */

/* to enable a wake-up source */
void pmu_enable_wkup_src(uint32_t sel);
/* to disable a wake-up source */
void pmu_disable_wkup_src(uint32_t sel);
/* to select an wkup active edge as a rising edge */
void pmu_enable_wkup_edge(uint32_t sel);
/* to select an wkup active edge as a falling edge */
void pmu_disable_wkup_edge(uint32_t sel);
/* to enable a sby source */
void pmu_enable_sby_src(uint32_t sel);
/* to disable a sby source */
void pmu_disable_sby_src(uint32_t sel);
/* to select a sby active edge as a rising edge */
void pmu_enable_sby_edge(uint32_t sel);
/* to select a sby active edge as a falling edge */
void pmu_disable_sby_edge(uint32_t sel);
/* to enable a sby irq */
void pmu_enable_sby_irq(uint32_t sel);
/* to disable a sby irq */
void pmu_disable_sby_irq(uint32_t sel);
/* to get wake-up status */
uint32_t pmu_get_wkup_stat(void);
/* to get current wakeup sources */
uint32_t pmu_get_wkup_src(void);
/* to get current sby status */
uint32_t pmu_get_sby_stat(void);
/* to get current sby sources */
uint32_t pmu_get_sby_src(void);
/* to trigger a pmu goto stanbby request */
void pmu_goto_sby(void);
/* to store and clear standby status */
void pmu_clear_sby(void);
/* to initialize PMU device */
void pmu_init(void);
void pmu_free_ao_gpio(void);

#endif
