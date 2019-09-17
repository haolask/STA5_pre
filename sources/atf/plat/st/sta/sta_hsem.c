/*
 *
 * (C) Copyright 2016/2017 ST-microlectronics ADG
 *
 */

#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <mmio.h>
#include <utils.h>
#include <xlat_tables_v2.h>
#include <sta_private.h>

#include <hsem.h>

#undef DEBUG /* Define it for debug traces */

#if defined(DEBUG) || LOG_LEVEL >= LOG_LEVEL_VERBOSE
# define debug(...)	printf(__VA_ARGS__)
#else
# define debug(...)
#endif

/*
 * The HSEM device hosts 16 different HW semaphores, with 2 set of
 * registers for interrupt management towards M3 and AP (Cx-R4/A7) CPU.
 */
#define HSEM_MAX_SEMAPHORE	16	/* a total of 16 semaphores */

#define HSEM_RESET		(0)	/* free */
#define HSEM_MASTER_ID		0x01

#define	HSEM_RX			0x00	/* data register (X = 0, ... 15) */
#define	HSEM_ICRALL		0x90	/* Interrupt clear all */
#define	HSEM_IMSCA		0xA0	/* Interrupt A mask */
#define	HSEM_RISA		0xA4	/* Raw interrupt A status */
#define	HSEM_MISA		0xA8	/* Masked interrupt A status */
#define	HSEM_ICRA		0xAC	/* Interrupt A clear */
#define	HSEM_IMSCB		0xB0	/* Interrupt B mask */
#define	HSEM_RISB		0xB4	/* Raw interrupt B status */
#define	HSEM_MISB		0xB8	/* Masked interrupt B status */
#define	HSEM_ICRB		0xBC	/* Interrupt B clear */

#define HSEM_RESET_IRQS		((1 << HSEM_MAX_SEMAPHORE) - 1)
#define HSEM_INTSX(n)		((n) << 4)
#define HSEM_DATA(n)		(HSEM_RX + 4 * (n))

/* Default HSEM reset code value */
static	unsigned long hsem_reset[HSEM_MAX_SEMAPHORE];
static	unsigned long hsem_count[HSEM_MAX_SEMAPHORE];

mmap_reserve(MAP_HSEM);
int hsem_trylock(int hsem, int irqmode)
{
	unsigned long base;

	assert(hsem < HSEM_MAX_SEMAPHORE);
	assert(IN_MMAP(STA_HSEM_BASE, MAP_HSEM));

	base = STA_HSEM_BASE + HSEM_DATA(hsem);

	if (hsem_count[hsem] == 0) {
		/* Set HSEM reset code with INTSx bits value in order
		 * to generate an IRQ if requested */
		if (irqmode)
			hsem_reset[hsem] = HSEM_RESET | HSEM_INTSX(HSEM_INTM3);
		else
			hsem_reset[hsem] = HSEM_RESET;

		mmio_write_32(base, HSEM_MASTER_ID);

		/* get only first 4 bit and compare to masterID.
		 * if equal, we have the semaphore, otherwise
		 * someone else has it.
		 */
		if (HSEM_MASTER_ID != (0x0F & mmio_read_32(base)))
			return 0; /* HSEM not taken */
	}

	debug("lock: %d\n", hsem);

	return ++hsem_count[hsem]; /* HSEM taken */
}

void hsem_unlock(int hsem)
{
	assert(hsem < HSEM_MAX_SEMAPHORE);

	if (--hsem_count[hsem] == 0) {
		 /* Release the lock by writing 0 to it
		  * According to the INTSx bits value,
		  * an IRQ can also be generated
		  */
		mmio_write_32(STA_HSEM_BASE + HSEM_DATA(hsem), hsem_reset[hsem]);
		debug("unlock: %d\n", hsem);
	}
}

int hsem_irq_status(int hsem, int irq)
{
	assert(hsem < HSEM_MAX_SEMAPHORE);

	/* Read IRQ status register prior to masking */
	if (irq == HSEM_INTAP)
		return mmio_read_32(STA_HSEM_BASE + HSEM_RISA) & BIT(hsem);
	else /* irq == HSEM_INTM3 */
		return mmio_read_32(STA_HSEM_BASE + HSEM_RISB) & BIT(hsem);
}

