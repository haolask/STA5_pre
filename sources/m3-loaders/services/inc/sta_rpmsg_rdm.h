/**
 * @file sta_rpmsg_rdm.h
 * @brief RPMsg Remote Device Manager
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#ifndef STA_RPMSG_RDM_H
#define STA_RPMSG_RDM_H

#ifndef MIN
#define MIN(a, b) (((a) > (b)) ? (b) : (a))
#endif

int rdm_relase_device(char *name);
void rdm_task(void *p);

#endif
