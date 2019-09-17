/**
 * @file sta_ccc_plat.h
 * @brief CCC driver platform configuration header.
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: ADG-MID team
 */

#ifndef _CCC_PLAT_H_
#define _CCC_PLAT_H_

#include "sta_ccc_types.h"

/* These platform configurations might be an argument of ccc_init() func. */
#define C3 0x49000000
#define C3APB4 0x49200000
#define C3_IRQ_ID C3_IRQChannel
#define C3_CLOCK_MAX_FREQ 204800000

/*
 * Controller configuration.
 * Only first dispatcher (0) is available on the platform.
 */
#define NR_DISPATCHERS 1
#define EN_DISPATCHERS 1
#define PROGRAM_SIZE_IN_BYTES 512

/* TRNG configuration. */
#define TRNG_INDEX 0
#define TRNG_CLOCK_MIN_FREQ 2000000
enum trng_noise_source ccc_plat_get_trng_noise_source(void);
unsigned int ccc_plat_get_trng_charge_pump_current(void);
unsigned int ccc_plat_get_trng_pll0_reference_clock(void);

/* MPAES configuration. */
#define MPAES_INDEX 1
static inline unsigned short ccc_plat_get_mpaes_sp_key_slots(void)
{
	return 0x0003;
}

static inline unsigned short ccc_plat_get_mpaes_gp_key_slots(void)
{
	return 0xfffc;
}

/*
 * Define the maximum data size that can be handled along with MAX_CHUNK_SIZE.
 * Current implementation supports a maximum data size of 512 kB broken down
 * in the following way:
 * - Minimum chunk number is (8 * MAX_CHUNK_SIZE) + 1 = 9
 * - Due to the AES CCM mode, data can be split into header + payload
 *   independently handled. Hereafter is the worst case :
 *   Header ~256 KB requires 6 chunks
 *     |   |
 *     |   | 4 => Size / MAX_CHUNK_SIZE
 *     |---|
 *     |---| 1 => Size mod MAX_CHUNK_SIZE
 *     |---| 1 => (Size mod MAX_CHUNK_SIZE) mod AES_BLOCK_SIZE
 *
 *   Payload ~256 KB requires 6 chunks
 *     |   |
 *     |   | 4 => Size / MAX_CHUNK_SIZE
 *     |---|
 *     |---| 1 => Size mod MAX_CHUNK_SIZE
 *     |---| 1 => (Size mod MAX_CHUNK_SIZE) mod AES_BLOCK_SIZE
 */
#define AES_NR_CHUNKS 12

/* PKA configuration. */
#define PKA_INDEX 2
#define RSA_NR_CHUNKS 4
#define ECC_NR_CHUNKS 4

/* MOVE configuration. */
#define MOVE_INDEX 3
#define MOVE_NR_CHUNKS 12

/* UH configuration. */
#define NR_UH_CHANNELS 1
#define UH_INDEX 4
#define NR_UH2_CHANNELS 0
#define NR_HASH_CHANNELS (NR_UH_CHANNELS + NR_UH2_CHANNELS)
#define HASH_NR_CHUNKS 12
/* Software-configurable constant */
#define NR_HASH_CRYPTO_CONTEXTS_PER_CHANNEL 2

/* Topology of integrated channels. */
#define NR_INT_CHANNELS 5
static inline int ccc_plat_set_topology(enum ccc_channel_type *plat_topology,
					unsigned int nr)
{
	if (nr < NR_INT_CHANNELS)
		return -EFAULT;
	for (unsigned int i = 0; i < nr ; i++, plat_topology++)
		switch (i) {
		case TRNG_INDEX:
			*plat_topology = TRNG;
			break;
		case MPAES_INDEX:
			*plat_topology = MPAES;
			break;
		case PKA_INDEX:
			*plat_topology = PKA;
			break;
		case MOVE_INDEX:
			*plat_topology = MOVE;
			break;
		case UH_INDEX:
			*plat_topology = UH;
			break;
		default:
			*plat_topology = NONE;
			break;
		}
	return 0;
}

int ccc_plat_init(void);
void ccc_plat_deinit(void);

#define LITTLE_ENDIAN
#undef REJECT_UNALIGNED_DATA
/*
 * Assume that disabling HIF input bytes swapping and performing output
 * swapping by software in case of MOVE and HASH coupling is not required on
 * this platform.
 */
#undef DISABLE_HIF_IF_CH_EN_SWAP
#endif /* _CCC_PLAT_H_ */
