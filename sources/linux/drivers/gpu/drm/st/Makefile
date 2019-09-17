ccflags-y := -Iinclude/drm
ccflags-y += -Idrivers/rpmsg

st-drm-y := \
	drv.o \
	ltdc.o

st-drm-$(CONFIG_DRM_ST_LTDC_HDMI) += st_hdmi.o

obj-$(CONFIG_DRM_ST_LTDC) += st-drm.o

