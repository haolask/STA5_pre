/*
 * @file sta_ltdc.h
 *
 * Copyright (C) ST-Microelectronics SA 2016
 * @author: APG-MID team
 */

#ifndef STA_LTDC_H_
#define STA_LTDC_H_

#include "sta_type.h"

enum ltdc_pix_fmt {
	PF_NONE,
	/* RGB formats */
	PF_ARGB8888,    /* ARGB [32 bits] */
	PF_RGBA8888,    /* RGBA [32 bits] */
	PF_RGB888,      /* RGB [24 bits] */
	PF_RGB565,      /* RGB [16 bits] */
	PF_ARGB1555,    /* ARGB A:1 bit RGB:15 bits [16 bits] */
	PF_ARGB4444,    /* ARGB A:4 bits R/G/B: 4 bits each [16 bits] */
	/* Indexed formats */
	PF_L8,          /* Indexed 8 bits [8 bits] */
	PF_AL44,        /* Alpha:4 bits + indexed 4 bits [8 bits] */
	PF_AL88,         /* Alpha:8 bits + indexed 8 bits [16 bits] */
	/* YUV format added in this list to identify those kind of formats */
	PF_FORMAT_YUYV,
	PF_FORMAT_YVYU,
	PF_FORMAT_UYVY,
	PF_FORMAT_VYUY,
	PF_FORMAT_NV12,
	PF_FORMAT_NV21
};

enum display_sel {
	EVEN, /* select display connected to PIXCLK */
	ODD, /* select display connected to PIXCLK1 */
	BOTH /* duplicate on both displays */
};

struct framebuffer {
	uint32_t paddr; /* physical address of the frame buffer plane */
	unsigned int pitches[4]; /* nb of bytes per line of each buffer plane */
	unsigned int offsets[4]; /* start address of the plane = paddr + offset */
	enum ltdc_pix_fmt pixel_format; /* aligned on CLCD pixel format */

	/* position/size of the image of interest in the framebuffer */
	uint32_t width;
	uint32_t height;
	int x;
	int y;
};

enum display_flags {
	DISPLAY_FLAGS_HSYNC_LOW		= (1 << 0),
	DISPLAY_FLAGS_HSYNC_HIGH	= (1 << 1),
	DISPLAY_FLAGS_VSYNC_LOW		= (1 << 2),
	DISPLAY_FLAGS_VSYNC_HIGH	= (1 << 3),

	/* data enable flag */
	DISPLAY_FLAGS_DE_LOW		= (1 << 4),
	DISPLAY_FLAGS_DE_HIGH		= (1 << 5),
	/* drive data on pos. edge */
	DISPLAY_FLAGS_PIXDATA_POSEDGE	= (1 << 6),
	/* drive data on neg. edge */
	DISPLAY_FLAGS_PIXDATA_NEGEDGE	= (1 << 7),
	DISPLAY_FLAGS_INTERLACED	= (1 << 8),
	DISPLAY_FLAGS_DOUBLESCAN	= (1 << 9),
	DISPLAY_FLAGS_DOUBLECLK		= (1 << 10),
};

#define GAMMA_SIZE	256

struct videomode {
	unsigned long pixelclock;	/* pixelclock in Hz */
	uint32_t hactive;
	uint32_t hfront_porch;
	uint32_t hback_porch;
	uint32_t hsync_len;
	uint32_t vactive;
	uint32_t vfront_porch;
	uint32_t vback_porch;
	uint32_t vsync_len;
	enum display_flags flags;
};

/**
 * @brief	Initialize CLCD IP properties and configure output RGB GPIOs
 * @return	0 if no error, not 0 otherwise
 */
int ltdc_init(void);

/**
 * @brief	Set CLCD IP with display mode and send background
 * @param	pps_vm: video mode timings
 * @param	dual_display: true if dual display is selected
 * @return	0 if no error, not 0 otherwise
 */
int ltdc_mode_set(struct videomode *pps_vm, bool dual_display);

/**
 * @brief	Update a layer, enable the layer if needed
 * @param	fb: buffer charateristics
 * @param	lay_id: layer ID
 * @param	x0, y0: position of the frame buffer in the layer
 * @param	display: select display on which to insert this layer
 *			Note: Used only when dual display is selected
 * @return	0 if no error, not 0 otherwise
 */
int ltdc_layer_update(struct framebuffer *fb, uint32_t lay_id,
		      uint32_t x0, uint32_t y0, enum display_sel display);

/**
 * @brief	Disable a layer
 * @param	lay_id: layer ID
 * @return	0 if no error, not 0 otherwise
 */
int ltdc_layer_disable(uint32_t lay_id);

/**
 * @brief	De-initialize CLCD IP
 * @return	0 if no error, not 0 otherwise
 */
int ltdc_disable(void);

/**
 * @brief	Book a layer
 * @param	lay_id: layer ID
 * @return	0 if no error, not 0 otherwise
 */
int ltdc_layer_book(uint32_t lay_id);

/**
 * @brief	Release a layer previsously booked
 * @param	lay_id: layer ID
 * @return	0 if no error, not 0 otherwise
 */
int ltdc_layer_release(uint32_t lay_id);

/**
 * @brief	Provide the top layer that supports a given color format
 *		Lock this layer and upper layer if any
 * @param	pix_fmt: pixel format
 * @param	lay_id: layer ID
 * @return	0 if no error, not 0 otherwise
 */
int ltdc_layer_book_top_layer(enum ltdc_pix_fmt pix_fmt, uint32_t *lay_id);

/**
 * @brief	Release this layer and upper layer if any
 * @param	lay_id: layer ID
 * @return	0 if no error, not 0 otherwise
 */
int ltdc_layer_release_top_layer(uint32_t lay_id);

/**
 * @brief	Set display IP gamma table
 * @param	gam_r: gamma table for red component
 * @param	gam_g: gamma table for green component
 * @param	gam_b: gamma table for blue component
 * @return	0 if no error, not 0 otherwise
 * Note: each component gam_* should have a GAMMA_SIZE size
 */
int ltdc_update_gamma_correction(uint16_t *gam_r, uint16_t *gam_g, uint16_t *gam_b);
#endif /* STA_LTDC_H_ */
