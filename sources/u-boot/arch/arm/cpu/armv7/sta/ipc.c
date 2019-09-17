/*
 *
 * (C) Copyright 2016 ST-microlectronics
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/ipc.h>
#include <asm/arch/hsem.h>

/*
 * The IPC relies on HSEM device to generate interrupt to remote processor
 * and a buffer mapped in shared memory.
 */
#define MBOX_HSEM_TYPE

/* Mailbox channel 15 is reserved by default */
#define IPC_TX_OFFSET		0x1F0
#define IPC_TX_SIZE		0x10

#if defined(MBOX_HSEM_TYPE)
#define HSEM_INTM3  0x2	/* IRQ_B to M3 */
#define HSEM_IPC_ID 15
#else
#define MBOX_IPSR   0x04
#define MBOX_IMR    0x08
#define MBOX_IPC_ID 15
#endif

/* IPC mailbox shared buffer configuration */
struct ipc_config {
	char		*name;
	u32		tx_offset;
	u32		tx_size;
};

static const struct ipc_config ipc = {
	.name		= "mbox15",
	.tx_offset	= IPC_TX_OFFSET,
	.tx_size	= IPC_TX_SIZE
};


int ipc_send_data(void *data, u32 size)
{
	u8 *buf, *wrb;
	u32 len;
	int j;

	/* Check previous message has already been handled by M3 */
#if defined(MBOX_HSEM_TYPE)
	if (hsem_irq_status(HSEM_IPC_ID, HSEM_INTM3))
#else
	if (readl(STA_MBOX_BASE + MBOX_IPSR) & BIT(MBOX_IPC_ID))
#endif
		return -1;

	/* Write data in shared buffer */
	buf = (u8 *)(DDRAM_APP_OS_MAILBOXES_BASE + ipc.tx_offset);
	/* Payload size */
	/* Size (1 byte max) stored in the 1st byte of the buffer */
	len = min(size, (ipc.tx_size - sizeof(*buf)));
	writeb((u8)len, (void __iomem *)buf);

	/* Payload data */
	for (j = 0, wrb = data, buf++; wrb && j < len; j++, wrb++, buf++)
		writeb(*wrb, (void __iomem *)buf);

#if defined(MBOX_HSEM_TYPE)
	/* Generate interrupt on the selected mailbox channel */
	while (!hsem_trylock(HSEM_IPC_ID, HSEM_INTM3))
		;
	hsem_unlock(HSEM_IPC_ID);
#else
	writel(BIT(MBOX_IPC_ID), STA_MBOX_BASE + MBOX_IMR);
	writel(BIT(MBOX_IPC_ID), STA_MBOX_BASE);
	while (readl(STA_MBOX_BASE + MBOX_IPSR) & BIT(MBOX_IPC_ID))
		;
#endif

	return 0;
}

