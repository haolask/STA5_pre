/**
 * @file sta_qos.h
 * @brief Quality Of Service header file
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */


#ifndef _STA_QOS_H_
#define _STA_QOS_H_

#define MAX_MASTER_BUS_PRIO		15

#define QOS_M0_BUS_ID_APPL_PROC		0x0000 /* AP: R4, A7... */
#define QOS_M1_BUS_ID_DMA0_M0		0x1000
#define QOS_M2_BUS_ID_DMA0_M1		0x2000
#define QOS_M3_BUS_ID_USB_M0		0x3000
#define QOS_M4_BUS_ID_USB_M1		0x4000
#define QOS_M5_BUS_ID_SGA		0x5000
#define QOS_M6_BUS_ID_CLCD		0x6000
#define QOS_M7_BUS_ID_CM3_CAN_SS	0xD000
#define QOS_M8_BUS_ID_DMA1_M0		0x7000
#define QOS_M9_BUS_ID_DMA1_M1		0x8000
#define QOS_M10_BUS_ID_VIDEO_DECODE	0x9000
#define QOS_M11_BUS_ID_GFX		0xA000
#define QOS_M12_BUS_ID_ETHERNET_AVB	0xB000
#define QOS_M13_BUS_ID_C3_EHSM		0xC000
#define QOS_M14_BUS_ID_SDIO		0xE000
#define QOS_M15_BUS_ID_FLEXRAY		0xF000
#define QOS_M16_BUS_ID_ETHERNET1	0x5000
#define QOS_M17_BUS_ID_SDIO		0x6000
#define QOS_M18_BUS_ID_CM3_CACHE_PORT	0x9000

struct qos_tab_t {
	uint16_t qos_bus_id;	/**< qos bus id */
	uint16_t prio;		/**< priority */
	uint8_t soc;		/**< SoC applicability of the QoS */
};

/* sets the read bus priority to the given value */
int set_master_priority_bus_read(uint32_t bus_id, unsigned int prio);

/* sets the write bus priority to the given value */
int set_master_priority_bus_write(uint32_t bus_id, unsigned int prio);

/* sets the r/w bus priority to the given value */
int set_master_priority_bus_rw(uint32_t bus_id, unsigned int prio);

/* initializes QoS by setting priorities for each Master on Bus */
int qos_init(void);

#endif /* _STA_QOS_H_ */
