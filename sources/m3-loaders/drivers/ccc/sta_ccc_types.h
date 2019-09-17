/**
 * @file sta_ccc_types.h
 * @brief Defines types that cannot be defined in sta_ccc.h
 * Must NOT be included outside CCC driver.
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: ADG-MID team
 */

#ifndef _CCC_TYPES_H_
#define _CCC_TYPES_H_

enum ccc_channel_type {
	TRNG = 0,
	MPAES,
	PKA,
	MOVE,
	HASH,
	UH = HASH,
	UH2 = HASH,
	NR_CHANNEL_TYPES,
	NONE
};

enum trng_noise_source {
	RING_OSCILLATOR,
	PLL
};
#endif /* _CCC_TYPES_H_ */
