/*
 * Copyright (C) STMicroelectronics SA 2016
 *
 * Author: Yannick Fertre <yannick.fertre@st.com>
 *
 * License terms:  GNU General Public License (GPL), version 2
 */

#ifndef _HDMI_ENCODER_H_
#define _HDMI_ENCODER_H_

struct drm_encoder *hdmi_encoder_create(struct drm_device *ddev,
					struct drm_bridge *bridge,
					unsigned int possible_crtcs);
int hdmi_encoder_destroy(struct drm_device *ddev, struct drm_bridge *bridge);
#endif
