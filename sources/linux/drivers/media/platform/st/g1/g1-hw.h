/*
 * Copyright (C) STMicroelectronics SA 2015
 * Authors: Hugues Fruchet <hugues.fruchet@st.com> for STMicroelectronics.
 * License terms:  GNU General Public License (GPL), version 2
 */

#ifndef G1_HW_H
#define G1_HW_H

#include <linux/regulator/driver.h>
#include <linux/clk.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/completion.h>

#define PERF_CIRCULAR_ARRAY_SIZE    10
struct g1_hw_perf {
	unsigned long max;
	unsigned long min;
	unsigned long sum;
	unsigned long n;
	unsigned long circular_idx;
	unsigned long duration[PERF_CIRCULAR_ARRAY_SIZE];
};

struct g1_hw_fuse {
	u32 h264_support_fuse;	/* HW supports h.264 */
	u32 mpeg4_support_fuse;	/* HW supports MPEG-4 */
	u32 mpeg2_support_fuse;	/* HW supports MPEG-2 */
	u32 sorenson_spark_support_fuse;	/* HW supports Sorenson Spark */
	u32 jpeg_support_fuse;	/* HW supports JPEG */
	u32 vp6_support_fuse;	/* HW supports VP6 */
	u32 vp7_support_fuse;	/* HW supports VP6 */
	u32 vp8_support_fuse;	/* HW supports VP6 */
	u32 vc1_support_fuse;	/* HW supports VC-1 Simple */
	u32 jpeg_prog_support_fuse;	/* HW supports Progressive JPEG */
	u32 pp_support_fuse;	/* HW supports post-processor */
	u32 pp_config_fuse;	/* HW post-processor functions bitmask */
	u32 max_dec_pic_width_fuse;/* Max video decoding width supported  */
	u32 max_pp_out_pic_width_fuse;/* Max output width of Post-Processor */
	u32 ref_buf_support_fuse;/* HW supports reference picture buffering */
	u32 avs_support_fuse;	/* one of the AVS values defined above */
	u32 rv_support_fuse;	/* one of the REAL values defined above */
	u32 mvc_support_fuse;
	u32 custom_mpeg4_support_fuse;	/* Fuse for custom MPEG-4 */
};

struct g1_hw_config {
	u32 mpeg4_support;	/* HW supports MPEG-4 */
	u32 custom_mpeg4_support;/* HW supports custom MPEG-4 features */
	u32 h264_support;	/* HW supports h.264 */
	u32 vc1_support;	/* HW supports VC-1 Simple */
	u32 mpeg2_support;	/* HW supports MPEG-2 */
	u32 jpeg_support;	/* HW supports JPEG */
	u32 jpeg_prog_support;	/* HW supports Progressive JPEG */
	u32 max_dec_pic_width;	/* Max video decoding width supported  */
	u32 pp_support;		/* HW supports post-processor */
	u32 pp_config;		/* HW post-processor functions bitmask */
	u32 max_pp_out_pic_width;/* Max output width of Post-Processor */
	u32 sorenson_spark_support;/* HW supports Sorenson Spark */
	u32 ref_buf_support;	/* HW supports reference picture buffering */
	u32 tiled_mode_support;	/* HW supports tiled reference pictuers */
	u32 vp6_support;	/* HW supports VP6 */
	u32 vp7_support;	/* HW supports VP7 */
	u32 vp8_support;	/* HW supports VP8 */
	u32 avs_support;	/* HW supports AVS */
	u32 jpeg_e_support;	/* HW supports JPEG extensions */
	u32 rv_support;		/* HW supports REAL */
	u32 mvc_support;	/* HW supports H264 MVC extension */
	u32 webp_support;	/* HW supports web_p (VP8 snapshot) */
	u32 ec_support;		/* HW supports error concealment */
	u32 stride_support;	/* HW supports arbitrary Y and C strides */
	u32 field_dpb_support;	/* HW supports field-mode DPB */
	u32 avs_plus_support;	/* one of the AVS PLUS values defined above */
};

struct g1_hw {
	void __iomem *regs;
	int regs_size;
	int irq;
	unsigned long int chip_id;
	struct mutex protect_mutex;
	struct completion dec_interrupt;
	struct completion pp_interrupt;
	struct clk *clk;
	struct regulator *regulator;

	/* debugfs */
	struct dentry *debugfs_dir;
	struct dentry *debugfs_regs;
	struct dentry *debugfs_stats;
	struct dentry *debugfs_config;
	/* profiling */
	struct g1_hw_perf perf;

	/* config */
	u32 asic_id;
	u32 dec_cfg;
	u32 dec_cfg2;
	u32 dec_fuse;
	u32 pp_cfg;
	u32 pp_fuse;
	struct g1_hw_fuse fuse;
	struct g1_hw_config config;
	int suspended_state;
};

#define G1_MAX_REGS  (60 + 40 + 10)
struct g1_hw_regs {
	int n;
	int offset[G1_MAX_REGS];
	int data[G1_MAX_REGS];
};

struct g1_dev;
struct g1_ctx;

int g1_hw_probe(struct g1_dev *g1);
void g1_hw_remove(struct g1_dev *g1);
int g1_hw_run(struct g1_ctx *ctx, struct g1_hw_regs *regs);
int g1_hw_suspend(struct g1_dev *g1);
int g1_hw_resume(struct g1_dev *g1);
int g1_hw_pm_suspend(struct g1_dev *g1);
int g1_hw_pm_resume(struct g1_dev *g1);

void g1_hw_cfg_build_config(struct g1_dev *g1);
void g1_hw_cfg_build_fuse(struct g1_dev *g1);
u32 g1_hw_cfg_get_asic_id(struct g1_dev *g1);
void g1_hw_cfg_get_config(struct g1_dev *g1, struct g1_hw_config *config);
void g1_hw_cfg_get_fuse(struct g1_dev *g1, struct g1_hw_fuse *fuse);
unsigned char *g1_hw_cfg_summary_str(struct g1_dev *g1,
				     unsigned char *str, unsigned int len);
unsigned char *g1_hw_cfg_dump_str(struct g1_dev *g1,
				  unsigned char *str, unsigned int len);

#endif /* G1_HW_H */
