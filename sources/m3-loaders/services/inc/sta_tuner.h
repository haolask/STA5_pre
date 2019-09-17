/**
 * @file sta_tuner.h
 * @brief Tuner service
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: ADG-MID team
 */

#if !defined STA_TUNER_H
#define STA_TUNER_H

void tuner_FWM_update(void *shadow_addr, uint32_t size);
void early_tuner_task(struct sta *context);

struct tuner_fwm_ty {
	    void *fwm_address;
	    uint32_t fwm_size;
};

#endif
