/*
 * Copyright (C) STMicroelectronics SA 2015
 * Authors: Philippe Cornu <philippe.cornu@st.com>
 *          Yannick Fertre <yannick.fertre@st.com>
 * License terms:  GNU General Public License (GPL), version 2
 */

#ifndef _ST_LTDC_H_
#define _ST_LTDC_H_

#include <drm/drmP.h>

int ltdc_crtc_enable_vblank(struct drm_device *dev, unsigned int crtc);
void ltdc_crtc_disable_vblank(struct drm_device *dev, unsigned int crtc);

int ltdc_mm_show(struct seq_file *m, void *arg);
int ltdc_fps_show(struct seq_file *m, void *arg);
int ltdc_dbg_show_regs(struct seq_file *m, void *arg);
int ltdc_dbg_show_and_reset_underrun(struct seq_file *m, void *arg);


int ltdc_drm_fps_filter_get(void *data, u64 *val);
int ltdc_drm_fps_filter_set(void *data, u64 val);

#endif
