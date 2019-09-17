/*
 * Copyright (C) STMicroelectronics SA 2016
 *
 * Author: Yannick Fertre <yannick.fertre@st.com>
 *
 * License terms:  GNU General Public License (GPL), version 2
 */

#include <linux/kernel.h>

#include <drm/drmP.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_atomic.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_encoder_slave.h>

#include "drv.h"

static void hdmi_encoder_disable(struct drm_encoder *encoder)
{
	DRM_DEBUG_DRIVER("\n");
}

static void hdmi_encoder_enable(struct drm_encoder *encoder)
{
	DRM_DEBUG_DRIVER("\n");
}

static const struct drm_encoder_helper_funcs hdmi_encoder_helper_funcs = {
	.disable = hdmi_encoder_disable,
	.enable = hdmi_encoder_enable,
};

static void hdmi_encoder_cleanup(struct drm_encoder *encoder)
{
	DRM_DEBUG_DRIVER("\n");
	drm_encoder_cleanup(encoder);
}

static const struct drm_encoder_funcs hdmi_encoder_funcs = {
	.destroy = hdmi_encoder_cleanup,
};

struct drm_encoder *hdmi_encoder_create(struct drm_device *ddev,
					struct drm_bridge *bridge,
					unsigned int possible_crtcs)
{
	struct drm_encoder *encoder;
	int ret;

	encoder = devm_kzalloc(ddev->dev, sizeof(*encoder), GFP_KERNEL);
	if (!encoder)
		return NULL;

	encoder->possible_crtcs = possible_crtcs;
	encoder->possible_clones = 0; /* No cloning support */

	drm_encoder_init(ddev, encoder, &hdmi_encoder_funcs,
			 DRM_MODE_ENCODER_TMDS, NULL);

	drm_encoder_helper_add(encoder, &hdmi_encoder_helper_funcs);

	/* Link drm_bridge to encoder */
	bridge->encoder = encoder;
	encoder->bridge = bridge;

	ret = drm_bridge_attach(ddev, bridge);
	if (ret) {
		drm_encoder_cleanup(encoder);
		return NULL;
	}

	DRM_DEBUG_DRIVER("HDMI encoder:%d created\n", encoder->base.id);

	return encoder;
}

int hdmi_encoder_destroy(struct drm_device *ddev, struct drm_bridge *bridge)
{
	if (bridge && bridge->encoder) {
		hdmi_encoder_cleanup(bridge->encoder);
		return 0;
	}

	return -1;
}
