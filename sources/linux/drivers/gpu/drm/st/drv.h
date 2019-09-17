/*
 * Copyright (C) STMicroelectronics SA 2015
 * Authors: Philippe Cornu <philippe.cornu@st.com>
 *          Yannick Fertre <yannick.fertre@st.com>
 * License terms:  GNU General Public License (GPL), version 2
 */

#ifndef _ST_DRV_H_
#define _ST_DRV_H_

#include <drm/drmP.h>

/**
 * ST drm private structure
 * This structure is stored as private in the drm_device
 *
 * @ltdc:                  ltdc ip
 * @drm_dev:               drm device
 * @fbdev_cma:             drm cma framebuffer device
 * @drm_file: drm file private data
 */
struct st_private {
	struct ltdc *ltdc;
	struct drm_device *drm_dev;
	struct drm_fbdev_cma *fbdev_cma;
	struct drm_file *filp;
};

extern struct platform_driver ltdc_driver;

#endif

