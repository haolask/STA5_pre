/**
 * @file sta_m3_irq.c
 * @brief M3 irq firmware functions
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include <errno.h>

#include "utils.h"

#include "sta_map.h"
#include "sta_m3_irq.h"
#include "sta_hsem.h"
#include "sta_lib.h"
#include "sta_sga.h"
#include "sta_dma.h"
#include "sta_i2c_service.h"
#include "sta_spi_service.h"
#include "sta_vip.h"
#include "sta_platform.h"

#define M3_EXT_IRQ_LINES            16
#define M3_IRQ_LINES_PER_GROUP      4
#define AP_IRQ_LINES_PER_GROUP      32
#define M3_EXT_IRQ_VECTOR_OFFSET    0x00000020

#define M3IRQ_ALL_INTERRUPTS	0xFFFFFFFF

#define M3IRQ_BASE(i)			M3IRQ0_BASE + (i * 0x1000)
#define M3IRQ_MAX_CONTROLLER	4

extern uint32_t _vectors;

static const struct m3irq_line sta1x95_ap_m3_irq_matrix[M3_EXT_IRQ_LINES] = {

	{0, NULL},				/* EXT_IRQ0 - BANK 0 */
	{_0_HSEM_IRQ_B, hsem_irq_handler},	/* EXT_IRQ1 */
	{_0_SGA, sga_irq_handler},		/* EXT_IRQ2 */
	{0, NULL},				/* EXT_IRQ3 */
	{_1_DMA0, dma0_irq_handler},		/* EXT_IRQ4 - BANK 1 */
	{0, NULL},				/* EXT_IRQ5 */
	{0, NULL},				/* EXT_IRQ6 */
	{_1_I2C1, I2C1_IRQHandler},		/* EXT_IRQ7 */
	{0, NULL},				/* EXT_IRQ8 - BANK 2 */
	{_2_VIP_FRM, vip_frame},		/* EXT_IRQ9 */
	{_2_SSP2, SPI2_IRQHandler},		/* EXT_IRQ10 */
	{_2_I2C2, I2C2_IRQHandler},		/* EXT_IRQ11 */
	{0, NULL},				/* EXT_IRQ12 - BANK 3 */
	{0, NULL},				/* EXT_IRQ13 */
	{0, NULL},				/* EXT_IRQ14 */
	{0, NULL},				/* EXT_IRQ15 */
};

static const struct m3irq_line sta1385_ap_m3_irq_matrix[M3_EXT_IRQ_LINES] = {

	{0, NULL},				/* EXT_IRQ0 - BANK 0 */
	{_0_HSEM_IRQ_B, hsem_irq_handler},	/* EXT_IRQ1 */
	{0, NULL},				/* EXT_IRQ2 */
	{0, NULL},				/* EXT_IRQ3 */
	{0, NULL},				/* EXT_IRQ4 - BANK 1 */
	{0, NULL},				/* EXT_IRQ5 */
	{0, NULL},				/* EXT_IRQ6 */
	{0, NULL},				/* EXT_IRQ7 */
	{0, NULL},				/* EXT_IRQ8 - BANK 2 */
	{0, NULL},				/* EXT_IRQ9 */
	{0, NULL},				/* EXT_IRQ10 */
	{0, NULL},				/* EXT_IRQ11 */
	{0, NULL},				/* EXT_IRQ12 - BANK 3 */
	{0, NULL},				/* EXT_IRQ13 */
	{0, NULL},				/* EXT_IRQ14 */
	{0, NULL},				/* EXT_IRQ15 */
};

struct m3irq devs[M3IRQ_MAX_CONTROLLER];

/**
 * @brief	Returns M3 IRQ device according to the base address
 * @param	base: the base address
 * @return	the device (if found)
 */
static t_m3irq *m3irq_get_device(uint32_t base)
{
	switch(base) {
	case M3IRQ0_BASE:
		return m3irq0_regs;
	case M3IRQ1_BASE:
		return m3irq1_regs;
	case M3IRQ2_BASE:
		return m3irq2_regs;
	case M3IRQ3_BASE:
		return m3irq3_regs;
	default:
		return NULL;
	}
}

/**
 * @brief	Updates a mask of a given controller
 * @dev		The device
 * @i		The line index
 * @set		True to set, false to clear
 * @return	0 if no error, not 0 otherwise
 */
static int __m3irq_update_mask(struct m3irq *dev, int i, bool set)
{
	t_m3irq *m3irq;

	if (!dev)
		return -ENODEV;

	if (i > AP_IRQ_LINES_PER_GROUP)
		return -EINVAL;

	m3irq = m3irq_get_device(dev->base);
	if (!m3irq)
		return -ENODEV;

	if (set)
		m3irq->masks |= BIT(i);
	else
		m3irq->maskc |= BIT(i);

	return 0;
}

/**
 * @brief	Clears a bit from the mask of a given controller
 * @dev		The device
 * @i		The line index
 * @return	0 if no error, not 0 otherwise
 */
int m3irq_clear_mask(struct m3irq *dev, int i)
{
	return __m3irq_update_mask(dev, i, false);
}

/**
 * @brief	Sets a bit from the mask of a given controller
 * @dev		The device
 * @i		The line index
 * @return	0 if no error, not 0 otherwise
 */
int m3irq_set_mask(struct m3irq *dev, int i)
{
	return __m3irq_update_mask(dev, i, true);
}

/**
 * @brief	Clears all masks of a given controller
 * @dev		The device
 * @return	0 if no error, not 0 otherwise
 */
int m3irq_clear_all_masks(struct m3irq *dev)
{
	int err = 0;
	t_m3irq *m3irq;

	if (!dev)
		return -ENODEV;

	m3irq = m3irq_get_device(dev->base);
	if (!m3irq)
		return -ENODEV;

	m3irq->maskc = 0xFFFFFFFF;
	return err;
}

/**
 * @brief	Sets all masks of a given controller
 * @dev		The device
 * @return	0 if no error, not 0 otherwise
 */
int m3irq_set_all_masks(struct m3irq *dev)
{
	int err = 0;
	t_m3irq *m3irq;

	if (!dev)
		return -ENODEV;

	m3irq = m3irq_get_device(dev->base);
	if (!m3irq)
		return -ENODEV;

	m3irq->masks = 0xFFFFFFFF;
	return err;
}

/**
 * @brief	Provides an M3 IRQ controller according to its index
 * @i		The device index (from 0 to M3IRQ_MAX_CONTROLLER)
 * @return	The corresponding device if found, NULL otherwise
 */
struct m3irq *m3irq_get_device_at_index(int i)
{
	if (i >= M3IRQ_MAX_CONTROLLER)
		return NULL;

	return &devs[i];
}

/**
 * @brief	Initializes a given M3 IRQ controller
 * @dev		The device to initialize
 * @id		The device id (from 0 to M3IRQ_MAX_CONTROLLER)
 * @return	0 if no error, not 0 otherwise
 */
static int m3irq_init_dev(struct m3irq *dev, int id)
{
	if (id >= M3IRQ_MAX_CONTROLLER)
		return -EINVAL;

	dev->id = id;
	dev->base = M3IRQ_BASE(id);

	return 0;
}

/**
 * @brief	Registers an IRQ handler at a given index
 * @index	The line index
 * @irq_handler		The callback (IRQ handler) to be registered
 */
static void m3irq_register_irqhandler(int index, void (*irq_handler)(void))
{
	*((uint32_t *) &_vectors + M3_EXT_IRQ_VECTOR_OFFSET + index)
		= (uint32_t)irq_handler;
}

/**
 * @brief	Initializes the m3 irq controllers
 * @return	0 if no error, not 0 otherwise
 */
int m3irq_init(void)
{
	int i = 0;
	uint8_t ap_rel_pos = 0;
	uint8_t m3_rel_pos = 0;
	uint32_t m3irq_base;
	struct m3irq *dev;
	t_m3irq *m3irq_dev;
	const struct m3irq_line *ap_m3_irq_matrix;

	switch (get_soc_id()) {
	case SOCID_STA1195:
	case SOCID_STA1295:
	case SOCID_STA1275:
		ap_m3_irq_matrix = sta1x95_ap_m3_irq_matrix;
		break;
	case SOCID_STA1385:
		ap_m3_irq_matrix = sta1385_ap_m3_irq_matrix;
		break;
	default:
		return -EINVAL;
	}

	/* initialize each controller and set all masks (mask IRQ) */
	for (i = 0; i < M3IRQ_MAX_CONTROLLER; i++) {
		dev = &devs[i];
		m3irq_init_dev(dev, i);
		m3irq_set_all_masks(dev);
	}

	for (i = 0; i < M3_EXT_IRQ_LINES; i++) {

		/* Only configure interrupt with a valid call-back */
		if (!ap_m3_irq_matrix[i].irq_handler)
			continue;

		/* Calculate relative inside the AP forwardable IRQ bank (0 - 31) */
		ap_rel_pos = ap_m3_irq_matrix[i].id % AP_IRQ_LINES_PER_GROUP;

		/* Calculate relative inside the M3 EXT IRQ bank (0 - 3) */
		m3_rel_pos = i % M3_IRQ_LINES_PER_GROUP;

		/* Calculate current controller, depending on the offset */
		m3irq_base = M3IRQ_BASE(ap_m3_irq_matrix[i].id / AP_IRQ_LINES_PER_GROUP);

		/* Clear the IRQ Mask */
		m3irq_dev = m3irq_get_device(m3irq_base);
		if (!m3irq_dev)
			return -ENODEV;

		m3irq_dev->maskc |= BIT(ap_rel_pos);

		/* Forward the interrupt to the appropriate M3 line */
		if (ap_rel_pos < (AP_IRQ_LINES_PER_GROUP/2) )
			m3irq_dev->irq_low |= (m3_rel_pos << (ap_rel_pos * 2));
		else
			m3irq_dev->irq_up |= (m3_rel_pos << ((ap_rel_pos - (AP_IRQ_LINES_PER_GROUP/2)) * 2));

		/*
		 * Register the corresponding IRQ handler,
		 * starting from INTISR[16] vector at 0x80.
		 */
		m3irq_register_irqhandler(i, ap_m3_irq_matrix[i].irq_handler);
	}

	return 0;
}
