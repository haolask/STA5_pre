/*
 * Copyright (C) STMicroelectronics 2016
 *
 * Authors:	Gabriele Simone <gabriele.simone@st.com>,
 *		Gian Antonio Sampietro <gianantonio.sampietro@st.com>
 *		Giancarlo Asnaghi <giancarlo.asnaghi@st.com>
 *		for STMicroelectronics.
 *
 * License terms:
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#ifndef ST_CODEC_AIF_H
#define ST_CODEC_AIF_H

#define MAX_NUM_AUDIOSOURCE	30
#define MAX_AUDIOSOURCE_NAME	32

/* channel-stream bitmask */
#define CHSTREAM_NONE		0
#define CHSTREAM_PLAYBACK	1
#define CHSTREAM_CAPTURE	2
#define CHSTREAM_RESERVED	3
#define CHSTREAM_MSP1		0x10
#define CHSTREAM_MSP2		0x20
#define CHSTREAM_MASK		0xFF

/* AIF registers */
#define AIF_DMABUS_CR		0x4000
#define AIF_DMABUS_TCM		0x6000
#define AIF_DMABUS_SIZE		512
#define AIF_SAI1_CSR		0x8C00
#define AIF_SAI2_CSR		0x8C04
#define AIF_SAI3_CSR		0x8C08
#define AIF_SAI4_CSR		0x8C0C
#define AIF_AIMUX		0x8800
#define AIF_LPF			0x8400
#define AIF_AOMUX		0x8804
#define AIF_ADCAUX_CR		0x10000

#define AUSS_BASE_ADDRESS	0x48060000
#define AUSS_ADCMIC_CR		0x0004
#define AUSS_DAC_CR		0x0010
#define AUSS_SSY_CR		0x0100
#define AUSS_MUX_CR		0x1000

/* SAI */
#define AIF_SAI_ENA_MASK	BIT(0)
#define AIF_SAI_IO_MASK		BIT(1) /* Only for SAI2 */
#define AIF_SAI_MME_MASK	BIT(3)
#define AIF_SAI_WL_MASK		(BIT(4) | BIT(5) | BIT(6))
#define AIF_SAI_DIR_MASK	BIT(7)
#define AIF_SAI_LRP_MASK	BIT(8)
#define AIF_SAI_CKP_MASK	BIT(9)
#define AIF_SAI_REL_MASK	BIT(10)
#define AIF_SAI_ADJ_MASK	BIT(11)
#define AIF_SAI_CNT_MASK	(BIT(12) | BIT(13))
#define AIF_SAI_SYN_MASK	(BIT(14) | BIT(15))
#define AIF_SAI_TM_MASK		BIT(23)

#define AIF_SAI_ENABLE			0x1
#define AIF_SAI2_DI		0x0
#define AIF_SAI2_DO		0x1
#define AIF_SAI_SLAVE		0x0
#define AIF_SAI_MASTER		0x1
#define AIF_SAI_WL_16_16	0x0
#define AIF_SAI_WL_24_16	0x2
#define AIF_SAI_WL_24_24	0x3
#define AIF_SAI_WL_32_16	0x4
#define AIF_SAI_WL_32_24	0x5
#define AIF_SAI_DIR_MSB		0x0
#define AIF_SAI_DIR_LSB		0x1
#define AIF_SAI_LRP_LEFT	0x0
#define AIF_SAI_LRP_RIGHT	0x1
#define AIF_SAI_CKP_NEGATIVE	0x0
#define AIF_SAI_CKP_POSITIVE	0x1
#define AIF_SAI_REL_FIRSTBIT	0x0
#define AIF_SAI_REL_I2S		0x1
#define AIF_SAI_ADJ_LEFT	0x0
#define AIF_SAI_ADJ_RIGHT	0x1
#define AIF_SAI_CNT_2W		0x0
#define AIF_SAI_CNT_4W		0x1
#define AIF_SAI_CNT_6W		0x2
#define AIF_SAI_CNT_8W		0x3
#define AIF_SAI_SIN_HALFFRAME	0x0
#define AIF_SAI_SIN_FIRSTWORD	0x1
#define AIF_SAI_SIN_FIRSTBIT	0x2
#define AIF_SAI_NORMAL		0x0
#define AIF_SAI_TESTMODE	0x1
#define AIF_SAI_DISABLE		0x00000000

#define AIF_SAI_ENA_SHIFT	0
#define AIF_SAI_IO_SHIFT	1
#define AIF_SAI_MME_SHIFT	3
#define AIF_SAI_WL_SHIFT	4
#define AIF_SAI_DIR_SHIFT	7
#define AIF_SAI_LRP_SHIFT	8
#define AIF_SAI_CKP_SHIFT	9
#define AIF_SAI_REL_SHIFT	10
#define AIF_SAI_ADJ_SHIFT	11
#define AIF_SAI_CNT_SHIFT	12
#define AIF_SAI_SYN_SHIFT	14
#define AIF_SAI_TM_SHIFT	23

/*LPF*/
enum {
	LPF_BYPASS,
	LPF_FIR0,
	LPF_UPX2,
	LPF_UPX4,
	LPF_DOWNX2,
	LPF_DOWNX3,
	LPF_DOWNX4,
	LPF_DOWNX6,
};

#define AIF_LPFCSR_CH0_MASK		(BIT(0) | BIT(1))
#define AIF_LPFCSR_CH1_MASK		(BIT(2) | BIT(3) | BIT(4))
#define AIF_LPFCSR_CH2_MASK		(BIT(5) | BIT(6) | BIT(15))
#define AIF_LPFCSR_CH3_MASK		(BIT(7) | BIT(8))
#define AIF_LPFCSR_CH4_MASK		(BIT(9) | BIT(10))
#define AIF_LPFCSR_CH5_MASK		(BIT(11) | BIT(12))

static const u32 lpf_channel_mask[] = { AIF_LPFCSR_CH0_MASK,
			AIF_LPFCSR_CH1_MASK, AIF_LPFCSR_CH2_MASK,
			AIF_LPFCSR_CH3_MASK, AIF_LPFCSR_CH4_MASK,
			AIF_LPFCSR_CH5_MASK};

#define AIF_LPFCSR_BYPASS			0x00000000
#define AIF_LPFCSR_FS0_FIR0			0x00000001
#define AIF_LPFCSR_FS0_UPX2			0x00000002
#define AIF_LPFCSR_FS0_UPX4			0x00000003
#define AIF_LPFCSR_FS1_FIR0			0x00000004
#define AIF_LPFCSR_FS1_DOWNX2		0x00000008
#define AIF_LPFCSR_FS1_DOWNX4		0x0000000c
#define AIF_LPFCSR_FS1_DOWNX6		0x00000010
#define AIF_LPFCSR_FS1_DOWNX3		0x00000014
#define AIF_LPFCSR_FS2_FIR0			0x00000020
#define AIF_LPFCSR_FS2_DOWNX2		0x00000040
#define AIF_LPFCSR_FS2_UPX2			0x00000060
#define AIF_LPFCSR_FS2_DOWNX3		0x00008000
#define AIF_LPFCSR_FS3_FIR0			0x00000080
#define AIF_LPFCSR_FS3_DOWNX2		0x00000100
#define AIF_LPFCSR_FS3_UPX2			0x00000180
#define AIF_LPFCSR_FS4_FIR0			0x00000200
#define AIF_LPFCSR_FS4_DOWNX2		0x00000400
#define AIF_LPFCSR_FS4_UPX2			0x00000600
#define AIF_LPFCSR_FS5_FIR0			0x00000800
#define AIF_LPFCSR_FS5_DOWNX2		0x00001000
#define AIF_LPFCSR_FS5_UPX2			0x00001800

#define AIF_LPF_ERROR				0xffffffff

/* SRC */
#define AIF_NUM_SRC		6
#define AIF_SRC0_CSR		0x8000
#define AIF_SRC1_CSR		0x8004
#define AIF_SRC2_CSR		0x8008
#define AIF_SRC3_CSR		0x800C
#define AIF_SRC4_CSR		0x8010
#define AIF_SRC5_CSR		0x8014
#define AIF_SRC_CSR(ch) (AIF_SRC0_CSR + ((ch) * 4))

#define AIF_SRC_NO_INPUT	0x00000000
#define AIF_SRC_DRLL_THRES	0x00000007
#define AIF_SRC_DITHER_EN	0x00000010
#define AIF_SRC_ROUNDING	0x00000020
#define AIF_SRC_BYPASS		0x00000040

/* AIF_AIMUX - AIF_AOMUX REGISTERS */
#define AIF_MUX_CH0_SHIFT		0
#define AIF_MUX_CH1_SHIFT		4
#define AIF_MUX_CH2_SHIFT		8
#define AIF_MUX_CH3_SHIFT		12
#define AIF_MUX_CH4_SHIFT		16
#define AIF_MUX_CH5_SHIFT		20
#define AIF_MUX_CH_SHIFT		4

#define AIF_MUX_CH0SEL_MASK	(BIT(0) | BIT(1) | BIT(2) | BIT(3))
#define AIF_MUX_CH1SEL_MASK	(BIT(4) | BIT(5) | BIT(6) | BIT(7))
#define AIF_MUX_CH2SEL_MASK	(BIT(8) | BIT(9) | BIT(10) | BIT(11))
#define AIF_MUX_CH3SEL_MASK	(BIT(12) | BIT(13) | BIT(14) | BIT(15))
#define AIF_MUX_CH4SEL_MASK	(BIT(16) | BIT(17) | BIT(18) | BIT(19))
#define AIF_MUX_CH5SEL_MASK	(BIT(20) | BIT(21) | BIT(22) | BIT(23))
#define AIF_AIMUX_LPF_DOWNX3X2_CASCADE_MASK			BIT(24)

/* AIF_AIMUX VALUES */
#define AIF_AIMUX_NO_INPUT	0x00000000
#define AIF_AIMUX_SAI1RX1	0x00000001
#define AIF_AIMUX_SAI2RX1	0x00000002
#define AIF_AIMUX_SAI2RX2	0x00000003
#define AIF_AIMUX_SAI2RX3	0x00000004
#define AIF_AIMUX_SAI2RX4	0x00000005
#define AIF_AIMUX_SAI3RX1	0x00000006
#define AIF_AIMUX_SAI3RX2	0x00000007
#define AIF_AIMUX_SAI3RX3	0x00000008
#define AIF_AIMUX_SAI3RX4	0x00000009
#define AIF_AIMUX_SAI4RX1	0x0000000A
#define AIF_AIMUX_SAI4RX2	0x0000000B
#define AIF_AIMUX_SAI4RX3	0x0000000C
#define AIF_AIMUX_SAI4RX4	0x0000000D
#define AIF_AIMUX_SPDIF		0x0000000E
#define AIF_AIMUX_LPFDI		0x0000000F
#define AIF_AIMUX_SAI2RX(ch) (AIF_AIMUX_SAI2RX1 + (ch))
#define AIF_AIMUX_SAI4RX(ch) (AIF_AIMUX_SAI4RX1 + (ch))

#define SAIRX(msp, ch) \
(((msp) == 1) ? AIF_AIMUX_SAI4RX(ch) : AIF_AIMUX_SAI2RX(ch))

#define SAITX(msp, ch) \
(((msp) == 1) ? AIF_AOMUX_SAI4TX(ch) : AIF_AOMUX_SAI2TX(ch))

/* AIF_AOMUX VALUES */
#define AIF_AOMUX_NO_OUTPUT	0x00000000
#define AIF_AOMUX_SAI2TX1	0x00000002
#define AIF_AOMUX_SAI3TX1	0x00000006
#define AIF_AOMUX_SAI3TX2	0x00000007
#define AIF_AOMUX_SAI3TX3	0x00000008
#define AIF_AOMUX_SAI3TX4	0x00000009
#define AIF_AOMUX_SAI4TX1	0x0000000A
#define AIF_AOMUX_SAI4TX2	0x0000000B
#define AIF_AOMUX_SAI4TX3	0x0000000C
#define AIF_AOMUX_SAI4TX4	0x0000000D
#define AIF_AOMUX_SRCDO		0x0000000E
#define AIF_AOMUX_SAI2TX(ch) (AIF_AOMUX_SAI2TX1 + (ch))
#define AIF_AOMUX_SAI4TX(ch) (AIF_AOMUX_SAI4TX1 + (ch))

/* ADCMIC */
#define ADCMIC_CHSEL1_MASK       BIT(0)
#define ADCMIC_RESETN1_MASK      BIT(1)
#define ADCMIC_BYPASSCALIB1_MASK BIT(2)
#define ADCMIC_STANDBY1_MASK     BIT(3)
#define ADCMIC_IOPSHIFT1_MASK    BIT(4)
#define ADCMIC_CHSEL2_MASK       BIT(8)
#define ADCMIC_RESETN2_MASK      BIT(9)
#define ADCMIC_BYPASSCALIB2_MASK BIT(10)
#define ADCMIC_STANDBY2_MASK     BIT(11)
#define ADCMIC_IOPSHIFT2_MASK    BIT(12)
#define ADCMIC_SDATA_SEL_MASK    (BIT(30) | BIT(31))

#define ADCMIC_STANDBY12 (ADCMIC_STANDBY1_MASK | ADCMIC_STANDBY2_MASK)
#define ADCMIC_STANDBY1  (ADCMIC_STANDBY1_MASK)
#define ADCMIC_STANDBY2  (ADCMIC_STANDBY2_MASK)
#define ADCMIC_ENABLE12  (ADCMIC_RESETN1_MASK | ADCMIC_RESETN2_MASK)
#define ADCMIC_ENABLE1   (ADCMIC_RESETN1_MASK)
#define ADCMIC_ENABLE2   (ADCMIC_RESETN2_MASK)
#define ADCMIC_SDATA1    0x00000000
#define ADCMIC_SDATA2    0x40000000
#define ADCMIC_SDATA12   0x80000000
#define ADCMIC_SDATA21   0xC0000000
#define ADCMIC1_AIN1     0x00000000
#define ADCMIC1_MICIN    ADCMIC_CHSEL1_MASK
#define ADCMIC2_AIN1     0x00000000
#define ADCMIC2_MICIN    ADCMIC_CHSEL2_MASK

/* DAC */
#define AUSS_DAC_MUTELEFT_MASK		(BIT(0) | BIT(1) | BIT(2))
#define AUSS_DAC_MUTERIGHT_MASK		(BIT(3) | BIT(4) | BIT(5))
#define AUSS_DAC_SB_MASK		(BIT(6) | BIT(7) | BIT(8))
#define AUSS_DAC_SBANA_MASK		(BIT(9) | BIT(10) | BIT(11))
#define AUSS_DAC_MUTEALL_MASK		(AUSS_DAC_MUTELEFT_MASK | \
					AUSS_DAC_MUTERIGHT_MASK)
#define AUSS_DAC_SBALL_MASK \
					(AUSS_DAC_SB_MASK | \
					AUSS_DAC_SBANA_MASK)

#define AUSS_DAC_ENABLE_ALL		0x00000000
#define AUSS_DAC_DISABLE_ALL		0x00000FFF
#define AUSS_DAC_MUTELEFT_DAC0		0x00000001
#define AUSS_DAC_MUTELEFT_DAC1		0x00000002
#define AUSS_DAC_MUTELEFT_DAC2		0x00000004
#define AUSS_DAC_MUTELEFT_DAC01		(AUSS_DAC_MUTELEFT_DAC0 | \
					AUSS_DAC_MUTELEFT_DAC1)
#define AUSS_DAC_MUTELEFT_DAC02		(AUSS_DAC_MUTELEFT_DAC0 | \
					AUSS_DAC_MUTELEFT_DAC2)
#define AUSS_DAC_MUTELEFT_DAC12		(AUSS_DAC_MUTELEFT_DAC1 | \
					AUSS_DAC_MUTELEFT_DAC2)
#define AUSS_DAC_MUTELEFT_DAC012	(AUSS_DAC_MUTELEFT_DAC0 | \
					AUSS_DAC_MUTELEFT_DAC1 | \
					AUSS_DAC_MUTELEFT_DAC2)
#define AUSS_DAC_MUTERIGHT_DAC0		0x00000008
#define AUSS_DAC_MUTERIGHT_DAC1		0x00000010
#define AUSS_DAC_MUTERIGHT_DAC2		0x00000020
#define AUSS_DAC_MUTERIGHT_DAC01	(AUSS_DAC_MUTERIGHT_DAC0 | \
					AUSS_DAC_MUTERIGHT_DAC1)
#define AUSS_DAC_MUTERIGHT_DAC02	(AUSS_DAC_MUTERIGHT_DAC0 | \
					AUSS_DAC_MUTERIGHT_DAC2)
#define AUSS_DAC_MUTERIGHT_DAC12	(AUSS_DAC_MUTERIGHT_DAC1 | \
					AUSS_DAC_MUTERIGHT_DAC2)
#define AUSS_DAC_MUTERIGHT_DAC012	(AUSS_DAC_MUTERIGHT_DAC0 | \
					AUSS_DAC_MUTERIGHT_DAC1 | \
					AUSS_DAC_MUTERIGHT_DAC2)
#define AUSS_DAC_MUTEALL_DAC		(AUSS_DAC_MUTELEFT_DAC012 | \
					AUSS_DAC_MUTERIGHT_DAC012)
#define AUSS_DAC_MUTESB_DAC0		0x00000040
#define AUSS_DAC_MUTESB_DAC1		0x00000080
#define AUSS_DAC_MUTESB_DAC2		0x00000100
#define AUSS_DAC_MUTESB_DAC01		(AUSS_DAC_MUTESB_DAC0 | \
					AUSS_DAC_MUTESB_DAC1)
#define AUSS_DAC_MUTESB_DAC02		(AUSS_DAC_MUTESB_DAC0 | \
					AUSS_DAC_MUTESB_DAC2)
#define AUSS_DAC_MUTESB_DAC12		(AUSS_DAC_MUTESB_DAC1 | \
					AUSS_DAC_MUTESB_DAC2)
#define AUSS_DAC_MUTESBANA_DAC0		0x00000200
#define AUSS_DAC_MUTESBANA_DAC1		0x00000400
#define AUSS_DAC_MUTESBANA_DAC2		0x00000800
#define AUSS_DAC_MUTESBANA_DAC01	(AUSS_DAC_MUTESBANA_DAC0 | \
					AUSS_DAC_MUTESBANA_DAC1)
#define AUSS_DAC_MUTESBANA_DAC02	(AUSS_DAC_MUTESBANA_DAC0 | \
					AUSS_DAC_MUTESBANA_DAC2)
#define AUSS_DAC_MUTESBANA_DAC12	(AUSS_DAC_MUTESBANA_DAC1 | \
					AUSS_DAC_MUTESBANA_DAC2)

/* MUX CR */
#define AUSS_MUX_MSP1EN_MASK		BIT(0)
#define AUSS_MUX_MSP2EN_MASK		BIT(1)
#define AUSS_MUX_MSP0REFCLKINEN_MASK	BIT(9)
#define AUSS_MUX_MSP1REFCLKINEN_MASK	(BIT(10) | BIT(5))
#define AUSS_MUX_MSP2REFCLKINEN_MASK	BIT(11)
#define AUSS_MUX_SAI1MEN_MASK		(BIT(6) | BIT(7) | BIT(8) | BIT(12))
#define AUSS_MUX_SAI4TOI2S0_MASK	BIT(13)
#define AUSS_MUX_SAI3DACONLY_MASK	BIT(14)

#define AUSS_MUX_MSP1REFCLKINEN_PLL3	(BIT(5))
#define AUSS_MUX_MSP1REFCLKINEN_EXT		(BIT(10) | BIT(5))
#define AUSS_MUX_MSP1REFCLKINEN_I2S0	0

#define AUSS_MUX_MSP1_SAI4_ENABLE	0x1
#define AUSS_MUX_MSP2_SAI2_ENABLE	0x1
#define AUSS_MUX_A2DP_ENABLE		0x1
#define AUSS_MUX_MSP0SCK_ENABLE		0x1
#define AUSS_MUX_MSP1SCK_ENABLE		0x1
#define AUSS_MUX_MSP2SCK_ENABLE		0x1
#define AUSS_MUX_SAI1MEN_SLAVE		0x1
#define AUSS_MUX_SAI4TOI2S0_ENABLE	0x1
#define AUSS_MUX_SAI3DACONLY_ENABLE	0x1
#define AUSS_MUX_SAI1SEL_SAI2		0x1
#define AUSS_MUX_SAI1SEL_SAI3		0x2
#define AUSS_MUX_SAI1SEL_SAI4		0x3
#define AUSS_MUX_SAI1SEL_MSP1		0x4
#define	AUSS_MUX_ENABLE			0x1
#define AUSS_MUX_DISABLE		0x0

#define AUSS_MUX_MSP1EN_SHIFT		0
#define AUSS_MUX_MSP2EN_SHIFT		1
#define AUSS_MUX_SAI1SEL_SHIFT		6
#define AUSS_MUX_MSP0REFCLKINEN_SHIFT	9
#define AUSS_MUX_MSP2REFCLKINEN_SHIFT	11
#define AUSS_MUX_SAI1MEN_SHIFT		12
#define AUSS_MUX_SAI4TOI2S0_SHIFT	13
#define AUSS_MUX_SAI3DACONLY_SHIFT	14

/*ADCAUX */
#define AIF_ADCAUX_STANDBY_MASK		BIT(12)
#define AIF_ADCAUX_CHSEL_MASK		BIT(16)
#define AIF_ADCAUX_BYPASSCALIB_MASK	BIT(20)
#define AIF_ADCAUX_RESTARTFIFO_MASK	BIT(24)

#define AIF_ADCAUX_ENABLE		0x00000000
#define AIF_ADCAUX_STANDBY		0x00001000
#define AIF_ADCAUX_CHSEL0		0x00000000
#define AIF_ADCAUX_CHSEL1		0x00010000
#define AIF_ADCAUX_BYPASSCALIB		0x00100000
#define AIF_ADCAUX_RESTARTFIFO		0x01000000

/* DMABUS_CR */
#define AIF_DMABUS_GATING		0x00000004
#define AIF_DMABUS_RUN			0x00000010
#define AIF_DMABUS_FREQ			0x00000060
#define AIF_DMABUS_FSYNC_CK1		0x00000180
#define AIF_DMABUS_FSYNC_CK2		0x00000600

/* DMABUS_TCM */
/* DMABUS SRC ADDRESSES */
#define STA_DMA_SRC0_DO			0x2010	/*ASRCs (stereo)*/
#define STA_DMA_SRC1_DO			0x2012
#define STA_DMA_SRC2_DO			0x2014
#define STA_DMA_SRC3_DO			0x2016
#define STA_DMA_SRC4_DO			0x2018
#define STA_DMA_SRC5_DO			0x201A
#define STA_DMA_SAI1_RX1		0x2310	/* SAIs (stereo) */
#define STA_DMA_SAI2_RX1		0x2314
#define STA_DMA_SAI2_RX2		0x2316
#define STA_DMA_SAI2_RX3		0x2318
#define STA_DMA_SAI2_RX4		0x231A
#define STA_DMA_SAI3_RX1		0x2320
#define STA_DMA_SAI3_RX2		0x2322
#define STA_DMA_SAI3_RX3		0x2324
#define STA_DMA_SAI3_RX4		0x2326
#define STA_DMA_SAI4_RX1		0x2330
#define STA_DMA_SAI4_RX2		0x2332
#define STA_DMA_SAI4_RX3		0x2334
#define STA_DMA_SAI4_RX4		0x2336
#define STA_DMA_FIFO_OUT		0x3080	/* FIFO (16 deep) */
#define STA_DMA_ADC_DATA		0x4001	/* ADC (stereo) */
#define STA_DMA_ADC_GPIN		0x4003	/* (mono) */
#define STA_DMA_DSP0_XOUT		0x5080	/* XOUT (128 ch) */
#define STA_DMA_DSP1_XOUT		0x6080
#define STA_DMA_DSP2_XOUT		0x7080
#define STA_DMA_SRC_DO(ch)		(STA_DMA_SRC0_DO + (2 * (ch)))
/* DMABUS DST ADDRESSES
 * note: LPFx_DI are r/w, addr can be SRC/DST
 */
#define STA_DMA_LPF0_DI			0x2110	/* LPFs (stereo) */
#define STA_DMA_LPF1_DI			0x2112
#define STA_DMA_LPF2_DI			0x2114
#define STA_DMA_LPF3_DI			0x2116
#define STA_DMA_LPF4_DI			0x2118
#define STA_DMA_LPF5_DI			0x211A
#define STA_DMA_SAI2_TX1		0x231C	/* SAIs (stereo) */
#define STA_DMA_SAI3_TX1		0x2328
#define STA_DMA_SAI3_TX2		0x232A
#define STA_DMA_SAI3_TX3		0x232C
#define STA_DMA_SAI3_TX4		0x232E
#define STA_DMA_SAI4_TX1		0x2338
#define STA_DMA_SAI4_TX2		0x233A
#define STA_DMA_SAI4_TX3		0x233C
#define STA_DMA_SAI4_TX4		0x233E
#define STA_DMA_FIFO_IN			0x3040	/* DP-MAILBOX FIFO1 */
#define STA_DMA_FIFO_OUT		0x3080	/* DP-MAILBOX FIFO2 */
#define STA_DMA_DCO_DATA		0x4003
#define STA_DMA_DSP0_XIN		0x5000	/*XIN (128 ch)*/
#define STA_DMA_DSP1_XIN		0x6000
#define STA_DMA_DSP2_XIN		0x7000
#define STA_DMA_LPF_DI(ch)		(STA_DMA_LPF0_DI + (2 * (ch)))

#define STA_DMA_DUMMY_MSP1		0x10000
#define STA_DMA_DUMMY_MSP2		0x20000
#define IS_DMA_DUMMY(addr) (!!((addr) & 0x30000))
#define STA_DMA_MSP(msp) \
	((msp) == 1 ? STA_DMA_DUMMY_MSP1 : STA_DMA_DUMMY_MSP2)

struct lpf_order {
	const unsigned int *channels;
	const unsigned int nchannels;
};

struct audioroutectl_protdesc {
	u32 msp1tx_sai4rx;
	u32 msp2tx_sai2rx;
	u32 sai1_men;
	u32 msp1_extrefclk0;
	u32 msp0_extrefclk;
	u32 msp1_extrefclk;
	u32 msp2_extrefclk;
	u32 sai4_i2s0;
	u32 sai3_dac_only;
};

struct sai_protdesc {
	u32 io_mode; /* only for SAI2 */
	u32 mme_mode;
	u32 word_length;
	u32 data_shift_dir;
	u32 lr_pol;
	u32 clk_pol;
	u32 rel_timing;
	u32 word_adj;
	u32 word_count;
	u32 frame_syn;
	u32 tm_mode;
};

struct dma_transfer_desc {
	u32 from;
	u32 from_shift;
	u32 to;
	u32 to_shift;
	u32 n_channels;
	u32 n_busy;
	struct list_head node;
};

struct challoc {
	u8 msp;
	u8 mspch;
	u8 dir;
	u8 lpf;
};

struct audio_source_desc {
	struct sai_protdesc *sai1_protdesc;
	struct sai_protdesc *sai2_protdesc;
	struct sai_protdesc *sai3_protdesc;
	struct sai_protdesc *sai4_protdesc;
	u32 chstream[AIF_NUM_SRC];
	u32 aimux;
	u32 aomux;
	u32 lpf;
	u32 src[AIF_NUM_SRC];
	u32 adcmic;
	u32 adcaux;
	struct audioroutectl_protdesc auss_mux_protdesc;
	struct list_head dma_transfer_list;
	const char *name;
	struct st_codec_aif_drvdata *aif_codec_drv;
};

struct st_codec_aif_drvdata {
	void __iomem *aif_registers;
	struct audio_source_desc *audio_source[MAX_NUM_AUDIOSOURCE];
	int source_index;
	struct device *dev;
	int dmabus_pos, dmabus_prev;
	struct mutex source_lock;
	struct challoc challoc[AIF_NUM_SRC];
	/* Clocks */
	struct clk *mclk;
	struct clk *fs512;

	struct regmap *aif_regmap;
	struct regmap *auss_regmap;
	struct timer_list source_timer;
#ifdef CONFIG_PM_SLEEP
	u32 auss_adcmic_cr, auss_dac_cr, auss_mux_cr, aif_dmabus_cr;
#endif
};

struct st_codec_aif_mixer_control {
	int min, max;
	int count;
};

static inline int st_codec_aif_filter_control_info(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_info *uinfo)
{
	struct st_codec_aif_mixer_control *mc =
		(struct st_codec_aif_mixer_control *)kcontrol->private_value;

	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = mc->count;
	uinfo->value.integer.min = mc->min;
	uinfo->value.integer.max = mc->max;

	return 0;
}

#define SOC_MULTI(xname, xhandler_get, xhandler_put, xcount, xmin, xmax) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
	.info = st_codec_aif_filter_control_info, \
	.get = xhandler_get, .put = xhandler_put, \
	.private_value = (unsigned long)&(struct st_codec_aif_mixer_control) \
	{.min = xmin, .max = xmax, .count = xcount} }

int st_codec_aif_channel_realloc(struct st_codec_aif_drvdata *aif_codec_drv);

#endif
