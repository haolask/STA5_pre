/*
 * Copyright (C) STMicroelectronics SA 2015
 * Authors: Philippe Cornu <philippe.cornu@st.com>
 *          Yannick Fertre <yannick.fertre@st.com>
 * License terms:  GNU General Public License (GPL), version 2
 */

#include <linux/component.h>
#include <linux/debugfs.h>
#include <linux/of_platform.h>

#include <drm/drm_atomic.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_fb_cma_helper.h>
#include <drm/drm_gem_cma_helper.h>

#include "drv.h"
#include "ltdc.h"

#define DRIVER_NAME	"st"
#define DRIVER_DESC	"STMicroelectronics SoC DRM"
#define DRIVER_DATE	"FIXME"
#define DRIVER_MAJOR	1
#define DRIVER_MINOR	0

#define ST_MAX_FB_WIDTH		2048
#define ST_MAX_FB_HEIGHT	2048 /* same as width to handle orientation */

static void st_fb_output_poll_changed(struct drm_device *drm_dev)
{
	struct st_private *private = drm_dev->dev_private;

	dev_dbg(drm_dev->dev, "%s\n", __func__);
	drm_fbdev_cma_hotplug_event(private->fbdev_cma);
}

static struct drm_mode_config_funcs st_mode_config_funcs = {
	.fb_create = drm_fb_cma_create,
	.output_poll_changed = st_fb_output_poll_changed,
	.atomic_check = drm_atomic_helper_check,
	.atomic_commit = drm_atomic_helper_commit,
};

static void st_drm_atomic_commit_hw_done(struct drm_atomic_state *state)
{
	struct drm_pending_vblank_event *event;
	struct drm_device *drm = state->dev;
	int i;
	unsigned long flags;
	struct drm_crtc *crtc;
	struct drm_crtc_state *crtc_state;

	for_each_crtc_in_state(state, crtc, crtc_state, i) {
		event = crtc->state->event;
		if (event) {
			crtc->state->event = NULL;

			spin_lock_irqsave(&drm->event_lock, flags);
			if (drm_crtc_vblank_get(crtc) == 0)
				drm_crtc_arm_vblank_event(crtc, event);
			else
				drm_crtc_send_vblank_event(crtc, event);
			spin_unlock_irqrestore(&drm->event_lock, flags);
		}
	}

	/* backend must have consumed any event before */
	drm_atomic_helper_commit_hw_done(state);
}

/** Fork of drm_atomic_helper_commit_tail() which is not compatible with
 * runtime PM
 */
static void st_drm_atomic_commit_tail(struct drm_atomic_state *state)
{
	struct drm_device *dev = state->dev;

	drm_atomic_helper_commit_modeset_disables(dev, state);

	drm_atomic_helper_commit_modeset_enables(dev, state);

	drm_atomic_helper_commit_planes(dev, state,
					DRM_PLANE_COMMIT_ACTIVE_ONLY);

	st_drm_atomic_commit_hw_done(state);

	drm_atomic_helper_wait_for_vblanks(dev, state);

	drm_atomic_helper_cleanup_planes(dev, state);
}

static struct drm_mode_config_helper_funcs st_drm_mode_config_helpers = {
	.atomic_commit_tail = st_drm_atomic_commit_tail,
};

static void st_mode_config_init(struct drm_device *dev)
{
	dev_dbg(dev->dev, "%s\n", __func__);

	dev->mode_config.min_width = 0;
	dev->mode_config.min_height = 0;

	/*
	 * set max width and height as default value.
	 * this value would be used to check framebuffer size limitation
	 * at drm_mode_addfb().
	 */
	dev->mode_config.max_width = ST_MAX_FB_WIDTH;
	dev->mode_config.max_height = ST_MAX_FB_HEIGHT;

	dev->mode_config.funcs = &st_mode_config_funcs;
	dev->mode_config.helper_private = &st_drm_mode_config_helpers;
}

static int st_load(struct drm_device *dev, unsigned long flags)
{
	struct st_private *private;
	int ret;

	dev_dbg(dev->dev, "%s\n", __func__);

	private = devm_kzalloc(dev->dev, sizeof(*private), GFP_KERNEL);
	if (!private) {
		DRM_ERROR("Failed to allocate private\n");
		return -ENOMEM;
	}
	dev->dev_private = (void *)private;
	private->drm_dev = dev;
	platform_set_drvdata(dev->platformdev, dev);

	drm_mode_config_init(dev);
	drm_kms_helper_poll_init(dev);

	st_mode_config_init(dev);

	ret = component_bind_all(dev->dev, dev);
	if (ret)
		goto err_bind;

	drm_mode_config_reset(dev);

#ifdef CONFIG_DRM_ST_LTDC_FBDEV
	private->fbdev_cma = drm_fbdev_cma_init(dev, 16,
						dev->mode_config.num_crtc,
						dev->mode_config.num_connector);

	if (IS_ERR(private->fbdev_cma)) {
		ret = PTR_ERR(private->fbdev_cma);
		goto err_fbdev;
	}
#endif
	return 0;

#ifdef CONFIG_DRM_ST_LTDC_FBDEV
err_fbdev:
	component_unbind_all(dev->dev, dev);
#endif
err_bind:
	drm_kms_helper_poll_fini(dev);
	drm_mode_config_cleanup(dev);
	return ret;
}

static int st_unload(struct drm_device *dev)
{
	struct st_private *private = dev->dev_private;

	dev_dbg(dev->dev, "%s\n", __func__);

	drm_fbdev_cma_fini(private->fbdev_cma);
	drm_kms_helper_poll_fini(dev);
	drm_mode_config_cleanup(dev);
	component_unbind_all(dev->dev, dev);

	return 0;
}

static int st_dumb_create(struct drm_file *file, struct drm_device *dev,
			  struct drm_mode_create_dumb *args)
{
	unsigned int min_pitch = DIV_ROUND_UP(args->width * args->bpp, 8);
	/*
	 * in order to comply with GPU constraint, pitch is aligned on
	 * 64 bytes, height is aligned on 4 bytes
	 */
	args->pitch = roundup(min_pitch, 64);
	args->height = roundup(args->height, 4);

	return drm_gem_cma_dumb_create_internal(file, dev, args);
}

static const struct file_operations st_driver_fops = {
	.owner = THIS_MODULE,
	.open = drm_open,
	.mmap = drm_gem_cma_mmap,
	.poll = drm_poll,
	.read = drm_read,
	.unlocked_ioctl = drm_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = drm_compat_ioctl,
#endif
	.release = drm_release,
};

static struct dma_buf *st_gem_prime_export(struct drm_device *dev,
					   struct drm_gem_object *obj,
					   int flags)
{
	dev_dbg(dev->dev, "%s\n", __func__);

	/* we want to be able to write in mmapped buffer */
	flags |= O_RDWR;
	return drm_gem_prime_export(dev, obj, flags);
}

#ifdef CONFIG_DEBUG_FS
static struct drm_info_list st_dbg_list[] = {
		{"fb", drm_fb_cma_debugfs_show, 0},
		{"mm", ltdc_mm_show, 0},
		{"fps_show", ltdc_fps_show, 0},
		{"ltdc_regs", ltdc_dbg_show_regs, 0},
		{"show_and_reset_underrun_nb",
			ltdc_dbg_show_and_reset_underrun, 0},
};

DEFINE_SIMPLE_ATTRIBUTE(st_drm_add_fps_filter_fops,
			ltdc_drm_fps_filter_get,
			ltdc_drm_fps_filter_set, "%llu\n");

static int st_drm_debugfs_create(struct dentry *root,
				 struct drm_minor *minor,
				 const char *name,
				 const struct file_operations *fops)
{
	struct drm_device *dev = minor->dev;
	struct drm_info_node *node;
	struct dentry *ent;

	ent = debugfs_create_file(name, 0644, root, dev, fops);
	if (IS_ERR(ent))
		return PTR_ERR(ent);

	node = kzalloc(sizeof(*node), GFP_KERNEL);
	if (!node) {
		debugfs_remove(ent);
		return -ENOMEM;
	}

	node->minor = minor;
	node->dent = ent;
	node->info_ent = (void *)fops;

	mutex_lock(&minor->debugfs_lock);
	list_add(&node->list, &minor->debugfs_list);
	mutex_unlock(&minor->debugfs_lock);

	return 0;
}

int st_debugfs_init(struct drm_minor *minor)
{
	int ret;

	ret = drm_debugfs_create_files(st_dbg_list, ARRAY_SIZE(st_dbg_list),
				       minor->debugfs_root, minor);
	if (ret) {
		DRM_ERROR("Cannot install debugfs\n");
		return ret;
	}

	ret = st_drm_debugfs_create(minor->debugfs_root, minor,
				    "fps_dont_count_duplicated_frames",
				     &st_drm_add_fps_filter_fops);
	if (ret) {
		drm_debugfs_remove_files(st_dbg_list, ARRAY_SIZE(st_dbg_list),
					 minor);
		DRM_ERROR("Cannot install debugfs\n");
	}

	return ret;
}

void st_debugfs_cleanup(struct drm_minor *minor)
{
	drm_debugfs_remove_files(st_dbg_list, ARRAY_SIZE(st_dbg_list), minor);
	drm_debugfs_remove_files((struct drm_info_list *)
				 &st_drm_add_fps_filter_fops,
				 1, minor);
}

#endif /* CONFIG_DEBUG_FS */

static int st_drm_open(struct drm_device *dev, struct drm_file *filp)
{
	struct st_private *private = dev->dev_private;

	private->filp = filp;

	return 0;
}

static struct drm_driver st_driver = {
	.driver_features = DRIVER_MODESET | DRIVER_GEM |
			   DRIVER_PRIME | DRIVER_ATOMIC,
	.open = st_drm_open,
	.load = st_load,
	.unload = st_unload,
	.gem_free_object = drm_gem_cma_free_object,
	.gem_vm_ops = &drm_gem_cma_vm_ops,
	.dumb_create = st_dumb_create,
	.dumb_map_offset = drm_gem_cma_dumb_map_offset,
	.dumb_destroy = drm_gem_dumb_destroy,
	.fops = &st_driver_fops,

	.get_vblank_counter = drm_vblank_no_hw_counter,
	.enable_vblank = ltdc_crtc_enable_vblank,
	.disable_vblank = ltdc_crtc_disable_vblank,

	.prime_handle_to_fd = drm_gem_prime_handle_to_fd,
	.prime_fd_to_handle = drm_gem_prime_fd_to_handle,
	.gem_prime_export = st_gem_prime_export,
	.gem_prime_import = drm_gem_prime_import,
	.gem_prime_get_sg_table = drm_gem_cma_prime_get_sg_table,
	.gem_prime_import_sg_table = drm_gem_cma_prime_import_sg_table,
	.gem_prime_vmap = drm_gem_cma_prime_vmap,
	.gem_prime_vunmap = drm_gem_cma_prime_vunmap,
	.gem_prime_mmap = drm_gem_cma_prime_mmap,

#ifdef CONFIG_DEBUG_FS
	.debugfs_init = st_debugfs_init,
	.debugfs_cleanup = st_debugfs_cleanup,
#endif

	.name = DRIVER_NAME,
	.desc = DRIVER_DESC,
	.date = DRIVER_DATE,
	.major = DRIVER_MAJOR,
	.minor = DRIVER_MINOR,
};

static int compare_of(struct device *dev, void *data)
{
	return dev->of_node == data;
}

static int st_bind(struct device *dev)
{
	return drm_platform_init(&st_driver, to_platform_device(dev));
}

static void st_unbind(struct device *dev)
{
	drm_put_dev(platform_get_drvdata(to_platform_device(dev)));
}

static const struct component_master_ops st_ops = {
	.bind = st_bind,
	.unbind = st_unbind,
};

static int st_platform_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node;
	struct device_node *child_np;
	struct component_match *match = NULL;

	DRM_DEBUG_DRIVER("\n");

	of_platform_populate(node, NULL, NULL, dev);

	child_np = of_get_next_available_child(node, NULL);

	while (child_np) {
		component_match_add(dev, &match, compare_of, child_np);
		of_node_put(child_np);
		child_np = of_get_next_available_child(node, child_np);
	}

	return component_master_add_with_match(dev, &st_ops, match);
}

static int st_platform_remove(struct platform_device *pdev)
{
	DRM_DEBUG_DRIVER("\n");

	component_master_del(&pdev->dev, &st_ops);
	of_platform_depopulate(&pdev->dev);

	return 0;
}

static const struct of_device_id st_dt_ids[] = {
	{ .compatible = "st,display-subsystem", },
	{ /* end node */ },
};
MODULE_DEVICE_TABLE(of, st_dt_ids);

static struct platform_driver st_platform_driver = {
	.probe = st_platform_probe,
	.remove = st_platform_remove,
	.driver = {
		.name = DRIVER_NAME,
		.of_match_table = st_dt_ids,
	},
};

static struct platform_driver * const drivers[] = {
	&st_platform_driver,
	&ltdc_driver,
};

static void st_unregister_drivers(void)
{
	int i;

	for (i = ARRAY_SIZE(drivers) - 1; i >= 0 ; i--) {
		if (!drivers[i])
			continue;
		platform_driver_unregister(drivers[i]);
	}
}

static int st_register_drivers(void)
{
	unsigned int i;
	int ret;

	for (i = 0; i < ARRAY_SIZE(drivers); i++) {
		if (!drivers[i])
			continue;

		ret = platform_driver_register(drivers[i]);
		if (ret)
			return ret;
	}

	return 0;
}

static int st_drm_init(void)
{
	int ret = st_register_drivers();

	if (ret) {
		DRM_ERROR("Failed to register driver\n");
		st_unregister_drivers();
	}

	return ret;
}

module_init(st_drm_init);

static void st_drm_exit(void)
{
	st_unregister_drivers();
}
module_exit(st_drm_exit);

MODULE_AUTHOR("Philippe Cornu <philippe.cornu@st.com>");
MODULE_AUTHOR("Yannick Fertre <yannick.fertre@st.com>");
MODULE_DESCRIPTION("STMicroelectronics ST DRM driver");
MODULE_LICENSE("GPL");

