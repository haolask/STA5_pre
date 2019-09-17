/**
 * @file sta_tvdec.h
 * @brief  This file provides all the TVDEC functions definitions
 *
 * Copyright (C) ST-Microelectronics SA 2016
 * @author: APG-MID team
 */

#ifndef _STA_TVDEC_H
#define _STA_TVDEC_H

#include "sta_map.h"
#include "sta_i2c.h"
#include "sta_cam.h"

#define I2C_ADV7182_PORT		i2c2_regs
#define I2C_ADV7182_ADDR		0x40
#define I2C_ADV7182_CLOCK		51200000
#define I2C_ADV7182_READWRITE_DELAY	50

/* Cropping:
 *  - Reduce the value of TVDEC_ACTIVE_COL for horizontal cropping
 *  - Reduce the value of TVDEC_ACTIVE_LINES for vertical cropping
 */

#define TVDEC_NBCOL			720
#define TVDEC_ACTIVE_COL		720

#if (CAM_STANDARD == CAM_PAL)
	#ifdef CAM_NO_DEINTERLEAVING
		#define TVDEC_NBLINES			288 /* Half PAL 576i */
		#define TVDEC_ACTIVE_LINES		288
	#else
		#define TVDEC_NBLINES			576 /* PAL 576i */
		#define TVDEC_ACTIVE_LINES		576
	#endif
#elif (CAM_STANDARD == CAM_NTSC)
	#ifdef CAM_NO_DEINTERLEAVING
		#define TVDEC_NBLINES			240 /* Half NTSC 480i */
		#define TVDEC_ACTIVE_LINES		240
	#else
		#define TVDEC_NBLINES			480 /* NTSC 480i */
		#define TVDEC_ACTIVE_LINES		480
	#endif
#elif (CAM_STANDARD == CAM_AUTODETECT)
	#ifdef CAM_NO_DEINTERLEAVING
		#define TVDEC_NBLINES			240 /* Half 480i */
		#define TVDEC_ACTIVE_LINES		240
	#else
		#define TVDEC_NBLINES			480 /* 480i */
		#define TVDEC_ACTIVE_LINES		480
	#endif
#else
#error CAM_STANDARD must be defined
#endif

#define TVDEC_BYTES_PER_PIXEL		2

#define ADV7182_INPUT_CVBS_AIN1 0x00
#define ADV7182_INPUT_CVBS_AIN2 0x01
#define ADV7182_INPUT_CVBS_AIN3 0x02
#define ADV7182_INPUT_CVBS_AIN4 0x03
#define ADV7182_INPUT_CVBS_AIN5 0x04
#define ADV7182_INPUT_CVBS_AIN6 0x05
#define ADV7182_INPUT_CVBS_AIN7 0x06
#define ADV7182_INPUT_CVBS_AIN8 0x07
#define ADV7182_INPUT_SVIDEO_AIN1_AIN2 0x08
#define ADV7182_INPUT_SVIDEO_AIN3_AIN4 0x09
#define ADV7182_INPUT_SVIDEO_AIN5_AIN6 0x0a
#define ADV7182_INPUT_SVIDEO_AIN7_AIN8 0x0b
#define ADV7182_INPUT_YPRPB_AIN1_AIN2_AIN3 0x0c
#define ADV7182_INPUT_YPRPB_AIN4_AIN5_AIN6 0x0d
#define ADV7182_INPUT_DIFF_CVBS_AIN1_AIN2 0x0e
#define ADV7182_INPUT_DIFF_CVBS_AIN3_AIN4 0x0f
#define ADV7182_INPUT_DIFF_CVBS_AIN5_AIN6 0x10
#define ADV7182_INPUT_DIFF_CVBS_AIN7_AIN8 0x11

#define ADV7180_REG_STATUS1		0x0010
#define ADV7180_REG_STATUS2		0x0012
#define ADV7180_REG_STATUS3		0x0013
#define ADV7180_STATUS1_IN_LOCK		BIT(0)
#define ADV7180_STATUS1_LOST_LOCK	BIT(1)
#define ADV7180_STATUS1_FSC_LOCK	BIT(2)
#define ADV7180_STATUS1_AUTOD_MASK	0x70
#define ADV7180_STATUS1_AUTOD_NTSC_M_J	0x00
#define ADV7180_STATUS1_AUTOD_NTSC_4_43 0x10
#define ADV7180_STATUS1_AUTOD_PAL_M	0x20
#define ADV7180_STATUS1_AUTOD_PAL_60	0x30
#define ADV7180_STATUS1_AUTOD_PAL_B_G	0x40
#define ADV7180_STATUS1_AUTOD_SECAM	0x50
#define ADV7180_STATUS1_AUTOD_PAL_COMB	0x60
#define ADV7180_STATUS1_AUTOD_SECAM_525	0x70

/* I2C bus configuration */
static const struct i2c_config i2c2_config = {
	I2C_DGTLFILTER_OFF,
	I2C_FAST_MODE,
	I2C_ADV7182_CLOCK,
	8,
	8,
	I2C_ADV7182_ADDR >> 1,
	0,
	0,
	I2C_STARTPROC_DISABLED
};

typedef struct adv_tab {
	uint8_t addr;
	uint8_t data;
} t_adv_tab;

typedef enum {
	CVBS_NTSC,
	YPBPR_NTSC,
} tvdec_op;

typedef enum sensor_error {
	SENSOR_OK			=  0, /* No error */
	SENSOR_INTERNAL_ERROR		= -1,
	SENSOR_NOT_CONFIGURED		= -2,
	SENSOR_REQUEST_NOT_APPLICABLE	= -3,
	SENSOR_INVALID_PARAMETER	= -4,
	SENSOR_UNSUPPORTED_FEATURE	= -5,
	SENSOR_UNSUPPORTED_HW		= -6,
	SENSOR_RESOURCE_NOT_AVIALABLE	= -7,
	SENSOR_MAIN_FIRMWARE_NOT_BUILD	= -8,
	SENSOR_GENERIC_FAILURE		= -9
} t_sensor_error;

struct tvdec_ctx {
	void *rpmsg_handle;
	uint32_t input;
	uint32_t std;
};

t_sensor_error tv_dec_setup(uint32_t input, uint32_t *std);
t_sensor_error read_video_type(void);

int init_adv7182(tvdec_op Op, uint32_t input, uint32_t *std);

#endif

