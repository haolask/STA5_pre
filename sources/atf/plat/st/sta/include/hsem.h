/*
 * (C) Copyright 2017 ST-microlectronics ADG
 *
 */

#ifndef __ASM_ARCH_HSEM_H
#define __ASM_ARCH_HSEM_H

/*
 * These functions are called to lock/unlock an hw semaphore
 * for shared resources access with M3 CPU
 */

/* HSEM interrupt mode argument */
#define HSEM_NOINT		0x0	/* No interrupt */
#define HSEM_INTAP		0x1	/* IRQ_A to AP */
#define HSEM_INTM3		0x2	/* IRQ_B to M3 */

/* Predefined static HSEM ID definition shared with AP */
enum {
	HSEM_FREE_MAX_ID = 11,
	HSEM_SQI_NOR_M3_ID = HSEM_FREE_MAX_ID,      /* M3 SQI NOR Access */
	HSEM_SQI_NOR_AP_ID,      /* AP SQI NOR Access */
	HSEM_NAND_ID,            /* AP-M3 NAND shared resource */
	HSEM_MMC_ID = HSEM_NAND_ID,  /* AP-M3 MMC shared resource */
	HSEM_CONSOLE_ID,	     /* UART Console shared access */
};

int hsem_trylock(int hsem, int irqmode);
void hsem_unlock(int hsem);
int hsem_irq_status(int hsem, int irq);

#endif /* __ASM_ARCH_HSEM_H */
