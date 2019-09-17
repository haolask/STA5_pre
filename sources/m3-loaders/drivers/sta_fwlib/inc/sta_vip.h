/**
 * @file sta_vip.h
 * @brief Video Input Port utilities header
 *
 * Copyright (C) ST-Microelectronics SA 2016
 * @author: APG-MID team
 */

#ifndef _VIP_H_
#define _VIP_H_

/*  Definitions */

/*
 * VIP register map (offsets)
 */
#define VIP_CTRL	0x00	/* Control Register                   */
#define VIP_EFECR	0x04	/* Even Field Embedded Codes Register */
#define VIP_CSTARTPR	0x08	/* Crop Start Point Register          */
#define VIP_CSTOPPR	0x0C	/* Crop Stop Point Register           */
#define VIP_MASK	0x10	/* Interrupt & DMA Mask Register      */
#define VIP_STATUS	0x14	/* Interrupt Status Register          */
#define VIP_DMAPR	0x18	/* DMA Pointer Register               */
#define VIP_OFECR	0x1C	/* Odd Field Embedded Codes Register  */

/* ITU656: Even Field Embedded Codes Register:
31 - 24: Start of Active Video, Blanking Area (F=0, V=1, H=0).
23 - 16: End of Active Video, Blanking Area (F=0, V=1, H=1).
15 - 8: Start of Active Video, Active Area (F=0, V=0, H=0).
7 - 0: End of Active Video, Active Area (F=0, V=0, H=1) */
#define ITU656_EMBEDDED_EVEN_CODE	0xABB6809D
/* ITU656: Odd Field Embedded Codes Register:
31 - 24: Start of Active Video, Blanking Area (F=1, V=1, H=0).
23 - 16: End of Active Video, Blanking Area (F=1, V=1, H=1).
15 - 8: Start of Active Video, Active Area (F=1, V=0, H=0).
7 - 0: End of Active Video, Active Area (F=1, V=0, H=1) */
#define ITU656_EMBEDDED_ODD_CODE	0xECF1C7DA

/*
 * Interrupt and Dma Mask Register
 */

#define IRQ_MASK_BREQ			(1 << 15)
#define IRQ_MASK_SREQ			(1 << 14)
#define IRQ_DUAL			(1 << 12)
#define IRQ_DMAEN			(1 << 11)
#define IRQ_FRAME_START			(1 << 6)
#define IRQ_FRAME_END			(1 << 5)
#define IRQ_LINE_END			(1 << 4)

#define IRQ_STATUS_FRAME_START_RAW	(1 << 18)
#define IRQ_STATUS_FRAME_END_RAW	(1 << 17)
#define IRQ_STATUS_FRAME_TYPE		(3 << 13)
#define IRQ_STATUS_FRAME_ODD		(1 << 13)
#define IRQ_STATUS_FRAME_TFR		(1 << 12)
#define IRQ_STATUS_FRAME_EVEN		(2 << 13)
#define IRQ_STATUS_FRAME_START		(1 << 6)
#define IRQ_STATUS_FRAME_END		(1 << 5)
#define IRQ_STATUS_LINE_END		(1 << 4)

#define IRQ_ENABLE (IRQ_FRAME_START | IRQ_FRAME_END  | IRQ_LINE_END)
#define IRQ_RESET 0x0


/* Enums */
typedef enum vip_error {
	VIP_OK				=  0, /* No error */
	VIP_INTERNAL_ERROR		= -1,
	VIP_NOT_CONFIGURED		= -2,
	VIP_REQUEST_NOT_APPLICABLE	= -3,
	VIP_INVALID_PARAMETER		= -4,
	VIP_UNSUPPORTED_FEATURE		= -5,
	VIP_UNSUPPORTED_HW		= -6,
	VIP_RESOURCE_NOT_AVIALABLE	= -7,
	VIP_MAIN_FIRMWARE_NOT_BUILD	= -8,
	VIP_GENERIC_FAILURE		= -9
} t_vip_error;

typedef enum vip_events {
	VIP_FRAME_START_EVEN,
	VIP_FRAME_END_EVEN,
	VIP_FRAME_START_ODD,
	VIP_FRAME_END_ODD,
	VIP_FRAME_MAX_EVENTS,
} t_vip_events;

struct sta_vip {
	unsigned long intstatus_prev;
	void (*notif)(t_vip_events ev);
#ifdef DEBUG
	unsigned line_cnt;
#endif
};

/* Procedure and Routine declarations */

void vip_deinit(void);
/**
 * @return	Error Condition (VIP_OK on success)
 */
t_vip_error vip_init(void);

t_vip_error vip_register_cb(
			    void (*func)(t_vip_events ev));

void vip_frame(void);

void vip_start_capture(void);
void vip_stop_capture(void);

#ifdef DEBUG
void vip_line(void);
#endif

#endif /* _VIP_H_ */
