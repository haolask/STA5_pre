/*
 * (C) Copyright 2016 ST-microlectronics
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARCH_IPC_H
#define __ASM_ARCH_IPC_H

/*
 * This function is called to send a message to M3 remote processor
 */
int ipc_send_data(void *data, u32 size);

#endif /* __ASM_ARCH_IPC_H */
