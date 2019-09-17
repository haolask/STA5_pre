/**
 * @file sta_qos.c
 * @brief Quality Of Service functions
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */
#include <errno.h>

#include "utils.h"
#include "sta_map.h"
#include "sta_common.h"
#include "sta_nic_security.h"
#include "sta_qos.h"

#define READ_QOS_OFFSET			0x100
#define WRITE_QOS_OFFSET		0x104

/**
 * @struct qos_tab_t
 * @brief QOS bus priority changes table
 */
static const struct qos_tab_t qos_bus_prio_tab[] = {
	/* QoS Master bus ID,        Prio,    SoC,     Prio reset value */
	{ QOS_M0_BUS_ID_APPL_PROC,	0,  SOC_ALL },	/* 14 */
	{ QOS_M1_BUS_ID_DMA0_M0,	13, SOC_ALL },  /* 0  */
	{ QOS_M2_BUS_ID_DMA0_M1,	13, SOC_ALL },	/* 1  */
	{ QOS_M3_BUS_ID_USB_M0,		2,  SOC_ALL },	/* 2  */
	{ QOS_M4_BUS_ID_USB_M1,		3,  SOC_ALL },	/* 3  */
	{ QOS_M5_BUS_ID_SGA,		12, SOC_A5 },	/* 4  */
	{ QOS_M6_BUS_ID_CLCD,		11, SOC_A5 },	/* 5  */
	{ QOS_M7_BUS_ID_CM3_CAN_SS,	15, SOC_ALL },	/* 15 (Highest) */
	{ QOS_M8_BUS_ID_DMA1_M0,	13, SOC_ALL },	/* 6  */
	{ QOS_M9_BUS_ID_DMA1_M1,	13, SOC_ALL },	/* 7  */
	{ QOS_M10_BUS_ID_VIDEO_DECODE,	9,  SOC_A5 },	/* 12 */
	{ QOS_M11_BUS_ID_GFX,		10, SOC_A5 },	/* 11 */
	{ QOS_M12_BUS_ID_ETHERNET_AVB,	10, SOC_ALL },	/* 10 */
	{ QOS_M13_BUS_ID_C3_EHSM,	8,  SOC_ALL },	/* 8  */
	{ QOS_M14_BUS_ID_SDIO,		7,  SOC_A5 },	/* 12 */
	{ QOS_M15_BUS_ID_FLEXRAY,	12, SOC_TC3P }, /* 12 */
	{ QOS_M16_BUS_ID_ETHERNET1,	4,  SOC_TC3P }, /* 4  */
	{ QOS_M17_BUS_ID_SDIO,		5,  SOC_TC3P },	/* 5  */
	{ QOS_M18_BUS_ID_CM3_CACHE_PORT, 12, SOC_TC3P },/* 12 */
};

/**
 * @brief  sets the read bus priority to the given value.
 * @param  bus_id: bus ID to be set
 * @param  prio: the priority on bus
 * @retval 0 if no error, not 0 otherwise
 */
int set_master_priority_bus_read(uint32_t bus_id, unsigned int prio)
{
	if (prio > MAX_MASTER_BUS_PRIO)
		return -EINVAL;

	write_reg(prio, QOS_BASE + READ_QOS_OFFSET + bus_id);
	return 0;
}

/**
 * @brief  sets the write bus priority to the given value.
 * @param  bus_id: bus ID to be set
 * @param  prio: the priority on bus
 * @retval 0 if no error, not 0 otherwise
 */
int set_master_priority_bus_write(uint32_t bus_id, unsigned int prio)
{
	if (prio > MAX_MASTER_BUS_PRIO)
		return -EINVAL;

	write_reg(prio, QOS_BASE + WRITE_QOS_OFFSET + bus_id);
	return 0;
}

/**
 * @brief  sets the read/write bus priority to the given value.
 * @param  bus_id: bus ID to be set
 * @param  prio: the priority on bus
 * @retval 0 if no error, not 0 otherwise
 */
int set_master_priority_bus_rw(uint32_t bus_id, unsigned int prio)
{
	int ret;

	ret = set_master_priority_bus_read(bus_id, prio);
	if (ret)
		return ret;

	ret = set_master_priority_bus_write(bus_id, prio);

	return ret;
}

/**
 * @brief	initializes QoS by setting priorities for each Master on Bus
 * @return	0 if no error, not 0 otherwise
 */
int qos_init(void)
{
	const struct qos_tab_t *qos;
	int ret;
	unsigned int i;
	uint8_t soc = SOC_A5;

	if (get_soc_id() == SOCID_STA1385)
		soc = SOC_TC3P;

	for (i = 0; i < NELEMS(qos_bus_prio_tab); i++) {
		qos = &qos_bus_prio_tab[i];

		if (qos->soc & soc) {
			ret = set_master_priority_bus_rw(qos->qos_bus_id,
							 qos->prio);
			if (ret)
				return ret;
		}
	}
	return 0;
}
