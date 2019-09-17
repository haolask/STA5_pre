/*
 * Copyright (C) STMicroelectronics 2016
 *
 * Authors:	Gabriele Simone <gabriele.simone@st.com>,
 *		Gian Antonio Sampietro <gianantonio.sampietro@st.com>,
 *		Giancarlo Asnaghi <giancarlo.asnaghi@st.com>
 *		for STMicroelectronics.
 *
 * License terms:
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#ifndef STA_MSP_I2S_H
#define STA_MSP_I2S_H

#define MAX_MSP_I2S_NAME 48
#define MSP_FIFO_DEPTH 8

/* Registers */
#define MSP_DR		0x00
#define MSP_GCR		0x04
#define MSP_TCF		0x08
#define MSP_RCF		0x0c
#define MSP_SRG		0x10
#define MSP_FLR		0x14
#define MSP_DMACR	0x18

#define MSP_IMSC	0x20
#define MSP_RIS		0x24
#define MSP_MIS		0x28
#define MSP_ICR		0x2c
#define MSP_MCR		0x30
#define MSP_RCV		0x34
#define MSP_RCM		0x38

#define MSP_TCE0	0x40
#define MSP_TCE1	0x44
#define MSP_TCE2	0x48
#define MSP_TCE3	0x4c

#define MSP_RCE0	0x60
#define MSP_RCE1	0x64
#define MSP_RCE2	0x68
#define MSP_RCE3	0x6c
#define MSP_IODLY	0x70

#define MSP_ITCR	0x80
#define MSP_ITIP	0x84
#define MSP_ITOP	0x88
#define MSP_TSTDR	0x8c

#define MSP_PID0	0xfe0
#define MSP_PID1	0xfe4
#define MSP_PID2	0xfe8
#define MSP_PID3	0xfec

#define MSP_CID0	0xff0
#define MSP_CID1	0xff4
#define MSP_CID2	0xff8
#define MSP_CID3	0xffc

/* GCR shifts */
#define RFFEN_SHIFT		1
#define LBM_SHIFT		7
#define TFFEN_SHIFT		9
#define TXDDL_SHIFT		15

/* GCR masks */
#define RX_ENABLE				BIT(0)
#define RX_FIFO_ENABLE_MASK		BIT(1)
#define RX_FIFO_ENABLE			1
#define RX_FSYNC_POL_MASK		BIT(2)
#define RX_FSYNC_ACTIVE_LOW		RX_FSYNC_POL_MASK
#define RX_FSYNC_ACTIVE_HIGH	0
#define DIRECT_COMPANDING_MASK	BIT(3)
#define RX_FSYNC_SEL_MASK		BIT(4)
#define RX_FSYNC_FGEN			RX_FSYNC_SEL_MASK
#define RX_FSYNC_EXT			0
#define RX_CLK_POL_MASK			BIT(5)
#define RX_CLK_FALLING			0
#define RX_CLK_RISING			RX_CLK_POL_MASK
#define RX_CLK_SEL_MASK			BIT(6)
#define RX_CLK_SGEN				RX_CLK_SEL_MASK
#define RX_CLK_EXT				0
#define LOOPBACK_MASK			BIT(7)
#define TX_ENABLE				BIT(8)
#define TX_FIFO_ENABLE_MASK		BIT(9)
#define TX_FIFO_ENABLE			1
#define TX_FSYNC_POL_MASK		BIT(10)
#define TX_FSYNC_ACTIVE_LOW		TX_FSYNC_POL_MASK
#define TX_FSYNC_ACTIVE_HIGH	0
#define TX_FSYNC_SEL_MASK		GENMASK(12, 11)
#define TX_FSYNC_FGEN			TX_FSYNC_SEL_MASK
#define TX_FSYNC_EXT			0
#define TX_CLK_POL_MASK			BIT(13)
#define TX_CLK_FALLING			TX_CLK_POL_MASK
#define TX_CLK_RISING			0
#define TX_CLK_SEL_MASK			BIT(14)
#define TX_CLK_SGEN				TX_CLK_SEL_MASK
#define TX_CLK_EXT				0
#define TX_EXTRA_DELAY_MASK		BIT(15)
#define SRG_ENABLE_MASK			BIT(16)
#define SRG_ENABLE				BIT(16)
#define SRG_CLK_POL_MASK		BIT(17)
#define SRG_CLK_FALLING			SRG_CLK_POL_MASK
#define SRG_CLK_RISING			0
#define SRG_CLK_SEL_MASK		GENMASK(19, 18)
#define SRG_CLK_INT				0
#define SRG_CLK_EXT				BIT(19)
#define FRAME_GEN_EN_MASK		BIT(20)
#define FRAME_GEN_ENABLE		BIT(20)
#define SPI_CLK_MODE_MASK		GENMASK(22, 21)
#define SPI_BURST_MODE_MASK		BIT(23)

/* TCF masks */
#define TX_ELEM_LEN_1_MASK		(BIT(0) | BIT(1) | BIT(2))
#define	TX_FRAME_LEN_1_MASK		(BIT(3) | BIT(4) | BIT(5) | \
						BIT(6) | BIT(7) | \
						BIT(8) | BIT(9))
#define TX_FRAME_1(x)			(((x) - 1) & 0x7F)
#define	TX_DATA_TYPE_MASK		(BIT(10) | BIT(11))
#define	TX_DATA_ENDIANNESS_MASK		BIT(12)
#define	TX_DATA_DELAY_MASK		(BIT(13) | BIT(14))
#define TX_FS_IGNORE_MASK		BIT(15)
#define	TX_ELEM_LEN_2_MASK		(BIT(16) | BIT(17) | BIT(18))
#define	TX_FRAME_LEN_2_MASK		(BIT(19) | BIT(20) | BIT(21) | \
						BIT(22) | BIT(23) | \
						BIT(24) | BIT(25))
#define TX_FRAME_2(x)			(((x) - 1) & 0x7F)
#define	TX_PHASE2_START_MODE_MASK	BIT(26)
#define	TX_PHASE2_ENABLE_MASK		BIT(27)
#define	TX_HALF_WORD_SWAP_MASK		(BIT(28) | BIT(29))

/* TCF shifts */
#define TP1ELEN_SHIFT		0
#define TP1FLEN_SHIFT		3
#define TDTYP_SHIFT		10
#define TENDN_SHIFT		12
#define TDDLY_SHIFT		13
#define TFSIG_SHIFT		15
#define TP2ELEN_SHIFT		16
#define TP2FLEN_SHIFT		19
#define TP2SM_SHIFT		26
#define TP2EN_SHIFT		27
#define TBSWAP_SHIFT		28

/* RCF masks */
#define RX_ELEM_LEN_1_MASK		(BIT(0) | BIT(1) | BIT(2))
#define	RX_FRAME_LEN_1_MASK		(BIT(3) | BIT(4) | BIT(5) | \
						BIT(6) | BIT(7) | \
						BIT(8) | BIT(9))
#define RX_FRAME_1(x)			(((x) - 1) & 0x7F)
#define	RX_DATA_TYPE_MASK		(BIT(10) | BIT(11))
#define	RX_DATA_ENDIANNESS_MASK		BIT(12)
#define	RX_DATA_DELAY_MASK		(BIT(13) | BIT(14))
#define RX_FS_IGNORE_MASK		BIT(15)
#define	RX_ELEM_LEN_2_MASK		(BIT(16) | BIT(17) | BIT(18))
#define	RX_FRAME_LEN_2_MASK		(BIT(19) | BIT(20) | BIT(21) | \
						BIT(22) | BIT(23) | \
						BIT(24) | BIT(25))
#define RX_FRAME_2(x)			(((x) - 1) & 0x7F)
#define	RX_PHASE2_START_MODE_MASK	BIT(26)
#define	RX_PHASE2_ENABLE_MASK		BIT(27)
#define	RX_HALF_WORD_SWAP_MASK		(BIT(28) | BIT(29))

/* RCF shifts */
#define RP1ELEN_SHIFT		0
#define RP1FLEN_SHIFT		3
#define RDTYP_SHIFT		10
#define RENDN_SHIFT		12
#define RDDLY_SHIFT		13
#define RFSIG_SHIFT		15
#define RP2ELEN_SHIFT		16
#define RP2FLEN_SHIFT		19
#define RP2SM_SHIFT		26
#define RP2EN_SHIFT		27
#define RBSWAP_SHIFT		28

static inline s32 msp_elen(unsigned int bits)
{
	switch (bits) {
	case  8: return 0;
	case 10: return 1;
	case 12: return 2;
	case 14: return 3;
	case 16: return 4;
	case 20: return 5;
	case 24: return 6;
	case 32: return 7;
	default: return -EINVAL;
	}
}

/* Flag register */
#define RX_BUSY				BIT(0)
#define RX_FIFO_EMPTY			BIT(1)
#define RX_FIFO_FULL			BIT(2)
#define TX_BUSY				BIT(3)
#define TX_FIFO_EMPTY			BIT(4)
#define TX_FIFO_FULL			BIT(5)

/* Multichannel control register */
#define RMCEN_SHIFT		0
#define RMCSF_SHIFT		1
#define RCMPM_SHIFT		3
#define TMCEN_SHIFT		5
#define TNCSF_SHIFT		6

/* Sample rate generator register */
#define FRWID_SHIFT		10
#define FRPER_SHIFT		16

#define SCK_DIV_MASK		0x0000003FF
#define FRAME_WIDTH_BITS(n)	((((n) - 1) << FRWID_SHIFT)  & 0x0000FC00)
#define FRAME_PERIOD_BITS(n)	((((n) - 1) << FRPER_SHIFT) & 0x1FFF0000)

/* DMA controller register */
#define RX_DMA_ENABLE		BIT(0)
#define TX_DMA_ENABLE		BIT(1)

/* Interrupt Register */
#define RX_SERVICE_INT			BIT(0)
#define RX_OVERRUN_ERROR_INT		BIT(1)
#define RX_FSYNC_ERR_INT		BIT(2)
#define RX_FSYNC_INT			BIT(3)
#define TX_SERVICE_INT			BIT(4)
#define TX_UNDERRUN_ERR_INT		BIT(5)
#define TX_FSYNC_ERR_INT		BIT(6)
#define TX_FSYNC_INT			BIT(7)
#define ALL_ERR				0x00000066
#define ALL_INT				0x000000ff

/* MSP test control register */
#define MSP_ITCR_ITEN		BIT(0)
#define MSP_ITCR_TESTFIFO	BIT(1)

#define RMCEN_BIT   BIT(0)
#define RMCSF_BIT   BIT(1)
#define RCMPM_BIT   BIT(3)
#define TMCEN_BIT   BIT(5)
#define TNCSF_BIT   BIT(6)


#endif
