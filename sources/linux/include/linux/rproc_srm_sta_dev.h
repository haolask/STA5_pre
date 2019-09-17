/*
 * rproc_srm_sta_dev.h
 *
 * Copyright (C) STMicroelectronics 2018
 * License terms:  GNU General Public License (GPL), version 2
 */

#ifndef RPROC_SRM_STA_DEV_H
#define RPROC_SRM_STA_DEV_H

int rproc_srm_sta_dev_enable(char *dev_name, bool state);
ssize_t rproc_rdm_get_devices(char *str, uint32_t len);
#endif
