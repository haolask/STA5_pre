/*
 * ITU-R BT.601 / BT.656 Video Input Port driver for Accordo5/sta SoC.
 *
 * Copyright (C) 2009-2016 STMicroelectronics Automotive Group
 *
 * author: Philippe Langlais (philippe.langlais@st.com)
 *         Pierre-Yves MORDRET (pierre-yves.mordre@st.com)
 * It's a derivated works from:
 * original cartesio driver by: Sandeep Kaushik (sandeep-mmc.kaushik@st.com)
 * and sta VIP PCI driver by: Federico Vaga <federico.vaga@gmail.com>
 *                                Andreas Kies <andreas.kies@windriver.com>
 *                                Vlad Lungu   <vlad.lungu@windriver.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/videodev2.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/vmalloc.h>
#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_graph.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/i2c.h>
#include <linux/debugfs.h>

#include <media/v4l2-common.h>
#include <media/v4l2-device.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-event.h>
#include <media/v4l2-of.h>
#include <media/videobuf2-dma-contig.h>

#include "sta_vip.h"

/* Write VIP register */
static inline void reg_vip_write(struct sta_vip *vip, unsigned int reg, u32 val)
{
	writel(val, vip->base + reg);
}

/* Read VIP register */
static inline u32 reg_vip_read(struct sta_vip *vip, unsigned int reg)
{
	return  readl(vip->base + reg);
}

/* Write SGA register */
static inline void reg_sga_write(struct sta_sga *sga, unsigned int reg, u32 val)
{
	writel(val, sga->base + reg);
}

/* Read SGA register */
static inline u32 reg_sga_read(struct sta_sga *sga, unsigned int reg)
{
	return  readl(sga->base + reg);
}

/* Forward declaration */
static void sta_sga_stop_grabbing(struct vb2_buffer *vb2);

/*
 * Service routine is hooked up but line interrupt is not
 * enabled.
 */
static irqreturn_t sga_irq(int irq, void *dev_id)
{
	struct sta_sga *sga = (struct sta_sga *)dev_id;
	struct sta_vip *vip =  container_of(sga, struct sta_vip, sga);
	struct vb2_v4l2_buffer *vb2_v4l2_buf;
	unsigned long mis, imsc, gitr;
	unsigned long index;
	unsigned long flags;
	bool running = sga->running;
	bool pending = vip->pending;
	u64 duration = 0;

	mis = reg_sga_read(sga, SGA_MIS);
	imsc = reg_sga_read(sga, SGA_IMSC);
	gitr = reg_sga_read(sga, SGA_GITR);

	if (mis & SGA_WARMUP_INT) {
		imsc |= SGA_WARMUP_INT;
		dev_dbg(vip->v4l2_dev.dev, "SGA Warmup !");
		sga->warmup_done = true;
		goto handled;
	}

	if (!vip->active) {
		running = false;
		dev_err(vip->v4l2_dev.dev, "It is not active\n");
		goto handled;
	}
	vb2_v4l2_buf = &vip->active->vbuf;
	index = vip->active->index;

	if (mis & (SGA_INT_AHBEMIS | SGA_INT_HANGMIS | SGA_INT_IFIFOOVMIS)) {
		running = false;
		dev_err(vip->v4l2_dev.dev, "SGA Error(0x%X)\n",
			(unsigned int)(mis &
				       (SGA_INT_AHBEMIS | SGA_INT_HANGMIS |
					SGA_INT_IFIFOOVMIS)));
		goto error;
	}

	if (gitr & (1 << SGA_ERROR_TST_BIT)) {
		running = false;
		/* clear error */
		reg_sga_write(sga, SGA_CITR, (1 << SGA_ERROR_TST_BIT));
		dev_dbg(vip->v4l2_dev.dev, "SGA Error(SGA_ERROR_TST_BIT)\n");
		goto error;
	}

	if (mis & SGA_RUNNING_INT) {
		running = true;
		imsc |= SGA_RUNNING_INT;
		dev_dbg(vip->v4l2_dev.dev, "SGA Running !");
		goto handled;
	}

	if (!(mis & (1 << (index + 8)))) {
		dev_err(vip->v4l2_dev.dev, "Unexpected IT\n");
		goto handled;
	}

	/* Remove the active buffer from the list */
	spin_lock_irqsave(&vip->rlock, flags);
	list_del_init(&vip->active->queue);
	vip->active = NULL;
	vip->pending = false;
	spin_unlock_irqrestore(&vip->rlock, flags);

	if (pending) {
		dev_dbg(vip->v4l2_dev.dev, "SGA Reschedule.\n");
		schedule_delayed_work(&vip->work, 0);
	}

	imsc |= (1 << (index + 8));
	spin_lock_irqsave(&sga->rlock, flags);
	sga->running = false;
	reg_sga_write(sga, SGA_ICR, mis);
	reg_sga_write(sga, SGA_IMSC, imsc);
	sga->heartbeat = SGA_HEARTBEAT;
	spin_unlock_irqrestore(&sga->rlock, flags);
	vb2_v4l2_buf->vb2_buf.timestamp = ktime_get_ns();
	vb2_v4l2_buf->sequence = vip->sequence++;
	vb2_v4l2_buf->field = vip->field;

	switch (vip->sequence) {
	case 1:
		vip->perf.start_streaming_ns = vb2_v4l2_buf->vb2_buf.timestamp;
		break;
	case 2:
		duration = vb2_v4l2_buf->vb2_buf.timestamp -
			   vip->perf.end_streaming_ns;
		vip->perf.min_duration = duration;
		vip->perf.max_duration = duration;
		break;
	default:
		duration = vb2_v4l2_buf->vb2_buf.timestamp -
			   vip->perf.end_streaming_ns;
		vip->perf.min_duration = min(duration, vip->perf.min_duration);
		vip->perf.max_duration = max(duration, vip->perf.max_duration);
		break;
	}

	vip->perf.end_streaming_ns = vb2_v4l2_buf->vb2_buf.timestamp;

	vb2_buffer_done(&vb2_v4l2_buf->vb2_buf, VB2_BUF_STATE_DONE);
	dev_dbg(vip->v4l2_dev.dev, "--> SGA(%lu)\n", index);

	sta_sga_stop_grabbing(&vb2_v4l2_buf->vb2_buf);

	return IRQ_HANDLED;

error:
	sta_sga_stop_grabbing(&vb2_v4l2_buf->vb2_buf);
	/* Remove the active buffer from the list */
	spin_lock_irqsave(&vip->rlock, flags);
	sga->running = running;
	vip->active = NULL;
	vip->pending = false;
	vip->frame_err++;
	spin_unlock_irqrestore(&vip->rlock, flags);
handled:
	spin_lock_irqsave(&sga->rlock, flags);
	sga->running = running;
	reg_sga_write(sga, SGA_ICR, mis);
	reg_sga_write(sga, SGA_IMSC, imsc);
	sga->heartbeat = SGA_HEARTBEAT;
	spin_unlock_irqrestore(&sga->rlock, flags);
	return IRQ_HANDLED;
}

static u32 sga_decode_fourcc(u32 fourcc)
{
	int i;

	for (i = 0; i < VIP_NUM_FORMATS; i++)
		if (sta_cap_formats[i].fourcc == fourcc)
			break;

	if (i >= VIP_NUM_FORMATS)
		return sta_cap_formats[0].sga_format;

	return sta_cap_formats[i].sga_format;
}

static void sta_sga_deinit_batch(struct vb2_buffer *vb2)
{
	struct sta_vip *vip = vb2_get_drv_priv(vb2->vb2_queue);
	struct sta_sga *sga = &vip->sga;
	struct vip_buffer *vip_buf = to_vip_buffer(vb2);
	unsigned long *fw_link;
	dma_addr_t last_inst;

	if (!sga->main_fw) {
		dev_err(vip->v4l2_dev.dev, "No Fw to release !\n");
		return;
	}

	fw_link = sga->main_fw + (2 * vip_buf->index);

	last_inst = sga->dma_main_fw + SGA_MAIN_FIRMWARE_SIZE - 1;

	*(fw_link + 1) = GOSUB
		| ((unsigned long)last_inst >> SGA_ADDRESS_ALIGN);

	dma_sync_single_for_device(vip->v4l2_dev.dev,
				   sga->dma_main_fw, SGA_MAIN_FIRMWARE_SIZE,
				   DMA_TO_DEVICE);

	sga->avail_batches |= (1 << vip_buf->index);
}

static int sta_sga_init_batch(struct vb2_buffer *vb2)
{
	struct sta_vip *vip = vb2_get_drv_priv(vb2->vb2_queue);
	struct vip_buffer *vip_buf = to_vip_buffer(vb2);
	struct sta_sga *sga = &vip->sga;

	unsigned int i;
	unsigned long *fw_link, *batch;

	unsigned long ctcmd, flags;

	if (sga->avail_batches == 0)
		return -EPERM;

	/* Search for free index */
	for (i = 0; i < sga->num_batches; i++)
		if (sga->avail_batches & (1 << i))
			break;

	/* Use the free index of SGA batches */
	sga->avail_batches &= ~(1 << i);
	batch = *(sga->batches + i);

	/* Link Batch with Main Fw */
	fw_link = sga->main_fw + (2 * i);

	spin_lock_irqsave(&sga->rlock, flags);
	ctcmd = reg_sga_read(sga, SGA_CTCMD);
	ctcmd |= SGA_CTCMD_IHALT;
	reg_sga_write(sga, SGA_CTCMD, ctcmd);
	spin_unlock_irqrestore(&sga->rlock, flags);

	/* Replace GOSUB Instruction */
	*(fw_link + 1) = GOSUB
		| ((unsigned long)*(sga->dma_batches + i) >> SGA_ADDRESS_ALIGN);
	/* Place a RETURN as the first batch instruction
	 * in waiting to be filled up
	 */
	*batch = RETURN;

	dma_sync_single_for_device(vip->v4l2_dev.dev,
				   sga->dma_main_fw, SGA_MAIN_FIRMWARE_SIZE,
				   DMA_TO_DEVICE);

	dma_sync_single_for_device(vip->v4l2_dev.dev,
				   *(sga->dma_batches + i),
				   SGA_BATCH_DEFAULT_SIZE, DMA_TO_DEVICE);

	spin_lock_irqsave(&sga->rlock, flags);
	ctcmd = reg_sga_read(sga, SGA_CTCMD);
	ctcmd |= SGA_CTCMD_IRESUME;
	reg_sga_write(sga, SGA_CTCMD, ctcmd);
	spin_unlock_irqrestore(&sga->rlock, flags);

	vip_buf->index = i;
	return 0;
}

static int sta_sga_build_main_fw(struct sta_sga *sga)
{
	unsigned long *main_fw;
	dma_addr_t last_inst;
	u32 last_inst_offset;
	int i = 0;

	main_fw = sga->main_fw;
	last_inst = ALIGN_DOWN(sga->dma_main_fw + SGA_MAIN_FIRMWARE_SIZE - 1,
			       SGA_MEMORY_ALIGN);
	last_inst_offset = last_inst - sga->dma_main_fw;

	/* Main Batch firmware + 1 for Warmup */
	for (i = 0; i < SGA_MAX_AVAIL_BATCHES + 1; i++) {
		*(main_fw + 2 * i) = TST_INSTR_TEST_REG | i;
		*(main_fw + 2 * i + 1) = GOSUB
			| ((unsigned long)last_inst >> SGA_ADDRESS_ALIGN);
	}
	i = (SGA_MAX_AVAIL_BATCHES + 1) << 1;
	*(main_fw + i++) = WAIT_INSTR_TEST_REG;
	*(main_fw + i++) = GOTO
		| ((unsigned long)sga->dma_main_fw >> SGA_ADDRESS_ALIGN);

	/* Insert the the "STOP" command at default batch firmware
	 * memory location
	 */
	*(sga->main_fw + (last_inst_offset / sizeof(sga->main_fw))) = STOP;

	sga->num_batches = SGA_MAX_AVAIL_BATCHES;
	sga->avail_batches = (1 << SGA_MAX_AVAIL_BATCHES) - 1;

	return 0;
}

static void sta_sga_run_main_batch_firmware(struct sta_sga *sga)
{
	unsigned long flags;
	unsigned long cmd, instr, ctcmd, cstat;
	int timeout;
	struct sta_vip *vip = container_of(sga, struct sta_vip, sga);

	dma_sync_single_for_device(vip->v4l2_dev.dev, sga->dma_main_fw,
				   SGA_MAIN_FIRMWARE_SIZE, DMA_TO_DEVICE);

	spin_lock_irqsave(&sga->rlock, flags);

	/* Start the main batch firmware */
	cmd = GOTO | (((unsigned long)sga->dma_main_fw) >> SGA_ADDRESS_ALIGN);
	reg_sga_write(sga, SGA_INSTR, cmd);

	/* Cache Configuration */
	instr =  CACHE_CTRL
		| SGA_CACHE_CONFIGURATION;
	reg_sga_write(sga, SGA_INSTR, instr);

	/* Auto Fetch Active */
	instr =  AHB
		| SGA_AHB_CONFIGURATION;
	reg_sga_write(sga, SGA_INSTR, instr);

	/* Enable the SGA Global resume bit */
	/* Enable the SGA Instructions resume bit */
	ctcmd = reg_sga_read(sga, SGA_CTCMD);
	ctcmd |= SGA_CTCMD_GRESUME | SGA_CTCMD_IRESUME;
	reg_sga_write(sga, SGA_CTCMD, ctcmd);

	timeout = 5;
	do  {
		timeout--;
		cstat = reg_sga_read(sga, SGA_CTSTAT);
	} while (((cstat & SGA_CTSTAT_IPEN) == 0) && (timeout > 0));
	if ((cstat & SGA_CTSTAT_IPEN) == 0)
		dev_err(vip->v4l2_dev.dev, "SGA definitely dead\n");

	spin_unlock_irqrestore(&sga->rlock, flags);
}

static void sta_sga_build_batch(struct vb2_buffer *vb2)
{
	unsigned long i, n;
	struct vip_buffer *vip_buf = to_vip_buffer(vb2);
	struct sta_vip *vip = vb2_get_drv_priv(vb2->vb2_queue);
	struct sta_sga *sga = &vip->sga;
	unsigned long *instr_ptr = *(sga->batches + vip_buf->index);
	dma_addr_t buf = vb2_dma_contig_plane_dma_addr(vb2, 0);
	/* Unit for WAIT_NEW_SYNCHRO timeout delay is: 256 SGA clock cycle
	 * Set timeout as 2 times the expected line duration
	 */
	u32 line_delay = (2 * LINE_DURATION_US * SGA_FREQUENCY_MHZ) / 256;
	u32 nb_instr_per_line;
	u32 nb_lines;
	unsigned long *last_instr;
	unsigned long *loop_instr;
	u32 tst_register_bit;

	dev_dbg(vip->v4l2_dev.dev, "VIP buf=0x%X\n", (unsigned int)buf);

	/* Pixel Format Image 0 = Monochrome 1bpp, in0 Off */
	*instr_ptr++ = IN0_SET_PIXEL_TYPE;

	/* Pixel Format Image 0 = Monochrome 1bpp, in1 Off */
	*instr_ptr++ = IN1_SET_PIXEL_TYPE;

	/* Pixel Format Image 0 = Monochrome 1bpp, in2 Off */
	*instr_ptr++ = IN2_SET_PIXEL_TYPE;

	/* HProt fixed value, BurstType INCR8, Auto Fetch Mode */
	*instr_ptr++ = AHB | SGA_AHB_CONFIGURATION;

	/* AutoFlushBank Active, Manual Flush, AutoInitCache */
	*instr_ptr++ = CACHE_CTRL | 0x7FD0;

	/* Transparency is disabled at Input and output */
	*instr_ptr++ = TRANSP_MODE | 0x00;

	/* Transparency KeyColor */
	*instr_ptr++ = TRANSP_IN_COLOR | 0x00;

	/* Allows to configure the Blending Environment of the Texture Blending
	 * Units
	 */
	*instr_ptr++ = SET_TEX_ENV_MSB
		/* Texture Unit ID #0 */
		| SGA_TEX_SOURCE_0

		/* We want to bypass this texture unit */
		| (SGA_TEX_ENV_REPLACE << SHFT_RGB_FNCN)

		/* We want to bypass this texture unit */
		| (SGA_TEX_ENV_REPLACE << SHFT_ALPHA_FNCN)

		/* We keep the composed color */
		| (SGA_TEX_ENV_SOURCE_TEXTURE << SHFT_RGB_SRC0)

		/* Not relevant */
		| (SGA_TEX_ENV_SOURCE_TEXTURE << SHFT_RGB_SRC1)

		/* Not relevant */
		| (SGA_TEX_ENV_SOURCE_TEXTURE << SHFT_RGB_SRC2)

		/* We keep the composed alpha*/
		| (SGA_TEX_ENV_SOURCE_TEXTURE << SHFT_ALPHA_SRC0)

		/* Not relevant */
		| (SGA_TEX_ENV_SOURCE_TEXTURE << SHFT_ALPHA_SRC1)

		/* Not relevant */
		| (SGA_TEX_ENV_SOURCE_TEXTURE << SHFT_ALPHA_SRC2);

	*instr_ptr++ = SET_TEX_ENV_LSB
		/* Texture Unit ID #0 */
		| SGA_TEX_SOURCE_0

		/* Color channel will remain in the color channel */
		| (SGA_TEX_ENV_RGB_OPR_COLOR << SHFT_OPR0_RGB)

		/* Not relevant */
		| (SGA_TEX_ENV_RGB_OPR_COLOR << SHFT_OPR1_RGB)

		/* Not relevant */
		| (SGA_TEX_ENV_RGB_OPR_COLOR << SHFT_OPR2_RGB)

		/* Alpha channel will remain in the color channel */
		| (SGA_TEX_ENV_A_OPR_ALPHA << SHFT_OPR0_ALPHA)

		/* Not relevant */
		| (SGA_TEX_ENV_A_OPR_ALPHA << SHFT_OPR1_ALPHA)

		/* Not relevant */
		| (SGA_TEX_ENV_A_OPR_ALPHA << SHFT_OPR2_ALPHA);

	*instr_ptr++ = SET_TEX_ENV_MSB
		/* Texture Unit ID #1 */
		| SGA_TEX_SOURCE_1
		/* We want to bypass this texture unit */
		| (SGA_TEX_ENV_REPLACE << SHFT_RGB_FNCN)
		/* We want to bypass this texture unit */
		| (SGA_TEX_ENV_REPLACE << SHFT_ALPHA_FNCN)
		/* We keep the previously composed color */
		| (SGA_TEX_ENV_SOURCE_PREVIOUS_COL << SHFT_RGB_SRC0)
		/* Not relevant */
		| (SGA_TEX_ENV_SOURCE_PREVIOUS_COL << SHFT_RGB_SRC1)
		/* Not relevant */
		| (SGA_TEX_ENV_SOURCE_PREVIOUS_COL << SHFT_RGB_SRC2)
		/* We keep the previously composed alpha */
		| (SGA_TEX_ENV_SOURCE_PREVIOUS_COL << SHFT_ALPHA_SRC0)
		/* Not relevant */
		| (SGA_TEX_ENV_SOURCE_PREVIOUS_COL << SHFT_ALPHA_SRC1)
		/* Not relevant */
		| (SGA_TEX_ENV_SOURCE_PREVIOUS_COL << SHFT_ALPHA_SRC2);

	*instr_ptr++ = SET_TEX_ENV_LSB
		/* Texture Unit ID #1 */
		| SGA_TEX_SOURCE_1
		/* Color channel will remain in the color channel */
		| (SGA_TEX_ENV_RGB_OPR_COLOR << SHFT_OPR0_RGB)
		/* Not relevant */
		| (SGA_TEX_ENV_RGB_OPR_COLOR << SHFT_OPR1_RGB)
		/* Not relevant */
		| (SGA_TEX_ENV_RGB_OPR_COLOR << SHFT_OPR2_RGB)
		/* Alpha channel will remain in the color channel */
		| (SGA_TEX_ENV_A_OPR_ALPHA << SHFT_OPR0_ALPHA)
		/* Not relevant */
		| (SGA_TEX_ENV_A_OPR_ALPHA << SHFT_OPR1_ALPHA)
		/* Not relevant */
		| (SGA_TEX_ENV_A_OPR_ALPHA << SHFT_OPR2_ALPHA);

	/* Allows to configure the RGB and Alpha Scaling in the Texture
	 * Units.
	 */
	*instr_ptr++ = SET_TEX_SCALE
		| SGA_TEX_SOURCE_1
		| 0xFFFF;		/* Rgb and alpha scales are both FF */
	*instr_ptr++ = SET_TEX_SCALE
		| SGA_TEX_SOURCE_0
		| 0xFFFF;		/* Rgb and alpha scales are both FF */

	/*
	 * Allows to configure the Green and Blue component of the Constant
	 * colour in the Texture Units.
	 */
	*instr_ptr++ = SET_TEX_COLORLSB
		| SGA_TEX_SOURCE_0	/* Texture Unit ID #0 */
		| 0xFF00;		/* Green(0xff) Constant color  */
	*instr_ptr++ = SET_TEX_COLORLSB
		| SGA_TEX_SOURCE_1	/* Texture Unit ID #1 */
		| 0xFF00;		/* Green(0xff) Constant color  */
	/*
	 * Allows to configure the Alpha and Red component of the Constant
	 * colour used in the Texture Units.
	 */
	*instr_ptr++ = SET_TEX_COLORMSB
		| SGA_TEX_SOURCE_0	/* Texture Unit ID #0 */
		| 0xFF00;		/* Alpha(0xff) Constant color  */
	*instr_ptr++ = SET_TEX_COLORMSB
		| SGA_TEX_SOURCE_1	/* Texture Unit ID #1 */
		| 0xFF00;		/* Alpha(0xff) Constant color  */

	/* Allows to configure the Blending Environment of the Frame Blending
	 * Unit
	 */
	*instr_ptr++ = SET_BLEND_ENV
		/* Add Blend Equation */
		| SGA_BLEND_OP_ADD
		/* Coef1 for RGB Frag  */
		| (SGA_BLEND_SRC_COEF_1 << SHFT_RGB_FRAG_SRC)
		/* Coef1 for Alpha Frag  */
		| (SGA_BLEND_SRC_COEF_1 << SHFT_ALPHA_FRAG_SRC)
		/* Coef0 for RGB Frame */
		| (SGA_BLEND_SRC_COEF_0 << SHFT_RGB_FRAME_SRC)
		/* Coef0 for Alpga Frame  */
		| (SGA_BLEND_SRC_COEF_0 << SHFT_ALPHA_FRAME_SRC);

	*instr_ptr++ = PIXEL_OP_MODE
		| SGA_STOP_CONCAT
		| SGA_ACTIVATE_SCISSOR_OPR
		| SGA_PIXEL_OPERATOR_BYPASS;

	/* Sets the amount of bytes to jump between 2 lines */
	*instr_ptr++ = IN2_SET_LINE_JUMP
		| ((vip->format_out.bytesperline) & 0x1FFF);

	/* Sets the X and Y size of the picture to process */
	*instr_ptr++ = IN2_SET_SIZE_XY
		| SGA_PACK_VECTOR(vip->format_out.width,
				  vip->format_out.height);

	/*
	 * Sets the signed pixel shift between a coordinate in the output image
	 * and in the input image 1
	 */
	*instr_ptr++ = IN2_SET_DELTA_XY
		| 0x0;

	*instr_ptr++ = SET_X_OFFSET
		| SGA_TEX_IN2;

	*instr_ptr++ = SET_XY_MODE
		| SGA_TEX_IN2
		| SGA_XYMODE_CLAMPING_X
		| SGA_XYMODE_CLAMPING_Y
		| SGA_XYMODE_ENABLE_ROTATION;

	*instr_ptr++ = SET_XX_COEF
		| SGA_TEX_IN2
		| (1 << 12); /*  Identity  */

	/*  No anamorphic transformation */
	*instr_ptr++ = SET_YX_COEF
		| SGA_TEX_IN2;
	*instr_ptr++ = SET_XY_COEF
		| SGA_TEX_IN2;

	*instr_ptr++ = SET_YY_COEF
		| SGA_TEX_IN2
		| (1 << 12);    /*  Identity  */

	*instr_ptr++ = SET_WX_COEF
		| SGA_TEX_IN2;
	*instr_ptr++ = SET_WY_COEF
		| SGA_TEX_IN2;
	*instr_ptr++ = SET_Y_OFFSET
		| SGA_TEX_IN2;
	*instr_ptr++ = SET_W_OFFSET
		| SGA_TEX_IN2;

	/* Sets the pixel format of the input image 1. */
	*instr_ptr++ = IN2_SET_PIXEL_TYPE
		| SGA_PIX_TYPE_ACTIVATE_FLOW
		| SGA_PIX_FMT_LITTLE_ENDIAN
		| sga_decode_fourcc(vip->format_out.pixelformat)
		| ((vip->format_out.pixelformat != vip->format_cap.pixelformat)
		   ? SGA_PIX_FMT_CONVRT : 0);

	*instr_ptr++ = IN2_BASE_ADD_MSB
		| ((((unsigned long)vip->phys + VIP_MEM_FIFO_BUFFER)
		    & 0xFF000000) >> 24);
	*instr_ptr++ = IN2_BASE_ADD
		| (((unsigned long)vip->phys + VIP_MEM_FIFO_BUFFER)
		   & 0x00FFFFFF);

	/* Sets the pixel format of the input image 0. */
	*instr_ptr++ = OUT_SET_PIXEL_TYPE
		/*  | inter_1->swap_rb */
		| sga_decode_fourcc(vip->format_cap.pixelformat)
		| SGA_PIX_TYPE_ACTIVATE_FLOW
		| SGA_PIX_FMT_LITTLE_ENDIAN;

	*instr_ptr++ = OUT_SET_LINE_JUMP
		| ((vip->format_cap.bytesperline) & 0x1FFF);
	*instr_ptr++ = OUT_SET_SIZE_XY
		| SGA_PACK_VECTOR(vip->format_cap.width,
				  vip->format_cap.height);
	*instr_ptr++ = OUT_SET_BASE_XY
		| 0x0;

	*instr_ptr++ = OUT_BASE_ADD_MSB
		| ((buf & 0xFF000000) >> 24);
	*instr_ptr++ = OUT_BASE_ADD
		| (buf & 0x00FFFFFF);

	/* Predict last instruction after grabbing all lines
	 * Also, take care on address alignment: GOTO instruction is performed
	 * on addresses multiple of 8-bytes (double instructions)
	 */
	nb_instr_per_line = 6; /* nb instructions inside the for loop */
	nb_lines = vip->format_out.height;

	if ((u32)instr_ptr & SGA_MEMORY_ALIGN_MASK)
		*instr_ptr++ = NO_OP;

	loop_instr = instr_ptr;

	*instr_ptr++ = TST_INSTR_TEST_REG | SGA_START_TST_BIT;
	/* If start bit is set, get out of loop */
	*instr_ptr++ = GOTO | (virt_to_phys(&loop_instr[4]) >>
			       SGA_ADDRESS_ALIGN);

	/* When test instruction register changes, go to "loop_instr"
	 * to test if start bit has been set
	 */
	*instr_ptr++ = WAIT_INSTR_TEST_REG;
	*instr_ptr++ = GOTO | (virt_to_phys(loop_instr) >> SGA_ADDRESS_ALIGN);

	*instr_ptr++ = NO_OP;

	/* The '+20' is for instructions outside of for loop plus
	 * address alignment
	 */
	last_instr = instr_ptr + (nb_lines * nb_instr_per_line) + 20;
	last_instr = (unsigned long *)((u32)last_instr &
				       ~SGA_MEMORY_ALIGN_MASK);

	/*
	 * First Part of the batch
	 * Draw in the intermediate buffer the field
	 */
	for (n = 2; n > 0; n--) {
		/* Wait VIP EOF */
		*instr_ptr++ = WAIT_NEW_SYNCHRO | 0xFFFFF1;
		/*  Fire an IT(0) to indicate SGA catches the beginning
		 *  of a frame
		 */
		if (n == 2) {
			*instr_ptr++ = SEND_INTERRUPT | SGA_RUNNING_INT_NB;
			tst_register_bit = SGA_EVEN_TST_BIT;
		} else {
			tst_register_bit = SGA_ODD_TST_BIT;
		}

		for (i = 0;
		     i < (vip->format_out.height - vip->format_cap.height) / 2;
		     i += 2) {
			/* Wait VIP Line */
			*instr_ptr++ = WAIT_NEW_SYNCHRO | 0xFFFFF0;
		}
		for (i = n - 1;
		     i < vip->format_cap.height;
		     i += 2) {
			*instr_ptr++ = SET_POINT0
				| SGA_PACK_VECTOR(0, i);
			*instr_ptr++ = SET_POINT1
				| SGA_PACK_VECTOR(vip->format_cap.width - 1, i);

			/* Wait VIP Line */
			if (i == (n - 1))
				*instr_ptr++ = WAIT_NEW_SYNCHRO | 0xFFFFF0;
			else
				*instr_ptr++ = WAIT_NEW_SYNCHRO	|
					(line_delay << 4) |
					0x0;

			*instr_ptr++ = DRAW_LINE;
			*instr_ptr++ = TST_INSTR_TEST_REG | tst_register_bit;
			*instr_ptr++ = GOTO |
				((unsigned long)virt_to_phys(last_instr) >>
				SGA_ADDRESS_ALIGN);
		}
	}

	/* Everything ended fine, we didn't jump to last instruction
	 * So, do not set error test bit
	 */
	*instr_ptr++ = GOTO |
		((unsigned long)virt_to_phys(&last_instr[2]) >>
		SGA_ADDRESS_ALIGN);

	if ((u32)instr_ptr > (u32)last_instr)
		dev_err(vip->v4l2_dev.dev,
			"Bad prediction for last instruction");

	while ((u32)instr_ptr < (u32)last_instr)
		*instr_ptr++ = NO_OP;

	/* This is referred as last instruction (last_instr) */
	*instr_ptr++ = SET_INSTR_TEST_REG | SGA_ERROR_TST_BIT;
	*instr_ptr++ = NO_OP;

	*instr_ptr++ = WAIT_PIPE_EMPTY;

	*instr_ptr++ = CLR_INSTR_TEST_REG | (vip_buf->index);

	/* General Purpose starts at 2 */
	*instr_ptr++ = SEND_INTERRUPT | (vip_buf->index + 2);
	*instr_ptr++ = RETURN;

	dma_sync_single_for_device(vip->v4l2_dev.dev,
				   *(sga->dma_batches + vip_buf->index),
				   SGA_BATCH_DEFAULT_SIZE, DMA_TO_DEVICE);

	vip_buf->size = instr_ptr - *(sga->batches + vip_buf->index);
}

static void sta_sga_run_warmup(struct sta_sga *sga)
{
	struct sta_vip *vip = container_of(sga, struct sta_vip, sga);
	unsigned long imsc, flags;

	dma_sync_single_for_device(vip->v4l2_dev.dev,
				   sga->dma_warmup, SGA_WARMUP_BATCH_SIZE,
				   DMA_TO_DEVICE);

	spin_lock_irqsave(&sga->rlock, flags);

	/*  General purpose start at 2 and mask at bit 8 */
	imsc = reg_sga_read(sga, SGA_IMSC);
	/*  Warmup batch is the very last one */
	imsc &= ~SGA_WARMUP_INT;
	reg_sga_write(sga, SGA_IMSC, imsc);

	/*  Ignite the SGA for the corresponding batch */
	reg_sga_write(sga, SGA_SITR, (1 << SGA_MAX_AVAIL_BATCHES));
	sga->warmup_done = false;

	spin_unlock_irqrestore(&sga->rlock, flags);
}

static int sta_sga_warmup(struct sta_sga *sga)
{
	unsigned long s;
	unsigned long *instr_ptr, *fw_link;

	instr_ptr = sga->warmup;

	*instr_ptr++ = IN0_SET_PIXEL_TYPE;
	*instr_ptr++ = IN2_SET_PIXEL_TYPE;
	*instr_ptr++ = AHB | SGA_AHB_CONFIGURATION;
	*instr_ptr++ = CACHE_CTRL  | SGA_CACHE_CONFIGURATION;
	*instr_ptr++ = TRANSP_MODE | 0x00;
	*instr_ptr++ = TRANSP_IN_COLOR | 0x00;
	*instr_ptr++ = TRANSP_IN_COLOR | 0x00;

	for (s = 0; s < SGA_NB_WARMUP_FRAMES; s++) {
		/* Wait VIP EOF */
		*instr_ptr++ = WAIT_NEW_SYNCHRO | 0xFFFF1;
		/* Wait VIP Line */
		*instr_ptr++ = WAIT_NEW_SYNCHRO | 0xFFFF0;
	}

	*instr_ptr++ = CLR_INSTR_TEST_REG | SGA_MAX_AVAIL_BATCHES;
	*instr_ptr++ = SEND_INTERRUPT     | SGA_WARMUP_INT_NB;
	*instr_ptr++ = RETURN;

	/* Link Batch with Main Fw */
	fw_link = sga->main_fw + (2 * SGA_MAX_AVAIL_BATCHES);
	/* Replace GOSUB Instruction */
	*(fw_link + 1) = GOSUB
		| ((unsigned long)sga->dma_warmup >> SGA_ADDRESS_ALIGN);

	return 0;
}

static void sta_sga_hwreset(struct sta_sga *sga)
{
	unsigned long flags;

	spin_lock_irqsave(&sga->rlock, flags);

	reg_sga_write(sga, SGA_CTCMD, SGA_CTCMD_GRST
		      | SGA_CTCMD_GHALT
		      | SGA_CTCMD_GINIT);
	spin_unlock_irqrestore(&sga->rlock, flags);
}

static int sta_sga_hwinit(struct sta_sga *sga)
{
	unsigned long flags;
	unsigned long ctcmd, grc, imsc;

	spin_lock_irqsave(&sga->rlock, flags);

	ctcmd = reg_sga_read(sga, SGA_CTCMD);
	grc = reg_sga_read(sga, SGA_GCR);
	imsc = reg_sga_read(sga, SGA_IMSC);

	ctcmd = SGA_CTCMD_GINIT;
	imsc = SGA_INT_GEN_ALL;
	grc &= ~SGA_GCR_INTCMOD1
		& ~SGA_GCR_HCLKGEN
		& ~SGA_GCR_FCLKGEN
		& ~SGA_GCR_INTCMOD;

	reg_sga_write(sga, SGA_CTCMD, ctcmd);
	reg_sga_write(sga, SGA_IMSC, imsc);
	reg_sga_write(sga, SGA_GCR, grc);

	spin_unlock_irqrestore(&sga->rlock, flags);

	if (sta_sga_build_main_fw(sga))
		return -EPERM;

	if (sta_sga_warmup(sga))
		return -EPERM;

	sta_sga_run_main_batch_firmware(sga);
	sta_sga_run_warmup(sga);

	return 0;
}

static void sta_sga_release(struct sta_sga *sga)
{
	sga->num_batches = 0;
	sga->avail_batches = 0;
}

static void sta_sga_stop_grabbing(struct vb2_buffer *vb2)
{
	struct sta_vip *vip = vb2_get_drv_priv(vb2->vb2_queue);
	struct sta_sga *sga = &vip->sga;
	struct vip_buffer *vip_buf = to_vip_buffer(vb2);
	unsigned long imsc, flags;

	spin_lock_irqsave(&sga->rlock, flags);

	/*  General purpose start at 2 and mask at bit 8 */
	imsc = reg_sga_read(sga, SGA_IMSC);
	imsc |= ((1 << (vip_buf->index + 8)) | SGA_RUNNING_INT);
	reg_sga_write(sga, SGA_IMSC, imsc);

	/*  Stop SGA for the corresponding batch */
	reg_sga_write(sga, SGA_CITR, (1 << vip_buf->index));
	sga->running = false;
	spin_unlock_irqrestore(&sga->rlock, flags);
}

static void sta_sga_start_grabbing(struct vb2_buffer *vb2)
{
	struct sta_vip *vip = vb2_get_drv_priv(vb2->vb2_queue);
	struct vip_buffer *vip_buf = to_vip_buffer(vb2);
	struct sta_sga *sga = &vip->sga;
	unsigned long imsc, flags;

	spin_lock_irqsave(&sga->rlock, flags);

	reg_sga_write(sga, SGA_CITR, 1 << SGA_START_TST_BIT);

	/*  General purpose start at 2 and mask at bit 8 */
	imsc = reg_sga_read(sga, SGA_IMSC);
	imsc &= ~((1 << (vip_buf->index + 8)) | SGA_RUNNING_INT);
	reg_sga_write(sga, SGA_IMSC, imsc);

	/*  Ignite the SGA for the corresponding batch */
	reg_sga_write(sga, SGA_SITR, (1 << vip_buf->index));
	/*  Running status is set by SGA itself */
	sga->running = false;
	spin_unlock_irqrestore(&sga->rlock, flags);
}

static u32 vip_dma_burst(u32 maxburst)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(burst_sizes); i++)
		if (burst_sizes[i].burstwords <= maxburst)
			break;

	return burst_sizes[i].reg;
}

static void sta_vip_hwinit(struct sta_vip *vip)
{
	unsigned long flags;

	/* Always configured in ITU R BT656 */
	spin_lock_irqsave(&vip->rlock, flags);
	vip->ctrl |= CTRL_INTL_EN
		| CTRL_PCK_POL
		| CTRL_IF_SYNC_TYPE
		| CTRL_EMB_CODES_EN
		| CTRL_DMA_BURST_SIZE(vip_dma_burst(VIP_DMA_BURST_SIZE))
		| CTRL_IF_TRANS(YUVCBYCRY)
		| CTRL_CAPTURE_MODES(VIDEO_MODE_ALL_FRAMES);

	reg_vip_write(vip, VIP_CTRL, vip->ctrl);
	reg_vip_write(vip, VIP_EFECR, ITU656_EMBEDDED_EVEN_CODE);
	reg_vip_write(vip, VIP_OFECR, ITU656_EMBEDDED_ODD_CODE);
	sta_sga_hwinit(&vip->sga);
	spin_unlock_irqrestore(&vip->rlock, flags);
}

static void sta_vip_hwreset(struct sta_vip *vip)
{
	unsigned long flags;

	spin_lock_irqsave(&vip->rlock, flags);
	reg_vip_write(vip, VIP_MASK, IRQ_RESET);

	vip->ctrl = 0;
	reg_vip_write(vip, VIP_CTRL, vip->ctrl);
	reg_vip_write(vip, VIP_EFECR, 0x0);
	reg_vip_write(vip, VIP_OFECR, 0x0);
	reg_vip_write(vip, VIP_CSTARTPR, 0x0);
	reg_vip_write(vip, VIP_CSTOPPR, 0x0);

	spin_unlock_irqrestore(&vip->rlock, flags);
	sta_sga_hwreset(&vip->sga);
}

/**
 * vip_start_capture - start video capturing
 * @vip: camera device
 *
 * Launch capturing. DMA channels should not be active yet. They should get
 * activated at the end of frame interrupt, to capture only whole frames, and
 * never begin the capture of a partial frame.
 */
static void vip_start_capture(struct sta_vip *vip)
{
	if (vb2_is_streaming(&vip->vq)) {	/* streaming is on */
		sta_sga_start_grabbing(&vip->active->vbuf.vb2_buf);
		return;
	}

	atomic_set(&vip_lines_nb, 0);
	vip->pending = false;
	vip->sga.running = false;
	/* sta_sga_run_warmup(&vip->sga); */

	reg_vip_write(vip, VIP_MASK, IRQ_ENABLE);
}

static void vip_stop_capture(struct sta_vip *vip)
{
	unsigned long flags;

	reg_vip_write(vip, VIP_MASK, IRQ_RESET);

	if (vip->active)
		sta_sga_stop_grabbing(&vip->active->vbuf.vb2_buf);
	spin_lock_irqsave(&vip->lock, flags);
	vip->active = NULL;
	spin_unlock_irqrestore(&vip->lock, flags);
}

/* Fetch the next buffer to activate */
static void vip_active_buf_next(struct sta_vip *vip)
{
	unsigned long flags;

	/* Get the next buffer */
	spin_lock_irqsave(&vip->lock, flags);
	if (list_empty(&vip->capture)) { /* No available buffer */
		vip->active = NULL;
		spin_unlock_irqrestore(&vip->lock, flags);
		return;
	}
	vip->active = list_first_entry(&vip->capture,
				       struct vip_buffer,
				       queue);
	spin_unlock_irqrestore(&vip->lock, flags);
}

static void vip_start_work(struct work_struct *work)
{
	struct sta_vip *vip;

	vip = container_of(to_delayed_work(work), struct sta_vip, work);

	if (!vip->sga.warmup_done)
		return;

	if (vip->intstatus == (IRQ_STATUS_FRAME_START
			       | IRQ_STATUS_FRAME_START_RAW
			       | IRQ_STATUS_FRAME_TFR
			       | IRQ_STATUS_FRAME_ODD)) {
		if (!vip->sga.running && vip->active) {
			/*  We got a problem SGA Miss the frame
			 *  start .. Drop it !
			 */
			vip->frame_err++;
			sta_sga_stop_grabbing(&vip->active->vbuf.vb2_buf);
			dev_dbg(vip->v4l2_dev.dev, "Frame dropped.\n");
			return;
		}
	}

	if (vb2_is_streaming(&vip->vq)) {	/* streaming is on */
		vip_active_buf_next(vip);
		if (vip->active) {
			if (vip->sga.heartbeat-- <= 0) {
				dev_err_ratelimited(vip->v4l2_dev.dev,
						    "SGA looks dead\n");
				vb2_queue_error(&vip->vq);
			} else  {
				dev_dbg(vip->v4l2_dev.dev, "-->\n");
				vip_start_capture(vip);	/* start capture */
			}
		} else {
			dev_dbg(vip->v4l2_dev.dev,
				"No active buffer available.\n");
		}
	}
}

#ifdef DEBUG
static irqreturn_t vip_line_irq(int irq, void *dev_id)
{
	struct sta_vip *vip = (struct sta_vip *)dev_id;
	unsigned long flags;

	spin_lock_irqsave(&vip->rlock, flags);
	reg_vip_write(vip, VIP_STATUS, IRQ_STATUS_LINE_END);
	spin_unlock_irqrestore(&vip->rlock, flags);
	atomic_inc(&vip_lines_nb);

	return IRQ_HANDLED;
}
#endif

static irqreturn_t vip_vsync_irq_thread(int irq, void *dev_id)
{
	struct sta_vip *vip = (struct sta_vip *)dev_id;
	unsigned long flags;

	switch (vip->event) {
	case VIP_FRAME_START_ODD:
		dev_dbg(vip->v4l2_dev.dev, "Start Odd(%d)\n",
			atomic_read(&vip_lines_nb));
		break;
	case VIP_FRAME_START_EVEN:
		dev_dbg(vip->v4l2_dev.dev, "Start Even(%d)\n",
			atomic_read(&vip_lines_nb));
		if (vip->sga.running) {
			spin_lock_irqsave(&vip->rlock, flags);
			/* Flag that indicates that is up
			 * to SGA to restart grabbing
			 */
			vip->pending = true;
			spin_unlock_irqrestore(&vip->rlock, flags);
		} else {
			vip->timeout = VIP_TIMEOUT;
			schedule_delayed_work(&vip->work, 0);
		}
		break;
	case VIP_FRAME_END_EVEN:
		dev_dbg(vip->v4l2_dev.dev, "End Even(%d/%d), %p\n",
			atomic_read(&vip_lines_nb),
			vip->format_cap.height, vip->active);
#ifdef DEBUG
		if (vip->sequence && (atomic_read(&vip_lines_nb) <
		    vip->format_cap.height)) {
			vip->incomplete_frame_nb++;
			dev_dbg(vip->v4l2_dev.dev,
				"Missing lines (%d/%d), %p\n",
				atomic_read(&vip_lines_nb),
				vip->format_cap.height, vip->active);
		}
		atomic_set(&vip_lines_nb, 0);
#endif
		break;
	case VIP_FRAME_END_ODD:
		dev_dbg(vip->v4l2_dev.dev, "End Odd(%d)\n",
			atomic_read(&vip_lines_nb));
		break;
	}

	return IRQ_HANDLED;
}

static irqreturn_t vip_vsync_irq(int irq, void *dev_id)
{
	struct sta_vip *vip = (struct sta_vip *)dev_id;
	unsigned long intstatus, flags, vip_mask;

	/* There is a bug on VIP hardware that makes frame interruptions to be
	 * raised at each line instead of each frame.
	 * In order to avoid having too many interrupts, the following
	 * workaround has been done:
	 *   - check that FRAME_START is raised on active frame (TFR flag set)
	 *   - check that FRAME_END is raised on blanking (TFR flag unset)
	 *   - masking the "start frame" interrupt until "end frame" is raised
	 *   - masking the "end frame" interrupt until "start frame" is raised
	 */

	intstatus = reg_vip_read(vip, VIP_STATUS) &
		(IRQ_STATUS_FRAME_START_RAW | IRQ_STATUS_FRAME_START
		 | IRQ_STATUS_FRAME_END_RAW | IRQ_STATUS_FRAME_END
		 | IRQ_STATUS_FRAME_TYPE
		 | IRQ_STATUS_FRAME_TFR);

	if (((intstatus & IRQ_STATUS_FRAME_START) &&
	     !(intstatus & IRQ_STATUS_FRAME_TFR)) ||
	    ((intstatus & IRQ_STATUS_FRAME_END) &&
	      (intstatus & IRQ_STATUS_FRAME_TFR))) {
		reg_vip_write(vip, VIP_STATUS, intstatus);
		return IRQ_HANDLED;
	}

	vip_mask = reg_vip_read(vip, VIP_MASK);

	if (intstatus & IRQ_STATUS_FRAME_START)
		vip_mask = (vip_mask & ~IRQ_FRAME_START) | IRQ_FRAME_END;
	else if (intstatus & IRQ_STATUS_FRAME_END)
		vip_mask = (vip_mask & ~IRQ_FRAME_END) | IRQ_FRAME_START;

	spin_lock_irqsave(&vip->rlock, flags);
	vip->intstatus = intstatus;
	reg_vip_write(vip, VIP_MASK, vip_mask);
	reg_vip_write(vip, VIP_STATUS, intstatus);
	spin_unlock_irqrestore(&vip->rlock, flags);

	if ((intstatus & IRQ_STATUS_FRAME_START) &&
	    ((intstatus & IRQ_STATUS_FRAME_TYPE) == IRQ_STATUS_FRAME_EVEN))
		vip->event = VIP_FRAME_START_EVEN;

	else if ((intstatus & IRQ_STATUS_FRAME_END) &&
		 ((intstatus & IRQ_STATUS_FRAME_TYPE) == IRQ_STATUS_FRAME_EVEN))
		vip->event = VIP_FRAME_END_EVEN;

	else if ((intstatus & IRQ_STATUS_FRAME_START) &&
		 ((intstatus & IRQ_STATUS_FRAME_TYPE) == IRQ_STATUS_FRAME_ODD))
		vip->event = VIP_FRAME_START_ODD;

	else if ((intstatus & IRQ_STATUS_FRAME_END) &&
		 ((intstatus & IRQ_STATUS_FRAME_TYPE) == IRQ_STATUS_FRAME_ODD))
		vip->event = VIP_FRAME_END_ODD;

	/* If the status register does not contain the frame event , simply
	 * return from the interrupt.
	 */
	else
		return IRQ_HANDLED;

	spin_lock_irqsave(&vip->sga.rlock, flags);
	switch (vip->event) {
	case VIP_FRAME_END_ODD:
		reg_sga_write(&vip->sga, SGA_CITR, 1 << SGA_ODD_TST_BIT);
		reg_sga_write(&vip->sga, SGA_SITR, 1 << SGA_EVEN_TST_BIT);
		reg_sga_write(&vip->sga, SGA_CITR, 1 << SGA_START_TST_BIT);
		break;
	case VIP_FRAME_END_EVEN:
		reg_sga_write(&vip->sga, SGA_CITR, 1 << SGA_EVEN_TST_BIT);
		reg_sga_write(&vip->sga, SGA_SITR, 1 << SGA_ODD_TST_BIT);
		reg_sga_write(&vip->sga, SGA_SITR, 1 << SGA_START_TST_BIT);
		break;
	default:
		reg_sga_write(&vip->sga, SGA_CITR, 1 << SGA_START_TST_BIT);
		break;
	}
	spin_unlock_irqrestore(&vip->sga.rlock, flags);

	return IRQ_WAKE_THREAD;
}

#if defined(RFU)
static void vip_setup_ctrl(struct sta_vip *vip,
			   struct v4l2_mbus_config *cfg)
{
	unsigned long flags;

	spin_lock_irqsave(&vip->rlock, flags);
	if (cfg->type == V4L2_MBUS_BT656) {
		vip->ctrl |= CTRL_EMB_CODES_EN
			| CTRL_PCK_POL | CTRL_IF_SYNC_TYPE;
		reg_vip_write(vip, VIP_EFECR, ITU656_EMBEDDED_EVEN_CODE);
		reg_vip_write(vip, VIP_OFECR, ITU656_EMBEDDED_ODD_CODE);
	} else {
		vip->ctrl &= ~CTRL_EMB_CODES_EN;
		vip->ctrl |= CTRL_EAV_SEL
			| CTRL_PCK_POL | CTRL_VS_POL | CTRL_HS_POL;

		if (cfg->flags & V4L2_MBUS_PCLK_SAMPLE_FALLING)
			vip->ctrl &= ~CTRL_PCK_POL;
		if (cfg->flags & V4L2_MBUS_HSYNC_ACTIVE_LOW)
			vip->ctrl &= ~CTRL_HS_POL;
		(cfg->flags & V4L2_MBUS_VSYNC_ACTIVE_LOW)
			vip->ctrl &= ~CTRL_VS_POL;
	}

	reg_vip_write(vip, VIP_CTRL, vip->ctrl);
	spin_unlock_irqrestore(&vip->rlock, flags);
}
#endif

/* Videobuf2 Operations */
static int vip_queue_setup(struct vb2_queue *vq, unsigned int *nbuffers,
			   unsigned int *nplanes, unsigned int sizes[],
			   struct device *alloc_devs[])
{
	struct sta_vip *vip = vb2_get_drv_priv(vq);

	if (!(*nbuffers) || *nbuffers > MAX_FRAMES)
		*nbuffers = MAX_FRAMES;

	*nplanes = 1;
	sizes[0] = vip->format_cap.sizeimage;

	vip->sequence = 0;
	vip->frame_err = 0;
	vip->frame_drop = 0;
	vip->incomplete_frame_nb = 0;
	vip->active = NULL;

	memset(&vip->perf, 0, sizeof(vip->perf));

	dev_dbg(vip->v4l2_dev.dev, "nbuf=%d, size=%u\n", *nbuffers, sizes[0]);

	return 0;
};

static int vip_buffer_init(struct vb2_buffer *vb)
{
	int ret;
	struct vip_buffer *vip_buf = to_vip_buffer(vb);
	struct sta_vip *vip = vb2_get_drv_priv(vb->vb2_queue);
	unsigned long image_size = vip->format_cap.sizeimage;

	INIT_LIST_HEAD(&vip_buf->queue);

	if (vb2_plane_size(vb, 0) < image_size)
		return -ENOMEM;

	ret = sta_sga_init_batch(vb);
	if (ret)
		return ret;

	sta_sga_build_batch(vb);
	return 0;
}

static int vip_buffer_prepare(struct vb2_buffer *vb)
{
	struct sta_vip *vip = vb2_get_drv_priv(vb->vb2_queue);
	struct vip_buffer *vip_buf = to_vip_buffer(vb);
	unsigned long size;

	size = vip->format_cap.sizeimage;
	if (vb2_plane_size(vb, 0) < size) {
		v4l2_err(&vip->v4l2_dev, "buffer too small (%lu < %lu)\n",
			 vb2_plane_size(vb, 0), size);
		return -EINVAL;
	}

	vb2_set_plane_payload(&vip_buf->vbuf.vb2_buf, 0, size);

#ifdef DEBUG
	/* This can be useful if you want to see if we actually fill
	 * the buffer with something
	 */
	memset(vb2_plane_vaddr(&vip_buf->vbuf.vb2_buf, 0), 0xaa, size);
#endif
	return 0;
}

static void vip_buffer_queue(struct vb2_buffer *vb)
{
	struct sta_vip *vip = vb2_get_drv_priv(vb->vb2_queue);
	struct vip_buffer *vip_buf = to_vip_buffer(vb);
	unsigned long flags;

	spin_lock_irqsave(&vip->lock, flags);
	list_add_tail(&vip_buf->queue, &vip->capture);
	if (!vip->active) {	/* No active buffer, active the first one */
		vip->active = list_first_entry(&vip->capture,
					       struct vip_buffer,
					       queue);
	}
	spin_unlock_irqrestore(&vip->lock, flags);
}

static void vip_buffer_finish(struct vb2_buffer *vb)
{
	struct sta_vip *vip = vb2_get_drv_priv(vb->vb2_queue);

	/* The Buffer is already removed from the queue at end SGA transfert */
	vip_active_buf_next(vip);
}

static void vip_buffer_cleanup(struct vb2_buffer *vb)
{
	struct sta_vip *vip = vb2_get_drv_priv(vb->vb2_queue);
	struct vip_buffer *vip_buf = to_vip_buffer(vb);

	if (vip_buf == vip->active) {
		/* System tries to cleanup the active buffer ? */
		dev_err(vip->v4l2_dev.dev,
			"Cleanup active VB Buffer not allowed\n");
		return;
	}

	sta_sga_stop_grabbing(vb);
	sta_sga_deinit_batch(vb);
}

static int vip_start_streaming(struct vb2_queue *vq, unsigned int count)
{
	struct sta_vip *vip = vb2_get_drv_priv(vq);

	/* Enable acquisition */
	vip_start_capture(vip);

	vip->perf.start_streaming_ns = ktime_get_ns();

	return 0;
}

/* abort streaming */
static void vip_stop_streaming(struct vb2_queue *vq)
{
	struct sta_vip *vip = vb2_get_drv_priv(vq);
	struct vip_buffer *vip_buf, *node;
	unsigned long flags;

	/* Disable acquisition */
	vip_stop_capture(vip);

	/* Release all active buffers */
	spin_lock_irqsave(&vip->lock, flags);
	list_for_each_entry_safe(vip_buf, node, &vip->capture, queue) {
		vb2_buffer_done(&vip_buf->vbuf.vb2_buf, VB2_BUF_STATE_ERROR);
		list_del(&vip_buf->queue);
	}
	spin_unlock_irqrestore(&vip->lock, flags);

	dev_dbg(vip->v4l2_dev.dev, "Frame errors/total: %d/%d\n",
		vip->frame_err, vip->sequence);
}

static int sta_vip_device_read(struct file *file, char __user *user_buf,
			       size_t size, loff_t *ppos)
{
	struct sta_vip *vip = file->private_data;
	unsigned char *cur = vip->str;
	unsigned int count = 0;
	size_t left = sizeof(vip->str);
	int cnt = 0;
	int ret = 0;

	memset(vip->str, 0, sizeof(vip->str));

	ret = snprintf(cur, left, "[%s]\n", vip->v4l2_dev.name);
	cnt = (left > ret ? ret : left);

	cur += cnt;
	left -= cnt;
	ret = snprintf(cur, left,
		       "registered as /dev/video%d\n", vip->video_dev.num);
	cnt = (left > ret ? ret : left);

	count = simple_read_from_buffer(user_buf, strlen(vip->str), ppos,
					vip->str, strlen(vip->str));

	return count;
}

static int sta_vip_hw_read(struct file *file, char __user *user_buf,
			   size_t size, loff_t *ppos)
{
	struct sta_vip *vip = file->private_data;
	struct sta_sga *sga = &vip->sga;
	unsigned int addr = 0;
	unsigned char *cur = vip->str;
	unsigned int count = 0;
	size_t left = sizeof(vip->str);
	int cnt = 0;
	int ret = 0;

	memset(vip->str, 0, sizeof(vip->str));

	ret = snprintf(cur, left, "VIP registers dump:\n");
	cnt = (left > ret ? ret : left);

	for (addr = 0x0; addr <= 0x1C; addr += 4) {
		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left,
			       "      0x%02X val 0x%X\n",
			       addr, reg_vip_read(vip, addr));
		cnt = (left > ret ? ret : left);
	}

	cur += cnt;
	left -= cnt;
	ret = snprintf(cur, left, "===================\n");
	cnt = (left > ret ? ret : left);

	cur += cnt;
	left -= cnt;
	ret = snprintf(cur, left, "SGA registers dump:\n");
	cnt = (left > ret ? ret : left);

	for (addr = 0x0; addr <= 0x60; addr += 4) {
		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left,
			       "      0x%02X val 0x%X\n",
			       addr, reg_sga_read(sga, addr));
		cnt = (left > ret ? ret : left);
	}

	cur += cnt;
	left -= cnt;
	ret = snprintf(cur, left, "===================\n");
	cnt = (left > ret ? ret : left);

	count = simple_read_from_buffer(user_buf, strlen(vip->str), ppos,
					vip->str, strlen(vip->str));

	return count;
}

static int sta_vip_last_read(struct file *file, char __user *user_buf,
			     size_t size, loff_t *ppos)
{
	struct sta_vip *vip = file->private_data;
	unsigned char *cur = vip->str;
	unsigned int count = 0;
	size_t left = sizeof(vip->str);
	int cnt = 0;
	int ret = 0;
	u64 duration_ns = 0;
	unsigned int duration = 0;
	int avg_fps = 0;

	memset(vip->str, 0, sizeof(vip->str));

	ret = snprintf(cur, left, "[last session]\n");
	cnt = (left > ret ? ret : left);

	cur += cnt;
	left -= cnt;
	ret = snprintf(cur, left,
		       "  |\n  |-[frame infos]\n");
	cnt = (left > ret ? ret : left);

	cur += cnt;
	left -= cnt;
	ret = snprintf(cur, left,
		       "  | |- frame errors: %d\n", vip->frame_err);
	cnt = (left > ret ? ret : left);

	cur += cnt;
	left -= cnt;
	ret = snprintf(cur, left,
		       "  | |- frame drop: %d\n", vip->frame_drop);
	cnt = (left > ret ? ret : left);

	cur += cnt;
	left -= cnt;
	ret = snprintf(cur, left,
		       "  | |- incomplete frames: %d\n",
		       vip->incomplete_frame_nb);
	cnt = (left > ret ? ret : left);

	cur += cnt;
	left -= cnt;
	ret = snprintf(cur, left,
		       "  | |- sequence: %d\n", vip->sequence);
	cnt = (left > ret ? ret : left);

	cur += cnt;
	left -= cnt;
	ret = snprintf(cur, left,
		       "  | |- capture format: %dx%d\n",
		       vip->format_cap.width,
		       vip->format_cap.height);
	cnt = (left > ret ? ret : left);

	cur += cnt;
	left -= cnt;
	ret = snprintf(cur, left,
		       "  | |- output format: %dx%d\n",
		       vip->format_out.width, vip->format_out.height);
	cnt = (left > ret ? ret : left);

	cur += cnt;
	left -= cnt;
	ret = snprintf(cur, left,
		       "  | |- standard: %s\n", v4l2_norm_to_name(vip->std));
	cnt = (left > ret ? ret : left);

	if (vip->perf.end_streaming_ns) {
		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left,
			       "  |\n  |-[performance]\n");
		cnt = (left > ret ? ret : left);

		duration_ns = vip->perf.min_duration;
		do_div(duration_ns, 1000ul);
		duration = (unsigned int)duration_ns;

		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left,
			       "  | |- min_duration (us): %d\n", duration);
		cnt = (left > ret ? ret : left);

		duration_ns = vip->perf.max_duration;
		do_div(duration_ns, 1000ul);
		duration = (unsigned int)duration_ns;

		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left,
			       "  | |- max_duration (us): %d\n", duration);
		cnt = (left > ret ? ret : left);

		duration_ns = vip->perf.end_streaming_ns -
			      vip->perf.start_streaming_ns;
		do_div(duration_ns, 1000000ul);
		duration = (unsigned int)duration_ns;

		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left,
			       "  | |- total_duration (ms): %d\n", duration);
		cnt = (left > ret ? ret : left);

		avg_fps = ((vip->sequence - 1) * 10000) / duration;

		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left,
			       "  | |- avg_fps (0.1Hz): %d\n", avg_fps);
		cnt = (left > ret ? ret : left);
	}

	count = simple_read_from_buffer(user_buf, strlen(vip->str), ppos,
					vip->str, strlen(vip->str));

	return count;
}

static const struct file_operations sta_vip_device_fops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = sta_vip_device_read,
};

static const struct file_operations sta_vip_last_fops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = sta_vip_last_read,
};

static const struct file_operations sta_vip_hw_fops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = sta_vip_hw_read,
};

void sta_vip_debugfs_create(struct sta_vip *vip)
{
	vip->debugfs_dir = debugfs_create_dir("sta_vip", NULL);

	vip->debugfs_device = debugfs_create_file("device", 0400,
				vip->debugfs_dir, vip, &sta_vip_device_fops);

	vip->debugfs_hw = debugfs_create_file("hw", 0400,
				vip->debugfs_dir, vip, &sta_vip_hw_fops);

	vip->debugfs_last = debugfs_create_file("last", 0400,
				vip->debugfs_dir, vip, &sta_vip_last_fops);
}

void sta_vip_debugfs_remove(struct sta_vip *vip)
{
	debugfs_remove(vip->debugfs_last);
	debugfs_remove(vip->debugfs_device);
	debugfs_remove(vip->debugfs_hw);
	debugfs_remove(vip->debugfs_dir);
}

static int sta_vip_g_v_ctrl(struct v4l2_ctrl *ctrl)
{
	int ret = 0;

	switch (ctrl->id) {
	case V4L2_CID_MIN_BUFFERS_FOR_CAPTURE:
		ctrl->val = NUM_MIN_BUFFERS;
		break;
	default:
		ret = -EINVAL;
	}
	return 0;
}

static const struct v4l2_ctrl_ops sta_vip_ctrl_ops = {
	.g_volatile_ctrl = sta_vip_g_v_ctrl,
};

static int sta_vip_init_controls(struct sta_vip *vip)
{
	int i;

	if (vip->v4l2_ctrls_is_configured)
		return 0;

	v4l2_ctrl_handler_init(&vip->ctrl_hdl, VIP_MAX_CTRLS);
	if (vip->ctrl_hdl.error) {
		int err = vip->ctrl_hdl.error;

		dev_err(vip->v4l2_dev.dev, "v4l2_ctrl_handler_init failed\n");
		v4l2_ctrl_handler_free(&vip->ctrl_hdl);
		return err;
	}

	vip->v4l2_dev.ctrl_handler = &vip->ctrl_hdl;
	for (i = 0; i < NUM_CTRLS; i++) {
		vip->ctrls[i] =
			v4l2_ctrl_new_std(&vip->ctrl_hdl,
					  &sta_vip_ctrl_ops,
					  controls[i].id, controls[i].minimum,
					  controls[i].maximum, controls[i].step,
					  controls[i].default_value);
		if (vip->ctrl_hdl.error) {
			dev_err(vip->v4l2_dev.dev,
				"Adding control (%d) failed\n", i);
			return vip->ctrl_hdl.error;
		}
		if (controls[i].is_volatile && vip->ctrls[i])
			vip->ctrls[i]->flags |= V4L2_CTRL_FLAG_VOLATILE;
	}

	vip->v4l2_ctrls_is_configured = true;
	return 0;
}

void sta_vip_delete_controls(struct sta_vip *vip)
{
	int i;

	if (vip->v4l2_ctrls_is_configured) {
		v4l2_ctrl_handler_free(&vip->ctrl_hdl);
		for (i = 0; i < NUM_CTRLS; i++)
			vip->ctrls[i] = NULL;
		vip->v4l2_ctrls_is_configured = false;
	}
}

static struct vb2_ops vip_video_qops = {
	.queue_setup		= vip_queue_setup,
	.buf_init		= vip_buffer_init,
	.buf_prepare		= vip_buffer_prepare,
	.buf_queue		= vip_buffer_queue,
	.buf_finish		= vip_buffer_finish,
	.buf_cleanup		= vip_buffer_cleanup,
	.start_streaming	= vip_start_streaming,
	.stop_streaming		= vip_stop_streaming,
};

static int sta_vip_v4l2_open(struct file *filp)
{
	struct video_device *video_dev = video_devdata(filp);
	struct sta_vip *vip = video_get_drvdata(video_dev);

	sta_vip_hwinit(vip);
	/* Power on adv7182 */
	v4l2_subdev_call(vip->decoder, core, s_power, true);
	/* Get current standard */
	v4l2_subdev_call(vip->decoder, video, querystd, &vip->std);
	if (V4L2_STD_525_60 & vip->std)
		vip->format_out = formats_60[0];
	else
		vip->format_out = formats_50[0];

	/*  Copy Output to Capture */
	vip->format_cap = vip->format_out;

	return v4l2_fh_open(filp);
}

static int sta_vip_v4l2_release(struct file *filp)
{
	struct video_device *video_dev = video_devdata(filp);
	struct sta_vip *vip = video_get_drvdata(video_dev);
	int ret;

	/* Power off adv7182 */
	if (vip->decoder)
		v4l2_subdev_call(vip->decoder, core, s_power, false);
	sta_vip_hwreset(vip);

	ret = vb2_fop_release(filp);

	sta_sga_release(&vip->sga);
	return ret;
}

/* File Operations */
static const struct v4l2_file_operations sta_vip_fops = {
	.owner = THIS_MODULE,
	.open = sta_vip_v4l2_open,
	.release = sta_vip_v4l2_release,
	.unlocked_ioctl = video_ioctl2,
	.read = vb2_fop_read,
	.mmap = vb2_fop_mmap,
	.poll = vb2_fop_poll,
#ifndef CONFIG_MMU
	.get_unmapped_area = vb2_fop_get_unmapped_area,
#endif
};

/**
 * sta_vip_querycap - return capabilities of device
 * @file: descriptor of device
 * @cap: contains return values
 *
 * the capabilities of the device are returned
 *
 * return value: 0, no error.
 */
static int sta_vip_querycap(struct file *file, void *priv,
			    struct v4l2_capability *cap)
{
	const char vip_description[] = "sta Video Input Port";
	char bus_info[32] = "platform:";

	strcat(bus_info, DRV_NAME);
	strlcpy(cap->driver, DRV_NAME, sizeof(cap->driver));
	strlcpy(cap->card, vip_description, sizeof(cap->card));
	strlcpy(cap->bus_info, bus_info, sizeof(cap->bus_info));

	cap->device_caps = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_READWRITE |
		V4L2_CAP_STREAMING;
	cap->capabilities = cap->device_caps | V4L2_CAP_DEVICE_CAPS;

	return 0;
}

/**
 * sta_vip_s_std - set video standard
 * @file: descriptor of device
 * @std: contains standard to be set
 *
 * the video standard is set
 *
 * return value: 0, no error.
 *
 * -EIO, no input signal detected
 *
 * other, returned from video decoder.
 */
static int sta_vip_s_std(struct file *file, void *priv, v4l2_std_id std)
{
	struct sta_vip *vip = video_drvdata(file);
	int ret;

	ret = v4l2_subdev_call(vip->decoder, video, s_std, std);
	if (ret) {
		dev_err(vip->v4l2_dev.dev,
			"sta_vip_s_std failed - std: %s\n",
			v4l2_norm_to_name(std));
		return ret;
	}

	vip->std = std;
	if (V4L2_STD_525_60 & std)
		vip->format_out = formats_60[0];
	else
		vip->format_out = formats_50[0];
	vip->format_cap = vip->format_out;
	return 0;
}

/**
 * sta_vip_g_std - get video standard
 * @file: descriptor of device
 * @std: contains return values
 *
 * the current video standard is returned
 *
 * return value: 0, no error.
 */
static int sta_vip_g_std(struct file *file, void *priv, v4l2_std_id *std)
{
	struct sta_vip *vip = video_drvdata(file);

	*std = vip->std;
	return 0;
}

/**
 * sta_vip_querystd - get possible video standards
 * @file: descriptor of device
 * @std: contains return values
 *
 * all possible video standards are returned
 *
 * return value: delivered by video ADV routine.
 */
static int sta_vip_querystd(struct file *file, void *priv, v4l2_std_id *std)
{
	struct sta_vip *vip = video_drvdata(file);

	return v4l2_subdev_call(vip->decoder, video, querystd, std);
}

static int sta_vip_enum_input(struct file *file, void *priv,
			      struct v4l2_input *inp)
{
	if (inp->index >= 1)
		return -EINVAL;

	inp->type = V4L2_INPUT_TYPE_CAMERA;
	inp->std = V4L2_STD_ALL;
	sprintf(inp->name, "Camera %u", inp->index);

	return 0;
}

/**
 * sta_vip_s_input - set input line
 * @file: descriptor of device
 * @i: new input line number
 *
 * the current active input line is set
 *
 * return value: 0, no error.
 *
 * -EINVAL, line number out of range
 */
static int sta_vip_s_input(struct file *file, void *priv, unsigned int i)
{
	struct sta_vip *vip = video_drvdata(file);
	int ret;

	if (i > 15)
		return -EINVAL;
	ret = v4l2_subdev_call(vip->decoder, video, s_routing, i, 0, 0);

	if (!ret)
		vip->input = i;

	return 0;
}

/**
 * sta_vip_g_input - return input line
 * @file: descriptor of device
 * @i: returned input line number
 *
 * the current active input line is returned
 *
 * return value: always 0.
 */
static int sta_vip_g_input(struct file *file, void *priv, unsigned int *i)
{
	struct sta_vip *vip = video_drvdata(file);

	*i = vip->input;
	return 0;
}

/**
 * sta_vip_enum_fmt_vid_cap - return video capture format
 * @f: returned format information
 *
 * returns name and format of video capture
 * For now, only UYVY is supported
 *
 * return value: always 0.
 */
static int sta_vip_enum_fmt_vid_cap(struct file *file, void *priv,
				    struct v4l2_fmtdesc *f)
{
	int i, num = 0;

	for (i = 0; i < VIP_NUM_FORMATS; i++) {
		if (num == f->index)
			break;
		++num;
	}

	/* No more output */
	if (i >= VIP_NUM_FORMATS)
		return -EINVAL;

	strlcpy(f->description, sta_cap_formats[i].name,
		sizeof(f->description));
	f->pixelformat = sta_cap_formats[i].fourcc;
	f->flags = 0;

	return 0;
}

/**
 * sta_vip_try_fmt_vid_cap - set video capture format
 * @file: descriptor of device
 * @f: new format
 *
 * new video format is set which includes width and
 * field type. width is fixed to 720, no scaling.
 * Only UYVY is supported by this hardware.
 * the maximum is 576 (PAL)
 *
 * return value: 0, no error
 *
 * -EINVAL, pixel or field format not supported
 *
 */
static int sta_vip_try_fmt_vid_cap(struct file *file, void *priv,
				   struct v4l2_format *f)
{
	struct sta_vip *vip = video_drvdata(file);
	int interlace_lim;
	int i;

	for (i = 0; i < VIP_NUM_FORMATS; i++) {
		if (sta_cap_formats[i].fourcc == f->fmt.pix.pixelformat)
			break;
	}

	if (i >= VIP_NUM_FORMATS) {
		v4l2_warn(&vip->v4l2_dev,
			  "Fourcc format (0x%08x) invalid.\n",
			  f->fmt.pix.pixelformat);
		return -EINVAL;
	}

	if (V4L2_STD_525_60 & vip->std)
		interlace_lim = FORMATS_60_HEIGHT;
	else
		interlace_lim = FORMATS_50_HEIGHT;

	if (f->fmt.pix.height > interlace_lim)
		f->fmt.pix.height = interlace_lim;

	/* Start/End pixel position must be word aligned =>even width */
	f->fmt.pix.pixelformat = sta_cap_formats[i].fourcc;
	f->fmt.pix.width &= ~1;
	if (f->fmt.pix.width > FORMATS_WIDTH)
		f->fmt.pix.width = FORMATS_WIDTH;
	f->fmt.pix.bytesperline = f->fmt.pix.width * sta_cap_formats[i].bpp;
	f->fmt.pix.sizeimage = f->fmt.pix.bytesperline * f->fmt.pix.height;
	f->fmt.pix.colorspace = V4L2_COLORSPACE_SMPTE170M;
	f->fmt.pix.field = vip->field;
	f->fmt.pix.priv = 0;
	return 0;
}

/**
 * sta_vip_s_fmt_vid_cap - set current video format parameters
 * @file: descriptor of device
 * @f: returned format information
 *
 * set new capture format
 * return value: 0, no error
 *
 * other, delivered by video decoder routine.
 */
static int sta_vip_s_fmt_vid_cap(struct file *file, void *priv,
				 struct v4l2_format *f)
{
	struct sta_vip *vip = video_drvdata(file);
	unsigned long flags;
	u32 max_height;
	int ret;

	ret = sta_vip_try_fmt_vid_cap(file, priv, f);
	if (ret)
		return ret;

	if (vb2_is_busy(&vip->vq)) {
		/* Can't change format during acquisition */
		v4l2_err(&vip->v4l2_dev, "device busy\n");
		return -EBUSY;
	}

	vip->format_cap = f->fmt.pix;

	spin_lock_irqsave(&vip->rlock, flags);
	vip->ctrl &= ~(CTRL_IF_TRANS_MASK  | CTRL_CROP_SELECT);

	switch (vip->format_out.pixelformat) {
	case V4L2_PIX_FMT_UYVY:
		vip->ctrl |= CTRL_IF_TRANS(YUVCBYCRY);
		break;
	default:
		/* Other format not yet supported */
		break;
	}

	max_height = (V4L2_STD_525_60 & vip->std) ? FORMATS_60_HEIGHT :
		FORMATS_50_HEIGHT;
	/* Use crop feature ? */
	if ((vip->format_cap.height != max_height) ||
	    (vip->format_cap.width != FORMATS_WIDTH)) {
		/* Top coordinate: 0,0 */
		reg_vip_write(vip, VIP_CSTARTPR,
			      (vip->format_out.width - vip->format_cap.width));
		/* Bottom coordinate: width, height */
		/* [Out + (Out - Cap) / 2] * 2 = Out + Cap */
		reg_vip_write(vip, VIP_CSTOPPR, (max_height << 16) |
			      (vip->format_out.width +
			       vip->format_cap.width));
		/* Enable croping */
		vip->ctrl |= CTRL_CROP_SELECT;
	}

	reg_vip_write(vip, VIP_CTRL, vip->ctrl);
	spin_unlock_irqrestore(&vip->rlock, flags);

	return 0;
}

/**
 * sta_vip_g_fmt_vid_cap - get current video format parameters
 * @file: descriptor of device
 * @f: contains format information
 *
 * returns current video format parameters
 *
 * return value: 0, always successful
 */
static int sta_vip_g_fmt_vid_cap(struct file *file, void *priv,
				 struct v4l2_format *f)
{
	struct sta_vip *vip = video_drvdata(file);

	f->fmt.pix = vip->format_cap;

	return 0;
}

static int sta_vip_subscribe_event(struct v4l2_fh *fh,
				   const struct v4l2_event_subscription *sub)
{
	switch (sub->type) {
	case V4L2_EVENT_SOURCE_CHANGE:
		return v4l2_src_change_event_subscribe(fh, sub);
	}
	return v4l2_ctrl_subscribe_event(fh, sub);
}

static const struct v4l2_ioctl_ops sta_vip_ioctl_ops = {
	.vidioc_querycap = sta_vip_querycap,
	/* FMT handling */
	.vidioc_enum_fmt_vid_cap = sta_vip_enum_fmt_vid_cap,
	.vidioc_g_fmt_vid_cap = sta_vip_g_fmt_vid_cap,
	.vidioc_s_fmt_vid_cap = sta_vip_s_fmt_vid_cap,
	.vidioc_try_fmt_vid_cap = sta_vip_try_fmt_vid_cap,
	/* Buffer handlers */
	.vidioc_create_bufs = vb2_ioctl_create_bufs,
	.vidioc_prepare_buf = vb2_ioctl_prepare_buf,
	.vidioc_reqbufs = vb2_ioctl_reqbufs,
	.vidioc_querybuf = vb2_ioctl_querybuf,
	.vidioc_expbuf = vb2_ioctl_expbuf,
	.vidioc_qbuf = vb2_ioctl_qbuf,
	.vidioc_dqbuf = vb2_ioctl_dqbuf,
	/* Stream on/off */
	.vidioc_streamon = vb2_ioctl_streamon,
	.vidioc_streamoff = vb2_ioctl_streamoff,
	/* Standard handling */
	.vidioc_g_std = sta_vip_g_std,
	.vidioc_s_std = sta_vip_s_std,
	.vidioc_querystd = sta_vip_querystd,
	/* Input handling */
	.vidioc_enum_input = sta_vip_enum_input,
	.vidioc_g_input = sta_vip_g_input,
	.vidioc_s_input = sta_vip_s_input,
	/* Log status ioctl */
	.vidioc_log_status = v4l2_ctrl_log_status,
	/* Event handling */
	.vidioc_subscribe_event = sta_vip_subscribe_event,
	.vidioc_unsubscribe_event = v4l2_event_unsubscribe,
};

static struct video_device sta_vip_vdev_template = {
	.name = DRV_NAME,
	.tvnorms = V4L2_STD_ALL,
	.fops = &sta_vip_fops,
	.ioctl_ops = &sta_vip_ioctl_ops,
	.release = video_device_release_empty,
};

static int sta_vip_init_buffer(struct sta_vip *vip)
{
	int err;

	memset(&vip->vq, 0, sizeof(struct vb2_queue));
	vip->vq.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	vip->vq.io_modes = VB2_MMAP | VB2_READ | VB2_USERPTR | VB2_DMABUF;
	vip->vq.drv_priv = vip;
	vip->vq.buf_struct_size = sizeof(struct vip_buffer);
	vip->vq.ops = &vip_video_qops;
	vip->vq.timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC;
	vip->vq.mem_ops = &vb2_dma_contig_memops;
	vip->vq.lock = &vip->mutex;
	vip->vq.dev = &vip->pdev->dev;
	err = vb2_queue_init(&vip->vq);
	if (err)
		return err;
	INIT_LIST_HEAD(&vip->capture);
	mutex_init(&vip->mutex);
	spin_lock_init(&vip->lock);

	return 0;
}

static void sta_vip_release_buffer(struct sta_vip *vip)
{
	vb2_queue_release(&vip->vq);
}

static void sta_vip_notify(struct v4l2_subdev *sd,
			   unsigned int notification, void *arg)
{
	struct sta_vip *vip =
		container_of(sd->v4l2_dev, struct sta_vip, v4l2_dev);

	switch (notification) {
	case V4L2_DEVICE_NOTIFY_EVENT:
		dev_dbg(vip->v4l2_dev.dev, "V4L2_DEVICE_NOTIFY_EVENT\n");
		v4l2_event_queue(&vip->video_dev, arg);
		break;
	default:
		break;
	}
}

static int sta_vip_v4l2_probe(struct sta_vip *vip)
{
	struct sta_sga *sga = &vip->sga;
	struct platform_device *pdev = vip->pdev;
	struct clk *clk;
	struct device_node *np = pdev->dev.of_node;
	int ret = 0;

	if (!vip->decoder) {
		dev_err(&pdev->dev, "Subdevice not bind\n");
		return -EIO;
	}

	/* Power on video decoder slave device */
	vip->pwr_pin = of_get_named_gpio(np, "pwr-gpio", 0);
	if (gpio_is_valid(vip->pwr_pin)) {
		if (devm_gpio_request(&pdev->dev, vip->pwr_pin,
				      "vip_slave_pwr"))
			dev_err(&pdev->dev, "could not request %d gpio\n",
				vip->pwr_pin);
		else
			gpio_direction_output(vip->pwr_pin, 1);
	}
	vip->reset_pin = of_get_named_gpio(np, "reset-gpio", 0);
	if (gpio_is_valid(vip->reset_pin)) {
		if (devm_gpio_request(&pdev->dev, vip->reset_pin,
				      "vip_slave_rst"))
			dev_err(&pdev->dev, "could not request %d gpio\n",
				vip->reset_pin);
		else {
			/* Datasheet says 5ms between PWR and RST */
			usleep_range(5000, 25000);
			gpio_direction_output(vip->reset_pin, 1);
			usleep_range(5000, 25000);
		}
	}

	/* get reference to device clock and enable it */
	clk = devm_clk_get(&pdev->dev, "apb_pclk");
	if (IS_ERR(clk)) {
		dev_err(&pdev->dev,
			"unable to get reference to VIP device clock\n");
		return PTR_ERR(clk);
	}

	ret = clk_prepare_enable(clk);
	if (ret) {
		dev_err(&pdev->dev, "failed to enable device clock\n");
		return ret;
	}
	vip->clk = clk;

	/* get reference to device clock and enable it */
	clk = devm_clk_get(&pdev->dev, "sga_clk");
	if (IS_ERR(clk)) {
		dev_err(&pdev->dev,
			"unable to get reference to SGA device clock\n");
		ret = PTR_ERR(clk);
		goto exit_vip_clk_disable;
	}

	ret = clk_prepare_enable(clk);
	if (ret) {
		dev_err(&pdev->dev, "failed to enable SGA device clock\n");
		goto exit_vip_clk_disable;
	}
	sga->clk = clk;

	ret = vb2_dma_contig_set_max_seg_size(&pdev->dev, DMA_BIT_MASK(32));
	if (ret) {
		dev_err(&pdev->dev, "failed to configure DMA max seg size\n");
		goto exit_sga_clk_disable;
	}

#ifdef DEBUG
	ret = devm_request_irq(&pdev->dev, vip->line_irq,
			       vip_line_irq, 0, "vip_line", vip);
	if (ret) {
		dev_err(&pdev->dev, "failed to allocate line IRQ %d\n",
			vip->line_irq);
		goto exit_vb2_clr_seg;
	}
#endif

	ret = devm_request_threaded_irq(&pdev->dev, vip->vsync_irq,
					vip_vsync_irq, vip_vsync_irq_thread,
					IRQF_ONESHOT, "vip_vsync", vip);
	if (ret) {
		dev_err(&pdev->dev, "failed to allocate vsync IRQ %d\n",
			vip->vsync_irq);
		goto exit_vb2_clr_seg;
	}

	ret = devm_request_threaded_irq(&pdev->dev, sga->irq,
					NULL, sga_irq, IRQF_ONESHOT, "sga_irq",
					sga);
	if (ret) {
		dev_err(&pdev->dev, "failed to allocate SGA IRQ %d\n",
			sga->irq);
		goto exit_vb2_clr_seg;
	}

	ret = v4l2_ctrl_add_handler(&vip->ctrl_hdl, vip->decoder->ctrl_handler,
				    NULL);
	if (ret) {
		dev_err(vip->v4l2_dev.dev, "failed to add subdev handler\n");
		goto exit_vb2_clr_seg;
	}

	/* initialize video device */
	vip->video_dev = sta_vip_vdev_template;
	vip->video_dev.v4l2_dev = &vip->v4l2_dev;
	vip->video_dev.queue = &vip->vq;
	vip->video_dev.lock = &vip->mutex;

	ret = video_register_device(&vip->video_dev, VFL_TYPE_GRABBER, -1);
	if (ret) {
		dev_err(vip->v4l2_dev.dev, "failed to register vip device\n");
		goto exit_vb2_clr_seg;
	}

	video_set_drvdata(&vip->video_dev, vip);

	vip->v4l2_dev.notify = sta_vip_notify;
	sta_vip_debugfs_create(vip);

	dev_info(vip->v4l2_dev.dev,
		 "registered as /dev/video%d\n", vip->video_dev.num);

	return 0;

exit_vb2_clr_seg:
	vb2_dma_contig_clear_max_seg_size(&pdev->dev);
exit_sga_clk_disable:
	clk_disable_unprepare(sga->clk);
exit_vip_clk_disable:
	clk_disable_unprepare(vip->clk);
	return ret;
}

static void sta_vip_v4l2_remove(struct sta_vip *vip)
{
	dev_dbg(vip->v4l2_dev.dev, "Removing %s\n",
		video_device_node_name(&vip->video_dev));

	vip->v4l2_dev.notify = NULL;

	sta_vip_debugfs_remove(vip);
	sta_vip_delete_controls(vip);
	video_set_drvdata(&vip->video_dev, NULL);
	video_unregister_device(&vip->video_dev);
	vb2_dma_contig_clear_max_seg_size(vip->v4l2_dev.dev);
	clk_disable_unprepare(vip->sga.clk);
	clk_disable_unprepare(vip->clk);
}

static int sta_vip_notify_complete(struct v4l2_async_notifier *notifier)
{
	struct sta_vip *vip = container_of(notifier, struct sta_vip, notifier);
	int ret = 0;

	ret = sta_vip_v4l2_probe(vip);
	if (ret < 0) {
		dev_err(vip->v4l2_dev.dev, "Failed to complete probing\n");
		return ret;
	}

	ret = v4l2_device_register_subdev_nodes(&vip->v4l2_dev);
	if (ret < 0) {
		dev_err(vip->v4l2_dev.dev, "Failed to register subdev nodes\n");
		return ret;
	}

	ret = v4l2_subdev_call(vip->decoder, video, s_routing, vip->input,
			       0, 0);
	if (ret) {
		dev_err(vip->v4l2_dev.dev, "failed to set default input (%d)\n",
			vip->input);
		return ret;
	}

	return 0;
}

static void sta_vip_notify_unbind(struct v4l2_async_notifier *notifier,
				  struct v4l2_subdev *subdev,
				  struct v4l2_async_subdev *asd)
{
	struct sta_vip *vip = container_of(notifier, struct sta_vip, notifier);

	if (vip->decoder == subdev) {
		dev_info(vip->v4l2_dev.dev, "unbind subdev\n");
		sta_vip_v4l2_remove(vip);
		vip->decoder = NULL;
		return;
	}

	dev_err(vip->v4l2_dev.dev, "no entity for subdev %s to unbind\n",
		subdev->name);
}

static int sta_vip_notify_bound(struct v4l2_async_notifier *notifier,
				struct v4l2_subdev *subdev,
				struct v4l2_async_subdev *asd)
{
	struct sta_vip *vip = container_of(notifier, struct sta_vip, notifier);

	v4l2_set_subdev_hostdata(subdev, vip);

	if (vip->asd.match.of.node == subdev->dev->of_node) {
		dev_info(vip->v4l2_dev.dev, "bound subdev %s\n", subdev->name);
		vip->decoder = subdev;
		return 0;
	}

	dev_err(vip->v4l2_dev.dev, "no entity for subdev %s to bind\n",
		subdev->name);
	return -EINVAL;
}

static int sta_vip_parse_nodes(struct sta_vip *vip)
{
	struct device_node *node = NULL;
	struct device *dev = &vip->pdev->dev;

	node = of_graph_get_next_endpoint(dev->of_node, node);
	if (!node) {
		dev_err(dev, "Could not find the endpoint in DT\n");
		return -EINVAL;
	}

	vip->asd.match.of.node = of_graph_get_remote_port_parent(node);
	vip->asd.match_type = V4L2_ASYNC_MATCH_OF;

	dev_info(dev, "Found subdevice %s\n",
		 of_node_full_name(vip->asd.match.of.node));

	vip->notifier.subdevs[0] = &vip->asd;
	vip->notifier.num_subdevs = 1;

	node = of_graph_get_next_endpoint(dev->of_node, node);
	if (node) {
		of_node_put(node);
		dev_err(dev, "Only one subdev supported\n");
		return -EINVAL;
	}

	return 0;
}

static void sta_vip_unmap_buffers(struct sta_vip *vip)
{
	struct sta_sga *sga = &vip->sga;
	struct platform_device *pdev = vip->pdev;
	int i = 0;

	for (i = 0; i < SGA_MAX_AVAIL_BATCHES; i++) {
		if (sga->dma_batches[i])
			dma_unmap_single(&pdev->dev,
					 sga->dma_batches[i],
					 SGA_BATCH_DEFAULT_SIZE,
					 DMA_TO_DEVICE);
		sga->dma_batches[i] = 0;
	}

	if (sga->dma_warmup)
		dma_unmap_single(&pdev->dev,
				 sga->dma_warmup,
				 SGA_BATCH_DEFAULT_SIZE,
				 DMA_TO_DEVICE);
	sga->dma_warmup = 0;

	if (sga->dma_main_fw)
		dma_unmap_single(&pdev->dev,
				 sga->dma_main_fw,
				 SGA_MAIN_FIRMWARE_SIZE,
				 DMA_TO_DEVICE);
	sga->dma_main_fw = 0;
}

static int sta_vip_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct sta_vip *vip;
	struct sta_sga *sga;
	struct device_node *np = pdev->dev.of_node;
	struct v4l2_async_subdev **subdevs = NULL;
	int ret;
	int i = 0;

	if (!np) {
		dev_err(&pdev->dev, "Device tree support is mandatory\n");
		return -EINVAL;
	}

	vip = devm_kzalloc(&pdev->dev, sizeof(struct sta_vip), GFP_KERNEL);
	if (!vip)
		return -ENOMEM;

	sga = &vip->sga;
	vip->pdev = pdev;

	of_property_read_u32(np, "input", &vip->input);

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "vip_regs");
	vip->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(vip->base)) {
		dev_err(&pdev->dev, "unable to remap register area\n");
		return PTR_ERR(vip->base);
	}
	vip->phys = res->start;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "sga_regs");
	sga->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(sga->base)) {
		dev_err(&pdev->dev, "unable to remap register area\n");
		return PTR_ERR(sga->base);
	}

	vip->line_irq = irq_of_parse_and_map(np, 0);
	if (vip->line_irq <= 0) {
		dev_err(&pdev->dev, "vip_line_irq resource not defined\n");
		return -EINVAL;
	}

	vip->vsync_irq = irq_of_parse_and_map(np, 1);
	if (vip->vsync_irq <= 0) {
		dev_err(&pdev->dev, "vip_vsync_irq resource not defined\n");
		return -EINVAL;
	}

	sga->irq = irq_of_parse_and_map(np, 2);
	if (sga->irq <= 0) {
		dev_err(&pdev->dev, "sga_irq resource not defined\n");
		return -EINVAL;
	}

	/* SGA Main FW allocation */
	sga->main_fw = devm_kzalloc(&pdev->dev, SGA_MAIN_FIRMWARE_SIZE,
				    GFP_KERNEL);
	if (!sga->main_fw) {
		dev_err(&pdev->dev, "Cannot allocate main_fw batch\n");
		return -ENOMEM;
	}

	/* Warmup batch allocation */
	sga->warmup = devm_kzalloc(&pdev->dev, SGA_WARMUP_BATCH_SIZE,
				   GFP_KERNEL);
	if (!sga->warmup) {
		dev_err(&pdev->dev, "Cannot allocate warmup batch\n");
		return -ENOMEM;
	}

	/* SGA Main FW DMA mapping */
	sga->dma_main_fw = dma_map_single(&pdev->dev,
					  (void *)sga->main_fw,
					  SGA_MAIN_FIRMWARE_SIZE,
					  DMA_TO_DEVICE);

	if (dma_mapping_error(&pdev->dev, sga->dma_main_fw)) {
		dev_err(&pdev->dev, "Unable to map main_fw address\n");
		ret = -EPERM;
		goto exit_unmap_buffers;
	}

	/* SGA Warmup batch DMA mapping */
	sga->dma_warmup = dma_map_single(&pdev->dev,
					 sga->warmup,
					 SGA_WARMUP_BATCH_SIZE,
					 DMA_TO_DEVICE);

	if (dma_mapping_error(&pdev->dev, sga->dma_warmup)) {
		dev_err(&pdev->dev, "Unable to map warmup address\n");
		ret = -EPERM;
		goto exit_unmap_buffers;
	}

	/* Allocate and map SGA batches */
	for (i = 0; i < SGA_MAX_AVAIL_BATCHES; i++) {
		sga->batches[i] = devm_kzalloc(&pdev->dev,
					       SGA_BATCH_DEFAULT_SIZE,
					       GFP_KERNEL);

		if (!sga->batches[i]) {
			dev_err(&pdev->dev, "Cannot allocate batch %d\n",
				i);
			ret = -ENOMEM;
			goto exit_unmap_buffers;
		}

		sga->dma_batches[i] = dma_map_single(&pdev->dev,
						     (void *)sga->batches[i],
						     SGA_BATCH_DEFAULT_SIZE,
						     DMA_TO_DEVICE);

		if (dma_mapping_error(&pdev->dev,
				      sga->dma_batches[i])) {
			dev_err(&pdev->dev,
				"Unable to map batch address\n");

			ret = -EPERM;
			goto exit_unmap_buffers;
		}
	}

	ret = v4l2_device_register(&pdev->dev, &vip->v4l2_dev);
	if (ret)
		goto exit_unmap_buffers;

	/* Initialize buffer */
	ret = sta_vip_init_buffer(vip);
	if (ret)
		goto exit_v4l2_device_unregister;

	ret = sta_vip_init_controls(vip);
	if (ret) {
		dev_err(vip->v4l2_dev.dev, "failed to init v4l2 controls\n");
		goto exit_release_buf;
	}

	INIT_DELAYED_WORK(&vip->work, vip_start_work);

	dev_info(&pdev->dev, "started at vip:0x%p, line IRQ %d, vsync IRQ %d\n",
		 vip->base, vip->line_irq, vip->vsync_irq);
	dev_info(&pdev->dev, "started at sga:0x%p, IRQ %d\n",
		 sga->base, sga->irq);

	vip->rlock = __SPIN_LOCK_UNLOCKED(vip.rlock);
	vip->lock = __SPIN_LOCK_UNLOCKED(vip.lock);
	sga->rlock = __SPIN_LOCK_UNLOCKED(sga.rlock);

	vip->v4l2_ctrls_is_configured = false;

	vip->field = VIP_FIELD;

	/* Register the subdevice notifier */
	subdevs = devm_kzalloc(&pdev->dev, sizeof(*subdevs), GFP_KERNEL);
	if (!subdevs) {
		ret = -ENOMEM;
		dev_err(&pdev->dev, "failed to alloc subdev structure\n");
		goto exit_v4l2_del_ctrls;
	}

	vip->notifier.num_subdevs = 0;
	vip->notifier.subdevs = subdevs;
	vip->notifier.bound = sta_vip_notify_bound;
	vip->notifier.unbind = sta_vip_notify_unbind;
	vip->notifier.complete = sta_vip_notify_complete;

	ret = sta_vip_parse_nodes(vip);
	if (ret) {
		dev_err(&pdev->dev, "Not able to find subdev in DT\n");
		goto exit_v4l2_del_ctrls;
	}

	ret = v4l2_async_notifier_register(&vip->v4l2_dev, &vip->notifier);
	if (ret < 0) {
		dev_err(&pdev->dev, "v4l2_async_notifier_register FAILED\n");
		goto exit_v4l2_del_ctrls;
	}

	return 0;

exit_v4l2_del_ctrls:
	sta_vip_delete_controls(vip);
exit_release_buf:
	sta_vip_release_buffer(vip);
exit_v4l2_device_unregister:
	v4l2_device_unregister(&vip->v4l2_dev);
exit_unmap_buffers:
	sta_vip_unmap_buffers(vip);
	return ret;
}

static int sta_vip_remove(struct platform_device *pdev)
{
	struct v4l2_device *v4l2_dev = platform_get_drvdata(pdev);
	struct sta_vip *vip =
		container_of(v4l2_dev, struct sta_vip, v4l2_dev);

	v4l2_async_notifier_unregister(&vip->notifier);
	sta_vip_delete_controls(vip);
	sta_vip_release_buffer(vip);
	v4l2_device_unregister(&vip->v4l2_dev);
	sta_vip_unmap_buffers(vip);
	return 0;
}

static int sta_vip_runtime_suspend(struct device *dev)
{
	/* TODO: to be completed */
	return 0;
}

static int sta_vip_runtime_resume(struct device *dev)
{
	/* TODO: to be completed */
	return 0;
}

static const struct dev_pm_ops sta_vip_pm_ops = {
	.runtime_suspend = sta_vip_runtime_suspend,
	.runtime_resume = sta_vip_runtime_resume,
};

static const struct of_device_id vip_of_match[] = {
	{ .compatible = "st,sta_vip" },
	{ }
};

MODULE_DEVICE_TABLE(of, vip_of_match);

static struct platform_driver sta_vip_driver = {
	.driver = {
		.name	= DRV_NAME,
		.pm	= &sta_vip_pm_ops,
		.of_match_table = vip_of_match,
	},
	.probe		= sta_vip_probe,
	.remove		= sta_vip_remove,
};

static int __init sta_vip_init(void)
{
	return platform_driver_register(&sta_vip_driver);
}

static void __exit sta_vip_exit(void)
{
	platform_driver_unregister(&sta_vip_driver);
}

#ifdef MODULE

module_init(sta_vip_init);
module_exit(sta_vip_exit);
#else
late_initcall_sync(sta_vip_init);
#endif

MODULE_DESCRIPTION("STA Video Input Port driver");
MODULE_AUTHOR("ST Automotive Group");
MODULE_AUTHOR("Pierre-Yves MORDRET <pierre-yves.mordret@st.com>");
MODULE_AUTHOR("Philippe Langlais <philippe.langlais@st.com>");
MODULE_AUTHOR("Sandeep Kaushik <sandeep-mmc.kaushik@st.com>");
MODULE_VERSION(DRV_VERSION);
MODULE_LICENSE("GPL v2");
