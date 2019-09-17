/*
 * sta10xx_audiomap.h
 *
 * Created on 2013/06/21 by Christophe Quarre
 *
 * ACCORDO2 STAudio Programmable Audio lib for EMERALD DSP
 *------------------------------------------------------------------
 * Copyright (c) 2013-2014, STMicroelectronics, All rights reserved.
 *
 * License terms:
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 */

#ifndef __STA10XX_AUDIO_MAP_H
#define __STA10XX_AUDIO_MAP_H

//only when compiling on real target !
#ifndef WIN32

#ifndef EXT
#define EXT extern
#endif

//#include "sta10xx_conf.h"
#include "sta10xx_type.h"

//-----------------------------------------------------------------------------
#ifndef REG_T
//lint -save -e960 -e9022 inhibits misra2004 19.10 for macros (REG_T...) (macro used to define hw registers, thus some param can't take parenthesis here or it won't compile)

/* Macro to define a register type */
#define REG_T(name)           typedef volatile union u_##name { u32 REG; struct s_##name {
#define REG_T_END(name)       } BIT; } name

/* Macro to define a register type AND declare it WITHIN a module's TypeDef */
#define REG_U(name)           volatile union u_##name { u32 REG; struct s_##name {
#define REG_U_END(name)       } BIT; } name

//lint -restore
#endif //REG_T
//-----------------------------------------------------------------------------


/******************************************************************************/
/*                          IP registers structures                           */
/******************************************************************************/


/*------------------------ AUDIO: Config Register (S23) ----------------------*/

typedef volatile struct AUD_CR_Typedef
{
    u8 unused1[0x0004-0x0000];

    //0x0004
    REG_U(ADCMIC_CR)
    u32  SEL                        : 1; //[0]		0:AIN1  1:MICIN
	u32  RESET                      : 1; //[1]		0:reset 1:normal op.  (new from CUT2)
    u32  __reserved1__              : 1; //[2]
    u32  BYPASS_CALIB               : 1; //[3]
    u32  STANDBY                    : 1; //[4]		0:EN    1:standby
	u32  CLKOUTPHASESHIFT           : 1; //[5]		for ADC testing only
    u32  __reserved2__              :26; //[31:6]
    REG_U_END(ADCMIC_CR);

    u8 unused2[0x0010-0x0008];

    //0x0010
    REG_U(DAC_CR)
    u32  MUTE_L                     : 3; //[2:0]
    u32  MUTE_R                     : 3; //[5:3]
    u32  SB                         : 3; //[8:6]
    u32  SBANA                      : 3; //[11:9]
    u32  __reserved1__              :20; //[31:12]
    REG_U_END(DAC_CR);

    u8 unused3[0x0100-0x0014];

    //0x0100
    REG_U(ACO_CR)
    u32  DSP_BigEndian              : 1; //[0]
    u32  AIF_AdrMode                : 1; //[1]
    u32  __reserved1__              : 1; //[2]
    u32  DSP_Start                  : 3; //[5:3]
    u32  DSP_EnableClk              : 3; //[8:6]
    u32  REFCLK_512Fs               : 1; //[9]
    u32  EnableRDFIFO               : 1; //[10]
    u32  EnableWRFIFO               : 1; //[11]
    u32  __reserved2__              :20; //[31:12]
    REG_U_END(ACO_CR);

    //0x0104
    REG_U(ACO_RIS)
    u32  DMA_BusErr                 : 1; //[0]
    u32  DMA_Runout                 : 1; //[1]
    u32  DSP_WaitInt                : 3; //[4:2]
    u32  DSP_Frozen                 : 3; //[7:5]
    u32  DSP_WaitSample             : 3; //[10:8]
    u32  ARM2DSP0_RIS               : 3; //[13:11]
    u32  ARM2DSP1_RIS               : 3; //[16:14]
    u32  ARM2DSP2_RIS               : 3; //[19:17]
    u32  FSYNC_RIS                  : 1; //[20]
    u32  __reserved1__              :11; //[31:21]
    REG_U_END(ACO_RIS);

    //0x0108
    REG_U(ACO_MIS)
    u32  DMA_BusErr                 : 1; //[0]
    u32  DMA_Runout                 : 1; //[1]
    u32  DSP_WaitInt                : 3; //[4:2]
    u32  DSP_Frozen                 : 3; //[7:5]
    u32  DSP_WaitSample             : 3; //[10:8]
    u32  ARM2DSP0_MIS               : 3; //[13:11]
    u32  ARM2DSP1_MIS               : 3; //[16:14]
    u32  ARM2DSP2_MIS               : 3; //[19:17]
    u32  FSYNC_MIS                  : 1; //[20]
    u32  __reserved1__              :11; //[31:21]
    REG_U_END(ACO_MIS);

    u8 unused4[0x0200-0x010C];

    //0x0200
    REG_U(ACO_IMSC)
    u32  DMA_BusErr                 : 1; //[0]
    u32  DMA_Runout                 : 1; //[1]
    u32  DSP_WaitInt                : 3; //[4:2]
    u32  DSP_Frozen                 : 3; //[7:5]
    u32  DSP_WaitSample             : 3; //[10:8]
    u32  ARM2DSP0_IMSC              : 1; //[11]
    u32  ARM2DSP1_IMSC              : 1; //[12]
    u32  ARM2DSP2_IMSC              : 1; //[13]
    u32  __reserved1__              : 6; //[19:14]
    u32  FSYNC_IMSC                 : 1; //[20]
    u32  __reserved2__              :11; //[31:21]
    REG_U_END(ACO_IMSC);

    u8 unused5[0x0208-0x0204];

    //0x0208
    REG_U(ACO_ICR)
    u32  DMA_BusErr                 : 1; //[0]
    u32  DMA_Runout                 : 1; //[1]
    u32  DSP_WaitInt                : 3; //[4:2]
    u32  DSP_Frozen                 : 3; //[7:5]
    u32  DSP_WaitSample             : 3; //[10:8]
    u32  ARM2DSP0_ICR               : 1; //[11]
    u32  ARM2DSP1_ICR               : 1; //[12]
    u32  ARM2DSP2_ICR               : 1; //[13]
    u32  __reserved1__              : 6; //[19:14]
    u32  FSYNC_ICR                  : 1; //[20]
    u32  __reserved2__              :11; //[31:21]
    REG_U_END(ACO_ICR);

    u8 unused6[0x0210-0x020C];

    //0x0210
    REG_U(ACO_EIR0)
    u32  __reserved1__              : 1; //[0]
    u32  ARM2DSP0_INT1              : 1; //[1]
    u32  ARM2DSP0_INT2              : 1; //[2]
    u32  ARM2DSP0_INT3              : 1; //[3]
    u32  FSYNCDSP0_INT4             : 1; //[4]
    u32  FSCK1DSP0_INT5             : 1; //[5]
    u32  FSCK2DSP0_INT6             : 1; //[6]
    u32  ARM2DSP0_INT7              : 1; //[7]
    u32  __reserved2__              :24; //[8:31]
    REG_U_END(ACO_EIR0);

    //0x0214
    REG_U(ACO_EIR1)
    u32  __reserved1__              : 1; //[0]
    u32  ARM2DSP1_INT1              : 1; //[1]
    u32  ARM2DSP1_INT2              : 1; //[2]
    u32  ARM2DSP1_INT3              : 1; //[3]
    u32  FSYNCDSP1_INT4             : 1; //[4]
    u32  FSCK1DSP1_INT5             : 1; //[5]
    u32  FSCK2DSP1_INT6             : 1; //[6]
    u32  ARM2DSP1_INT7              : 1; //[7]
    u32  __reserved2__              :24; //[8:31]
    REG_U_END(ACO_EIR1);

    //0x0218
    REG_U(ACO_EIR2)
    u32  __reserved1__              : 1; //[0]
    u32  ARM2DSP2_INT1              : 1; //[1]
    u32  ARM2DSP2_INT2              : 1; //[2]
    u32  ARM2DSP2_INT3              : 1; //[3]
    u32  FSYNCDSP2_INT4             : 1; //[4]
    u32  FSCK1DSP2_INT5             : 1; //[5]
    u32  FSCK2DSP2_INT6             : 1; //[6]
    u32  ARM2DSP2_INT7              : 1; //[7]
    u32  __reserved2__              :24; //[8:31]
    REG_U_END(ACO_EIR2);

    u8 unused10[0x0400-0x021C];

    //0x0400
    REG_U(DCO_CR0)
    u32  MUX0SEL                    : 6; //[5:0]
    u32  MUX1SEL                    : 6; //[11:6]
    u32  MUX2SEL                    : 6; //[17:12]
    u32  __reserved1__              :14; //[31:18]
    REG_U_END(DCO_CR0);

    //0x0404
    REG_U(DCO_CR1)
    u32  MUX3SEL                    : 6; //[5:0]
    u32  MUX4SEL                    : 6; //[11:6]
    u32  MUX5SEL                    : 6; //[17:12]
    u32  __reserved1__              :14; //[31:18]
    REG_U_END(DCO_CR1);

    u8 unused11[0x0800-0x0408];

    //0x0800
    REG_U(ACO_DBG)
    u32  WSMP                       : 3; //[2:0]
    u32  FROZEN                     : 3; //[5:3]
    u32  WINT                       : 3; //[8:6]
    u32  __reserved1__              :23; //[31:9]
    REG_U_END(ACO_DBG);

    u8 unused12[0x1000-0x0804];

    //0x1000
    REG_U(MUX_CR)
    u32  MSP1_EN                    : 1; //[0]
    u32  MSP2_EN                    : 1; //[1]
    u32  SAI3_DAC_EN                : 1; //[2]
    u32  SAI3_ADC_EN                : 1; //[3]
    u32  SAI3_EXT_CODEC             : 1; //[4]
    u32  SAI4_A2DP_EN               : 1; //[5]
    u32  __reserved1__              : 3; //[8:6]
    u32  AUD_REFCLK_MSP0            : 1; //[9]
    u32  AUD_REFCLK_MSP1            : 1; //[10]
    u32  AUD_REFCLK_MSP2            : 1; //[11]
    u32  SAI1_MEN                   : 1; //[12]
    u32  SAI4TOI2S0                 : 1; //[13]
    u32  SAI3DACOnly                : 1; //[14]
    u32  __reserved2__              :17; //[31:15]
    REG_U_END(MUX_CR);

    //0x1004
    //OTP_CR                        //Not used

} AUD_CR_Typedef;

/*------------------------ AUDIO: DSPs (P0,P1,P2) ---------------------------*/

typedef volatile struct AUD_DSP0_Typedef
{
    u32  PRAM[6*1024];              //0x00000*4
    u32  unused1[0x10000-0x1800];
    u32  DBG_CMD[1];                //0x10000*4   TODO(low): clarify the size ??
    u32  DBG_STS[1];                //                       clarify the size ??
    u32  unused2[0x20000-0x10002];
    u32  XRAM[4*1024];              //0x20000*4
    u32  XIN[128];                  //0x21000*4
    u32  XOUT[128];                 //0x21080*4
    u32  unused3[0x22000-0x21100];
    u32  IRQ_D2A_SE;                //0x22000*4
    u32  IRQ_D2A_CL;
    u32  IRQ_A2D_EN;
    u32  IRQ_A2D_ST;
    u32  unused4[0x30000-0x22004];
	u32  YRAM[4*1024];
} AUD_DSP0_Typedef;


typedef volatile struct AUD_DSP2_Typedef
{
    u32  PRAM[8*1024];              //0x00000*4
    u32  unused1[0x10000-0x2000];
    u32  DBG_CMD[1];                //0x10000*4   TODO(low): clarify the size ??
    u32  DBG_STS[1];                //                       clarify the size ??
    u32  unused2[0x20000-0x10002];
    u32  XRAM[12*1024];              //0x20000*4
    u32  XIN[128];                  //0x21000*4
    u32  XOUT[128];                 //0x21080*4
    u32  unused3[0x24000-0x23100];
    u32  IRQ_D2A_SE;                //0x22000*4
    u32  IRQ_D2A_CL;
    u32  IRQ_A2D_EN;
    u32  IRQ_A2D_ST;
    u32  unused4[0x30000-0x24004];
	u32  YRAM[12*1024];
} AUD_DSP2_Typedef;

/*------------------------ AUDIO: ADCAUX ---------------------------*/
typedef volatile struct ADCAUX_Typedef
{
	REG_U(ADCAUX_CR)
    u32  _res1                      :12; //[0:11]
    u32  STANDBY                    : 1; //[12]
    u32  _res2                      : 3; //[13:15]
    u32  CHSEL                      : 1; //[16]
    u32  _res3                      : 3; //[17:19]
    u32  BypassCalib                : 1; //[20]
    u32  _res4                      : 3; //[21:23]
    u32  ResetFIFO                  : 1; //[24]
    REG_U_END(ADCAUX_CR);

    u32 DATA_L;
    u32 DATA_R;

} ADCAUX_Typedef;

/*------------------------ AUDIO: DMABUS (P4) ----------------------------*/
//DMABus
typedef volatile struct AUD_DMA_Typedef
{
	/*
    REG_U(DMA_CR)
    u32  _res1                      : 2; //[1:0]
    u32  GATING_EN                  : 1; //[2]
    u32  _res2                      : 1; //[3]
    u32  DMA_RUN                    : 1; //[4]
    u32  DMAFREQ_SEL                : 2; //[6:5]
    u32  FSYNC_CK1_SEL              : 2; //[8:7]	//CUT2   or [8:9]?
    u32  FSYNC_CK2_SEL              : 2; //[10:9]	//CUT2   or [10:11]?
    u32  _res3                      :21; //[31:7]
    REG_U_END(DMA_CR);
	*/
    REG_U(DMA_CR)
    u32  _res1                      : 2; //[1:0]
    u32  GATING_EN                  : 1; //[2]
    u32  _res2                      : 1; //[3]
    u32  DMA_RUN                    : 1; //[4]
    u32  DMAFREQ_SEL                : 2; //[6:5]
    u32  _res3                      : 1; //[7]
    u32  FSYNC_CK1_SEL              : 2; //[9:8]	//CUT2
    u32  FSYNC_CK2_SEL              : 2; //[11:10]	//CUT2
    u32  _res4                      :20; //[31:7]
    REG_U_END(DMA_CR);

    u32 unused1[0x800-1];

    REG_U(DMA_TX)
    u32  SRC                        :16; //[15:0]
    u32  DST                        :16; //[31:16]
    REG_U_END(DMA_TX[512]);
} AUD_DMA_Typedef;

/*------------------------ AUDIO: AIF (P4) ----------------------------*/

//SRCs
//AUDIO_P4_BASE + 0x8000
REG_T(AUD_SRC_CR)
    u32  DRLL_THRES                 : 4; //[3:0]
    u32  DITHER_EN                  : 1; //[4]
    u32  ROUND_EN                   : 1; //[5]
    u32  BYPASS                     : 1; //[6]
    u32  _res1                      : 5; //[11:7]
    u32  DRLL_DIFF                  :14; //[25:12]
    u32  _res2                      : 2; //[27:26]
    u32  DRLL_LOCK                  : 2; //[29:28]
    u32  _res3                      : 2; //[31:30]
REG_T_END(AUD_SRC_CR);

REG_T(AUD_SRC_DO)
    u32  _res1                      : 2; //[1:0]
    u32  OUT_DATA                   :22; //[23:2]
    u32  _res2                      : 8; //[31:24]
REG_T_END(AUD_SRC_DO);

REG_T(AUD_SRC_RR)
    u32  RATIO                      :22; //[21:0]
    u32  FC                         : 1; //[22]
    u32  _res1                      : 9; //[31:23]
REG_T_END(AUD_SRC_RR);

//LPF
REG_T(AUD_LPF_CR)
    u32  FS_0                       : 2; //[1:0]
    u32  FS_1                       : 3; //[4:2]
    u32  FS_2                       : 2; //[6:5]
    u32  FS_3                       : 2; //[8:7]
    u32  FS_4                       : 2; //[10:9]
    u32  FS_5                       : 2; //[12:11]
    u32  TST_OFD                    : 1; //[13]
    u32  CLRF                       : 1; //[14]
    u32  FS_2_BIT2                  : 1; //[15]
    u32  LPF_UFL                    : 6; //[21:16]
    u32  _res2                      : 2; //[23:22]
    u32  LPF_OVF                    : 6; //[29:24]
    u32  _res3                      : 2; //[31:30]
REG_T_END(AUD_LPF_CR);

REG_T(AUD_LPF_DT)
    u32  _res1                      : 4; //[3:0]
    u32  DATA                       :20; //[23:4]
    u32  _res2                      : 8; //[31:24]
REG_T_END(AUD_LPF_DT);

//AUDIO I/O MUX
REG_T(AUD_AIOMUX)
    u32  CH0SEL                     : 4; //[3:0]
    u32  CH1SEL                     : 4; //[7:4]
    u32  CH2SEL                     : 4; //[11:8]
    u32  CH3SEL                     : 4; //[15:12]
    u32  CH4SEL                     : 4; //[19:16]
    u32  CH5SEL                     : 4; //[23:20]
	u32  CH2CH3_DOWNx3_x2_CASCADE	: 1; //[24]		//for AIMUX only
    u32  _res1                      : 7; //[31:25]
REG_T_END(AUD_AIOMUX);

//LPFSRC FSYNC I/O
REG_T(AUD_FSYNCSEL)
    u32  CH0SEL                     : 2; //[1:0]
    u32  CH1SEL                     : 2; //[3:2]
    u32  CH2SEL                     : 2; //[5:4]
    u32  CH3SEL                     : 2; //[7:6]
    u32  CH4SEL                     : 2; //[9:8]
    u32  CH5SEL                     : 2; //[11:10]
    u32  _res1                      :20; //[31:12]
REG_T_END(AUD_FSYNCSEL);

//SAIs
REG_T(AUD_SAI_CR)
    u32  EN                         : 1; //[0]
    u32  IO                         : 1; //[1]
    u32  _res1                      : 1; //[2]
    u32  MME                        : 1; //[3]
    u32  WL                         : 3; //[6:4]
    u32  DIR                        : 1; //[7]
    u32  LRP                        : 1; //[8]
    u32  CKP                        : 1; //[8]
    u32  REL                        : 1; //[10]
    u32  ADJ                        : 1; //[11]
    u32  CNT                        : 2; //[13:12]
    u32  SYN                        : 2; //[15:14]
    u32  _res2                      : 7; //[22:16]
    u32  TM                         : 1; //[23]
    u32  _res3                      : 8; //[31:24]
REG_T_END(AUD_SAI_CR);

REG_T(AUD_SPDIF_CR)
    u32  EN                         : 1; //[0]
    u32  SSEL                       : 2; //[2:1]
    u32  CHS                        : 1; //[3]
    u32  EMV                        : 1; //[4]
    u32  EM                         : 3; //[7:5]
    u32  _res1                      : 4; //[11:8]
    u32  PLR                        : 1; //[12]
    u32  PL                         : 1; //[13]
    u32  PR                         : 1; //[14]
    u32  VLR                        : 1; //[15]
    u32  VL                         : 1; //[16]
    u32  VR                         : 1; //[17]
    u32  ERFLR                      : 1; //[18]
    u32  ERFL                       : 1; //[19]
    u32  ERFR                       : 1; //[20]
    u32  ERCLR                      : 1; //[21]
    u32  _res2                      :10; //[31:22]
REG_T_END(AUD_SPDIF_CR);

typedef volatile struct AUD_AIF_Typedef
{
    //SRCs
    AUD_SRC_CR   SRC_CR[6];          //0x2000*4
    u32 unused1[0x2010-0x2006];
    AUD_SRC_DO   SRC0_DO[2];         //0x2010*4   note: even addr = left data, odd addr = right data
    AUD_SRC_DO   SRC1_DO[2];         //0x2012*4   note: even addr = left data, odd addr = right data
    AUD_SRC_DO   SRC2_DO[2];         //0x2014*4   note: even addr = left data, odd addr = right data
    AUD_SRC_DO   SRC3_DO[2];         //0x2016*4   note: even addr = left data, odd addr = right data
    AUD_SRC_DO   SRC4_DO[2];         //0x2018*4   note: even addr = left data, odd addr = right data
    AUD_SRC_DO   SRC5_DO[2];         //0x201A*4   note: even addr = left data, odd addr = right data
    u32 unused2[0x2030-0x201C];
    AUD_SRC_RR   SRC_RR[6];          //0x2030*4
    u32 unused3[0x2100-0x2036];

    //LPFs
    AUD_LPF_CR   LPF_CR;             //0x2100*4
    u32 unused4[0x2110-0x2101];
    AUD_LPF_DT   LPF0_DI[2];         //0x2110*4   note: even addr = left data, odd addr = right data
    AUD_LPF_DT   LPF1_DI[2];         //0x2112*4   note: even addr = left data, odd addr = right data
    AUD_LPF_DT   LPF2_DI[2];         //0x2114*4   note: even addr = left data, odd addr = right data
    AUD_LPF_DT   LPF3_DI[2];         //0x2116*4   note: even addr = left data, odd addr = right data
    AUD_LPF_DT   LPF4_DI[2];         //0x2118*4   note: even addr = left data, odd addr = right data
    AUD_LPF_DT   LPF5_DI[2];         //0x211A*4   note: even addr = left data, odd addr = right data
    u32 unused5[0x2120-0x211C];
    AUD_LPF_DT   LPF0_DO[2];         //0x2120*4   note: tmp regs between LPF and SRC, (for test purpose only because not at 48kHz)
    AUD_LPF_DT   LPF1_DO[2];         //0x2122*4   note: tmp regs between LPF and SRC, (for test purpose only because not at 48kHz)
    AUD_LPF_DT   LPF2_DO[2];         //0x2124*4   note: tmp regs between LPF and SRC, (for test purpose only because not at 48kHz)
    AUD_LPF_DT   LPF3_DO[2];         //0x2126*4   note: tmp regs between LPF and SRC, (for test purpose only because not at 48kHz)
    AUD_LPF_DT   LPF4_DO[2];         //0x2128*4   note: tmp regs between LPF and SRC, (for test purpose only because not at 48kHz)
    AUD_LPF_DT   LPF5_DO[2];         //0x212A*4   note: tmp regs between LPF and SRC, (for test purpose only because not at 48kHz)

    //LPF Overflow/Underflow
	u32 LPFOVF_COMP;                 //0x212C*4
	u32 LPFUNF_COMP;                 //0x212D*4

    u32 unused6[0x2200-0x212E];

    //AUDIO I/O MUX
    AUD_AIOMUX   AIMUX;              //0x2200*4
    AUD_AIOMUX   AOMUX;              //0x2201*4

	//LPFSRC FSYNC I/O
	AUD_FSYNCSEL FSYNCIN;            //0x2202*4
	AUD_FSYNCSEL FSYNCOUT;           //0x2203*4

    u32 unused7[0x2300-0x2204];

    //SAIs CTRL
//  AUD_SAI_CR   SAI_CR[4];          //0x2300*4
    AUD_SAI_CR   SAI1_CR;            //0x2300*4
    AUD_SAI_CR   SAI2_CR;            //0x2301*4
    AUD_SAI_CR   SAI3_CR;            //0x2302*4
    AUD_SAI_CR   SAI4_CR;            //0x2303*4

    //SPDIF
    AUD_SPDIF_CR SPDIF_CR;           //0x2304*4
    u32  unused8[11];

    //SAIs RX/TX
    u32  SAI1_RX[2];                 //0x2310*4
    u32  unused9[2];
    u32  SAI2_RX1[2];
    u32  SAI2_RX2[2];
    u32  SAI2_RX3[2];
    u32  SAI2_RX4[2];
    u32  SAI2_TX1[2];
    u32  unused10[2];
    u32  SAI3_RX1[2];
    u32  SAI3_RX2[2];
    u32  SAI3_RX3[2];
    u32  SAI3_RX4[2];
    u32  SAI3_TX1[2];
    u32  SAI3_TX2[2];
    u32  SAI3_TX3[2];
    u32  SAI3_TX4[2];
    u32  SAI4_RX1[2];
    u32  SAI4_RX2[2];
    u32  SAI4_RX3[2];
    u32  SAI4_RX4[2];
    u32  SAI4_TX1[2];
    u32  SAI4_TX2[2];
    u32  SAI4_TX3[2];
    u32  SAI4_TX4[2];
} AUD_AIF_Typedef;

/*------------------------ AUDIO: AHB2DMA Bridge (P4) -----------------------*/
/*
typedef volatile struct AUD_P4_Typedef
{
    u32 unused1[0x1000-0x0000];
    AUD_DMA_CR  DMA_CR;
    u32 unused2[0x1800-0x1001];
    AUD_DMA_TX  DMA_TX[0x0200];

} AUD_P4_Typedef;
*/

/******************************************************************************/
/*                       Peripheral memory map                                */
/******************************************************************************/

/* Audio Sub-system --------------------------------------------------------*/
#ifdef LINUX_OS
extern void *AUDIO_CR_BASE;
extern void *AUDIO_P4_BASE;
#else
#define AUDIO_CR_BASE               ((u32)0x48060000)
#define AUDIO_P0_BASE               ((u32)0x48900000)
#define AUDIO_P1_BASE               ((u32)0x48A00000)
#define AUDIO_P2_BASE               ((u32)0x48B00000)
#define AUDIO_P3_BASE               ((u32)0x48C00000)
#define AUDIO_P4_BASE               ((u32)0x48D00000)

#define DSP0_BASE                   ((u32)0x48900000)
#define DSP1_BASE                   ((u32)0x48A00000)
#define DSP2_BASE                   ((u32)0x48B00000)
#endif

//P4 sub-bases
#define AUDIO_DMA_BASE              (AUDIO_P4_BASE + 0x04000)
#define AUDIO_DMA_TX_BASE           (AUDIO_P4_BASE + 0x06000)
#define AIF_BASE                    (AUDIO_P4_BASE + 0x08000)
#define ADCAUX_BASE					(AUDIO_P4_BASE + 0x10000)
#define DMABUS_XIN0_BASE            (AUDIO_P4_BASE + 0x14000)
#define DMABUS_XOUT0_BASE           (AUDIO_P4_BASE + 0x14200)
#define DMABUS_XIN1_BASE            (AUDIO_P4_BASE + 0x18000)
#define DMABUS_XOUT1_BASE           (AUDIO_P4_BASE + 0x18200)
#define DMABUS_XIN2_BASE            (AUDIO_P4_BASE + 0x1C000)
#define DMABUS_XOUT2_BASE           (AUDIO_P4_BASE + 0x1C200)

//-----------------------------------------------------------------
//from sta10xx_dsp.h
/*
#ifndef DSP0_PRAM_ADDR

//DSP0 mapping
#define DSP0_PRAM_ADDR				( (vu32*)(DSP0_BASE + 0x00000))
#define DSP0_XRAM_ADDR				( (vu32*)(DSP0_BASE + 0x80000))
#define DSP0_XIN_ADDR				( (vu32*)(DSP0_BASE + 0x84000))
#define DSP0_XOUT_ADDR				( (vu32*)(DSP0_BASE + 0x84200))
#define DSP0_DSP2ARM_IRQ_SET		(*(vu32*)(DSP0_BASE + 0x88000))
#define DSP0_DSP2ARM_IRQ_CLR		(*(vu32*)(DSP0_BASE + 0x88004))
#define DSP0_ARM2DSP_IRQ_EN			(*(vu32*)(DSP0_BASE + 0x88008))
#define DSP0_ARM2DSP_IRQ_STS		(*(vu32*)(DSP0_BASE + 0x8800C))
#define DSP0_YRAM_ADDR				( (vu32*)(DSP0_BASE + 0xC0000))

//DSP1 mapping
#define DSP1_PRAM_ADDR				( (vu32*)(DSP1_BASE + 0x00000))
#define DSP1_XRAM_ADDR				( (vu32*)(DSP1_BASE + 0x80000))
#define DSP1_XIN_ADDR				( (vu32*)(DSP1_BASE + 0x84000))
#define DSP1_XOUT_ADDR				( (vu32*)(DSP1_BASE + 0x84200))
#define DSP1_DSP2ARM_IRQ_SET		(*(vu32*)(DSP1_BASE + 0x88000))
#define DSP1_DSP2ARM_IRQ_CLR		(*(vu32*)(DSP1_BASE + 0x88004))
#define DSP1_ARM2DSP_IRQ_EN			(*(vu32*)(DSP1_BASE + 0x88008))
#define DSP1_ARM2DSP_IRQ_STS		(*(vu32*)(DSP1_BASE + 0x8800C))
#define DSP1_YRAM_ADDR				( (vu32*)(DSP1_BASE + 0xC0000))

//DSP2 mapping
#define DSP2_PRAM_ADDR				( (vu32*)(DSP2_BASE + 0x00000))
#define DSP2_XRAM_ADDR				( (vu32*)(DSP2_BASE + 0x80000))
#define DSP2_XIN_ADDR				( (vu32*)(DSP2_BASE + 0x8c000))
#define DSP2_XOUT_ADDR				( (vu32*)(DSP2_BASE + 0x8c200))
#define DSP2_DSP2ARM_IRQ_SET		(*(vu32*)(DSP2_BASE + 0x90000))
#define DSP2_DSP2ARM_IRQ_CLR		(*(vu32*)(DSP2_BASE + 0x90004))
#define DSP2_ARM2DSP_IRQ_EN			(*(vu32*)(DSP2_BASE + 0x90008))
#define DSP2_ARM2DSP_IRQ_STS		(*(vu32*)(DSP2_BASE + 0x9000C))
#define DSP2_YRAM_ADDR				( (vu32*)(DSP2_BASE + 0xC0000))

//DSP mems sizes
#define DSP0_PRAM_SIZE				( 6*1024)
#define DSP0_XRAM_SIZE				( 4*1024)
#define DSP0_YRAM_SIZE				( 4*1024)
#define DSP1_PRAM_SIZE				( 6*1024)
#define DSP1_XRAM_SIZE				( 4*1024)
#define DSP1_YRAM_SIZE				( 4*1024)
#define DSP2_PRAM_SIZE				( 8*1024)
#define DSP2_XRAM_SIZE				(12*1024)
#define DSP2_YRAM_SIZE				(12*1024)

#endif //incl from sta10xx_dsp.h
*/

/******************************************************************************/
/*                            IPs' declaration                                */
/******************************************************************************/

/*------------------- Non Debug Mode -----------------------------------------*/
#ifndef DEBUG

//#ifdef _AUDIO_SUBSYSTEM
  #define AUDCR              ((AUD_CR_Typedef*)  AUDIO_CR_BASE)
  #define AUDDMA             ((AUD_DMA_Typedef*) AUDIO_DMA_BASE)
  #define AIF                ((AUD_AIF_Typedef*) AIF_BASE)
  #define DSP0               ((AUD_DSP0_Typedef*) DSP0_BASE)
  #define DSP1               ((AUD_DSP0_Typedef*) DSP1_BASE)
  #define DSP2               ((AUD_DSP2_Typedef*) DSP2_BASE)
  #define ADCAUX             ((ADCAUX_Typedef*)  ADCAUX_BASE)
//#endif

/*----------------------  Debug Mode -----------------------------------------*/
#else   /* DEBUG */


//#ifdef _AUDIO_SUBSYSTEM
  EXT AUD_CR_Typedef          *AUDCR;
  EXT AUD_DMA_Typedef         *AUDDMA;
  EXT AUD_AIF_Typedef         *AIF;
  EXT AUD_DSP0_Typedef        *DSP0;
  EXT AUD_DSP0_Typedef        *DSP1;
  EXT AUD_DSP2_Typedef        *DSP2;
  EXT ADCAUX_Typedef          *ADCAUX;
//#endif

#endif  /* DEBUG */


#endif // not WIN32
#endif // __STA10XX_AUDIO_MAP_H

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
