/***********************************************************************************/
/*!
*  \file      staudio.h
*
*  \brief     <b> STAudioLib Header </b>
*
*  \details   STAudioLib is the API to program the audio effects on the EMERALD DSPs on Accordo2.
*             This is the main header file to be included by in the Audio Application
*
*  \author    Quarre Christophe, TVa, ODo
*
*  \author    (original version, created on 2013/04/22) Quarre Christophe
*
*  \version   M17 v0.4
*
*  \date      2017 May 24
*
*  \bug       see Readme.txt
*
*  \warning
*
*  This file is part of STAudioLib and is dual licensed,
*  ST Proprietary License or GNU General Public License version 2.
*
********************************************************************************
*
* Copyright (c) 2017 STMicroelectronics - All Rights Reserved
* Reproduction and Communication of this document is strictly prohibited
* unless specifically authorized in writing by STMicroelectronics.
* FOR MORE INFORMATION PLEASE READ CAREFULLY THE LICENSE AGREEMENT LOCATED
* IN THE ROOT DIRECTORY OF THIS SOFTWARE PACKAGE.
*
********************************************************************************
*
* ALTERNATIVELY, this software may be distributed under the terms of the
* GNU General Public License ("GPL") version 2, in which case the following
* provisions apply instead of the ones mentioned above :
*
********************************************************************************
*
* STAudioLib is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* STAudioLib is distributed in the hope that it will be
* useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* Please refer to <http://www.gnu.org/licenses/>.
*
*/
/***********************************************************************************/

#ifndef STAUDIO_H_
#define STAUDIO_H_

#ifdef __cplusplus
extern "C" {
#endif

//sta_platforms.h: defines the target OS and basic types.
#include "sta_platforms.h"


//=== Defines ================================================================

//WARNING: STA_DSP_VERSION must match the DSP audio firmware version (@0x30 in.Y file with IMG header)
#define STA_LIB_VERSION			57
#define STA_DSP_VERSION			39
#define STA_DCO_VERSION			3

//SW Capabilities
#define STA_MAX_MODULE_NAME_SIZE	24  /* included the final /0 */

//DSP Capabilities
//#define STA_MAX_GAIN_CHANNELS	10
#define STA_MAX_EQ_NUM_BANDS	16
#define STA_MAX_MIXER_INS		9	//note: and only 2 insets per multi-channels mixer
#define STA_MAX_MIXER_CH		6
#define STA_MAX_DELAY_CH		6
#define STA_MAX_MUX_INS			18
#define STA_MAX_MUX_OUT			10
#define STA_MAX_UPDATE_SLOTS	20
#define STA_MAX_DCO_DELAY		47

#define STA_XIN_SIZE			128
#define STA_XOUT_SIZE			128
#define STA_DMA_SIZE			512


//DMABUS SRC ADDRESSES
#define STA_DMA_SRC0_DO			0x2010		/*ASRCs (stereo)*/
#define STA_DMA_SRC1_DO			0x2012
#define STA_DMA_SRC2_DO			0x2014
#define STA_DMA_SRC3_DO			0x2016
#define STA_DMA_SRC4_DO			0x2018
#define STA_DMA_SRC5_DO			0x201A
#define STA_DMA_SAI1_RX1		0x2310		/*SAIs (stereo)*/
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
#define STA_DMA_FIFO_OUT		0x3080		/*FIFO (16 deep)*/
#define STA_DMA_ADC_DATA		0x4001		/*ADC (stereo) */
#define STA_DMA_ADC_GPIN		0x4003		/*    (mono)   */
#define STA_DMA_DSP0_XOUT		0x5080		/*XOUT (128 ch)*/
#define STA_DMA_DSP1_XOUT		0x6080
#define STA_DMA_DSP2_XOUT		0x7080
#define STA_DMA_DCO_DATA		0x4003

//DMABUS DST ADDRESSES
#define STA_DMA_LPF0_DI			0x2110		/*LPFs (stereo)*/
#define STA_DMA_LPF1_DI			0x2112		/*note: LPFx_DI are r/w, and can thus also be SRC*/
#define STA_DMA_LPF2_DI			0x2114
#define STA_DMA_LPF3_DI			0x2116
#define STA_DMA_LPF4_DI			0x2118
#define STA_DMA_LPF5_DI			0x211A
#define STA_DMA_SAI2_TX1		0x231C		/*SAIs (stereo)*/
#define STA_DMA_SAI3_TX1		0x2328
#define STA_DMA_SAI3_TX2		0x232A
#define STA_DMA_SAI3_TX3		0x232C
#define STA_DMA_SAI3_TX4		0x232E
#define STA_DMA_SAI4_TX1		0x2338
#define STA_DMA_SAI4_TX2		0x233A
#define STA_DMA_SAI4_TX3		0x233C
#define STA_DMA_SAI4_TX4		0x233E
#define STA_DMA_FIFO_IN			0x3040		/*FIFO (16 deep)*/
#define STA_DMA_DSP0_XIN		0x5000		/*XIN (128 ch)*/
#define STA_DMA_DSP1_XIN		0x6000
#define STA_DMA_DSP2_XIN		0x7000

//DMABUS tranfer types
#define STA_DMA_FSCK1_SLOT0		0x00000000
#define STA_DMA_FSCK1_SLOT1		0x10000000
#define STA_DMA_FSCK1_SLOT2		0x20000000
#define STA_DMA_FSCK1_SLOT3		0x30000000
#define STA_DMA_FSCK1_SLOT4		0x40000000
#define STA_DMA_FSCK1_SLOT5		0x50000000
#define STA_DMA_FSCK2_SLOT0		0x80000000
#define STA_DMA_FSCK2_SLOT1		0x90000000
#define STA_DMA_FSCK2_SLOT2		0xa0000000
#define STA_DMA_FSCK2_SLOT3		0xb0000000
#define STA_DMA_FSCK2_SLOT4		0xc0000000
#define STA_DMA_FSCK2_SLOT5		0xd0000000
#define STA_DMA_FSYNC			0xf0000000

//DMABUS_CR
/*
#define STA_DMA_FSCK1_24KHZ		0x00000000
#define STA_DMA_FSCK1_16KHZ		0x00000080
#define STA_DMA_FSCK1_12KHZ		0x00000100
#define STA_DMA_FSCK1_8KHZ		0x00000180

#define STA_DMA_FSCK2_24KHZ		0x00000000
#define STA_DMA_FSCK2_16KHZ		0x00000200
#define STA_DMA_FSCK2_12KHZ		0x00000400
#define STA_DMA_FSCK2_8KHZ		0x00000600
*/
#define STA_DMA_FSCK12_24KHZ	0
#define STA_DMA_FSCK12_16KHZ	1
#define STA_DMA_FSCK12_12KHZ	2
#define STA_DMA_FSCK12_8KHZ		3


//=== TYPEDEFS/STRUCTS =======================================================

typedef u32						STAModule;
typedef u32						STAConnection;

/** DSP type: signed fractional (1 sign bit + 23 fractional bits) representing fixed point fractional values between [-1:1[ \n
    i.e. "-1" = 0x800000, "1" ~ 0x7fffff". \n
    'q23' can be used to set the EQ biquad's coefficients directly in the dsp format for example. */
typedef s32						q23;

typedef struct STALinLimiter_s {
	s32 ymax;
	s32 ymin;
	f32 k; // k is the linear interpolation slope in floating point
	f32 d; // k/d is the linear interpolation slope in fixed point
	f32 y0;
} STALinLimiter;

typedef struct STAConnector_s {
	STAModule	from;
	STAModule	to;
	u16			chout;			//from channel out
	u16			chin;			//to channel in
} STAConnector;


/*
// Used to hold decibel level in units of 0.1 dB
typedef s32 tAudioLevel_dB;

// Emerald-compatible fraction datatype [32 23]
// this means that fraction 0x7FFFFF (Eme) = 0x007FFFFF (ARM)
typedef s32 tAudioFraction;

// Used to hold time in units of 0.1 ms
typedef u32 tAudioDelayTime;
*/

//=== ENUMS ===============================================================

typedef enum STA_ErrorCode_e {
	STA_NO_ERROR			 = 0,
	STA_NOT_INITIALIZED		 = 1,
	STA_INVALID_MODULE		 = 2,
	STA_INVALID_MODULE_TYPE  = 3,
	STA_INVALID_CHANNEL		 = 4,
	STA_INVALID_PARAM		 = 5,
	STA_INVALID_VALUE		 = 6,
	STA_INVALID_TYPE		 = 7,
	STA_INVALID_SIZE		 = 8,
	STA_OUT_OF_MEMORY		 = 9,
	STA_NOT_ON_SAME_DSP 	 = 10,
	STA_DSP0_OUT_OF_XMEM	 = 11,
	STA_DSP1_OUT_OF_XMEM	 = 12,
	STA_DSP2_OUT_OF_XMEM	 = 13,
	STA_DSP0_OUT_OF_YMEM	 = 14,
	STA_DSP1_OUT_OF_YMEM	 = 15,
	STA_DSP2_OUT_OF_YMEM	 = 16,
	STA_DSP0_XMEM_NOT_LOADED = 17,
	STA_DSP1_XMEM_NOT_LOADED = 18,
	STA_DSP2_XMEM_NOT_LOADED = 19,
	STA_DSP0_YMEM_NOT_LOADED = 20,
	STA_DSP1_YMEM_NOT_LOADED = 21,
	STA_DSP2_YMEM_NOT_LOADED = 22,
	STA_DSP0_LIB_NOT_MATCHING= 23,
	STA_DSP1_LIB_NOT_MATCHING= 24,
	STA_DSP2_LIB_NOT_MATCHING= 25,
	STA_DSP0_XMEM_SIZE_ERROR = 26,
	STA_DSP1_XMEM_SIZE_ERROR = 27,
	STA_DSP2_XMEM_SIZE_ERROR = 28,
	STA_DSP0_YMEM_SIZE_ERROR = 29,
	STA_DSP1_YMEM_SIZE_ERROR = 30,
	STA_DSP2_YMEM_SIZE_ERROR = 31,
	STA_DSP0_NOT_STARTED	 = 32,
	STA_DSP1_NOT_STARTED	 = 33,
	STA_DSP2_NOT_STARTED	 = 34,
	STA_DSP0_PLAY_ERROR		 = 35,
	STA_DSP1_PLAY_ERROR		 = 36,
	STA_DSP2_PLAY_ERROR		 = 37,
	STA_DSP_STILL_RUNNING	 = 38,
	STA_DMA_OUT_OF_SLOT		 = 39,
	STA_DMA_STILL_RUNNING	 = 40,
	STA_INVALID_DMA_SRC_ADDR = 41,
	STA_INVALID_DMA_DST_ADDR = 42,
	STA_INVALID_DMA_TYPE	 = 43,
	STA_INVALID_FLOW		 = 44,
	STA_STILL_PLAYING	 	 = 45,
	STA_INVALID_AFTER_BUILT	 = 46,
	STA_INVALID_BEFORE_BUILT = 47,
	STA_INVALID_BEFORE_DSP_INIT_DONE = 48,
	STA_FAILED_TO_GENERATE_ID = 49,
	STA_ID_ALREADY_EXIST	 = 50,
	STA_WARN_CONNECTION_DO_EXIST = 51,
	STA_WARN_CONNECTION_NOT_EXIST = 52,
	STA_NUM_ERROR_CODE = 53
} STA_ErrorCode;


typedef enum STA_Dsp_e {
	STA_DSP0,
	STA_DSP1,
	STA_DSP2
} STA_Dsp;


//can be used in place of STAModule for connections
typedef enum STA_DspIO_e {
	STA_XIN0	= 200,
	STA_XIN1	= 201,
	STA_XIN2	= 202,
	STA_XOUT0	= 203,
	STA_XOUT1	= 204,
	STA_XOUT2	= 205,
} STA_DspIO;


//--- ASRCs -------------------------------------------
//NOT USED
/*
typedef enum STA_ASRC_e {
	STA_ASRC0,
	STA_ASRC1,
	STA_ASRC2,
	STA_ASRC3,
	STA_ASRC4,
	STA_ASRC5,
} STA_ASRC;

typedef enum STA_AIMUXSel_e {
	STA_NO_INPUT		= 0,
	STA_SAI1_RX1		= 1,		//from Tuner
	STA_SAI2_RX1		= 2,		//from CD/USB/SD
	STA_SAI2_RX2		= 3,
	STA_SAI2_RX3		= 4,
	STA_SAI2_RX4		= 5,
	STA_SAI3_RX1		= 6,		//from ADC
	STA_SAI3_RX2		= 7,
	STA_SAI3_RX3		= 8,
	STA_SAI3_RX4		= 9,
	STA_SAI4_RX1		= 10,		//from BT
	STA_SAI4_RX2		= 11,
	STA_SAI4_RX3		= 12,
	STA_SAI4_RX4		= 13,
	STA_SPDIF			= 14,		//from SPDIF
	STA_LPF_DI			= 15		//from DMABus
} STA_AIMUXSel;

typedef enum STA_AOMUXSel_e {
	STA_SAI2_TX1		= 2,		//to USB/SD
	STA_SAI3_TX1		= 6,		//to DAC
	STA_SAI3_TX2		= 7,
	STA_SAI3_TX3		= 8,
	STA_SAI3_TX4		= 9,
	STA_SAI4_TX1		= 10,		//to BT
	STA_SAI4_TX2		= 11,
	STA_SAI4_TX3		= 12,
	STA_SAI4_TX4		= 13,
	STA_ASRC_DO			= 14		//to DMABus
} STA_AOMUXSel;
*/
//--- DMA BUS ------------------------------------------

//--- Audio Modules -------------------------------------

/** \anchor STA_ModuleType_e STA module DSP types (used for buildflow)*/
//Note: This enum tries to match T_AudioFuncType of dsp_mem_def.h (EMERALD)
typedef enum STA_ModuleType_e {
	STA_XIN,
	STA_XOUT,
	STA_MUX_2OUT,					//mux have STA_MAX_MUX_INS inputs
	STA_MUX_4OUT,
	STA_MUX_6OUT,
	STA_MUX_8OUT,
	STA_MUX_10OUT,
	STA_CD_DEEMPH,
	STA_GAIN_STATIC_NCH,			//2ch by default
	STA_GAIN_SMOOTH_NCH,			//2ch by default
	STA_GAIN_LINEAR_NCH,			//2ch by default
	STA_LOUD_STATIC_DP,
	STA_LOUD_STATIC_STEREO_DP,		//can't be bound to mixer
	STA_TONE_STATIC_DP,
	STA_TONE_STATIC_STEREO_DP,
	STA_EQUA_STATIC_1BAND_DP,		//2ch by default
	STA_EQUA_STATIC_2BANDS_DP,		//2ch by default
	STA_EQUA_STATIC_3BANDS_DP,		//2ch by default
	STA_EQUA_STATIC_4BANDS_DP,		//2ch by default
	STA_EQUA_STATIC_5BANDS_DP,		//2ch by default
	STA_EQUA_STATIC_6BANDS_DP,		//2ch by default
	STA_EQUA_STATIC_7BANDS_DP,		//2ch by default
	STA_EQUA_STATIC_8BANDS_DP,		//2ch by default
	STA_EQUA_STATIC_9BANDS_DP,		//2ch by default
	STA_EQUA_STATIC_10BANDS_DP,		//2ch by default
	STA_EQUA_STATIC_11BANDS_DP,		//2ch by default
	STA_EQUA_STATIC_12BANDS_DP,		//2ch by default
	STA_EQUA_STATIC_13BANDS_DP,		//2ch by default
	STA_EQUA_STATIC_14BANDS_DP,		//2ch by default
	STA_EQUA_STATIC_15BANDS_DP,		//2ch by default
	STA_EQUA_STATIC_16BANDS_DP,		//2ch by default
	STA_EQUA_STATIC_1BAND_SP,		//2ch by default
	STA_EQUA_STATIC_2BANDS_SP,		//2ch by default
	STA_EQUA_STATIC_3BANDS_SP,		//2ch by default
	STA_EQUA_STATIC_4BANDS_SP,		//2ch by default
	STA_EQUA_STATIC_5BANDS_SP,		//2ch by default
	STA_EQUA_STATIC_6BANDS_SP,		//2ch by default
	STA_EQUA_STATIC_7BANDS_SP,		//2ch by default
	STA_EQUA_STATIC_8BANDS_SP,		//2ch by default
	STA_EQUA_STATIC_9BANDS_SP,		//2ch by default
	STA_EQUA_STATIC_10BANDS_SP,		//2ch by default
	STA_EQUA_STATIC_11BANDS_SP,		//2ch by default
	STA_EQUA_STATIC_12BANDS_SP,		//2ch by default
	STA_EQUA_STATIC_13BANDS_SP,		//2ch by default
	STA_EQUA_STATIC_14BANDS_SP,		//2ch by default
	STA_EQUA_STATIC_15BANDS_SP,		//2ch by default
	STA_EQUA_STATIC_16BANDS_SP,		//2ch by default
	STA_MIXE_2INS_NCH,				//2ch by default
	STA_MIXE_3INS_NCH,				//2ch by default
	STA_MIXE_4INS_NCH,				//2ch by default
	STA_MIXE_5INS_NCH,				//2ch by default
	STA_MIXE_6INS_NCH,				//2ch by default
	STA_MIXE_7INS_NCH,				//2ch by default
	STA_MIXE_8INS_NCH,				//2ch by default
	STA_MIXE_9INS_NCH,				//2ch by default
	STA_MIXE_BALANCE,				//balance for mixer
	STA_MIXE_FADER,					//fader   for mixer
	STA_COMP_1CH,					//compander
	STA_COMP_2CH,					//compander
	STA_COMP_4CH,					//compander
	STA_COMP_6CH,					//compander
	STA_LMTR_1CH,					//SW limiter
	STA_LMTR_2CH,					//SW limiter
	STA_LMTR_4CH,					//SW limiter
	STA_LMTR_6CH,					//SW limiter
	STA_CLIPLMTR_NCH,				//HW limiter: 2ch by default
	STA_DLAY_Y_2CH,
	STA_DLAY_Y_4CH,
	STA_DLAY_Y_6CH,
	STA_DLAY_X_2CH,
	STA_DLAY_X_4CH,
	STA_DLAY_X_6CH,
	STA_SINE_2CH,					//Sinewave Generator
	STA_PCMCHIME_12BIT_Y_2CH,		//PCM-chime (clic-clac) data stored in YMEM
	STA_PCMCHIME_12BIT_X_2CH,		//PCM-chime (clic-clac) data stored in XMEM
	STA_CHIMEGEN,					//Chime generator (mono pulses with varying frequencies)
	STA_BITSHIFTER_NCH,				//bit shifter: 2ch by default
	STA_PEAKDETECT_NCH,             //peak detector: 2ch by default
	STA_SPECMTR_NBANDS_1BIQ,		//spectrum meter Nbands x 1biquad per bands (9bands by default)
	STA_DCO_1CH,					//dc offset detection
	STA_DCO_4CH_MONOZCF,
	STA_MOD_LAST,					//to keep last but before STA_USER_0 (NOT a module!)
	STA_USER_0            = 0x1000, //lint !e960 misra2004 9.3 allow specific enum value
} STA_ModuleType;

/*
typedef enum STA_ModuleState_e {
	STA_IDLE,
	STA_UPDATE,
} STA_ModuleState;
*/

//--- CD Deemphasis ----------------------------------

typedef enum STA_DeemphMode_e {
	STA_DEEMPH_OFF,				/**< default */
	STA_DEEMPH_ON,
	STA_DEEMPH_AUTO
} STA_DeemphMode;

//--- Biquad ------------------------------------------

//Biquad Gain Scale (+-1/step dB) (WARNING: changing this value requires to recompile the STAudioLib)
#define STA_BIQ_GAIN_SCALE		10

/** Biquad filter parameter's types (see \ref STA_SetFilterParam) */
typedef enum STA_FilterParam_e {
	STA_GAIN,					/**< filter gain [-240:240] dB/10 (1 step = 0.1 dB) Ex: -143 for -14.3 dB */
	STA_QUAL,					/**< filter quality factor [1:100] (1/10) (1 step = 0.1) Ex: 55 for 5.5 */
	STA_FREQ,					/**< filter center frequency or frequency cut [20:20000] Hz (1 step = 1 Hz) Ex: 440 for 440 Hz */
	STA_TYPE					/**< filter type (see \ref STA_FilterType_e) */
} STA_FilterParam;

/** Biquad filter types */
typedef enum STA_FilterType_e {
	STA_BIQUAD_BYPASS,			/**< Bypass filter */
	STA_BIQUAD_LOWP_BUTTW_1,	/**< Low-Pass Butterworth order 1 filter */
	STA_BIQUAD_LOWP_BUTTW_2,	/**< Low-Pass Butterworth order 2 filter */
	STA_BIQUAD_HIGP_BUTTW_1,	/**< High-Pass Butterworth order 1 filter */
	STA_BIQUAD_HIGP_BUTTW_2,	/**< High-Pass Butterworth order 2 filter */
	STA_BIQUAD_BASS_SHELV_1,	/**< Bass-Shelving order 1 filter */
	STA_BIQUAD_BASS_SHELV_2,	/**< Bass-Shelving order 2 filter */
	STA_BIQUAD_TREB_SHELV_1,	/**< Trebble-Shelving order 1 filter */
	STA_BIQUAD_TREB_SHELV_2,	/**< Trebble-Shelving order 2 filter */
	STA_BIQUAD_BAND_BOOST_2,	/**< Band-boost, notch/peak order 2 filter */
	STA_BIQUAD_AP2_CB,			/**< All-pass filter (for phase shift) (CookBook) */
	STA_BIQUAD_LP2_CB,			/**< Low-Pass (CookBook) */
	STA_BIQUAD_HP2_CB,			/**< High-pass (CookBook) */
	STA_BIQUAD_BP2_CB,			/**< Band-pass (constant 0dB peak gain) (CookBook) */
	STA_BIQUAD_NOTCH_CB,		/**< Notch (CookBook) */
	STA_BIQUAD_PEAKING_CB,		/**< Peaking (CookBook) */
	STA_BIQUAD_LSHELF_CB,		/**< Low Shelf (CookBook) */
	STA_BIQUAD_HSHELF_CB,		/**< High Shelf (CookBook) */
	STA_BIQUAD_NUM				/* Keep last */
} STA_FilterType;

//for backward compatibility
#define STA_BIQUAD_ALLP_2 (STA_BIQUAD_AP2_CB)

//--- LOUDNESS ------------------------------------------

/** Loudness filter modes (see \ref STA_SetMode) */
typedef enum STA_LoudMode_e {
	STA_LOUD_OFF,				/**< default */
	STA_LOUD_AUTO,				/**< AUTO bass Q and auto Gain */
	STA_LOUD_MANUAL
} STA_LoudMode;

/** Loudness filters (Bass and Treble) */
typedef enum STA_LoudFilter_e {
	STA_LOUD_BASS,
	STA_LOUD_TREB
} STA_LoudFilter;

/** Loudness filter types */
typedef enum STA_LoudFilterType_e {
	STA_LOUD_SHELVING_1,
	STA_LOUD_SHELVING_2,
	STA_LOUD_BANDBOOST_2,		/**< default */
} STA_LoudFilterType;

//--- TONE CTRL ---------------------------------------

/** Tone filter modes (see \ref STA_SetMode) */
typedef enum STA_ToneMode_e {
	STA_TONE_OFF,				/**< default */
	STA_TONE_AUTO_BASS_Q,		/**< AUTO: Bass Q varies linearly between Qstart at Gmax and Qstop at Gmin */
	STA_TONE_MANUAL
} STA_ToneMode;

/** Tone filters B,M,T */
typedef enum STA_ToneFilter_e {
	STA_TONE_BASS,
	STA_TONE_MIDL,
	STA_TONE_TREB
} STA_ToneFilter;

/** Tone's bass and treble filter types (middle is bandboost2) */
typedef enum STA_ToneFilterType_e {
	STA_TONE_SHELVING_1,
	STA_TONE_SHELVING_2,
	STA_TONE_BANDBOOST_2,		/**< default */
	STA_TONE_DUALMODE			/**< only for bass */
} STA_ToneFilterType;

//--- EQ ------------------------------------------

/** EQ modes (see \ref STA_SetMode) */
typedef enum STA_EqMode_e {
	STA_EQ_OFF,					/**< default */
	STA_EQ_ON,
} STA_EqMode;

//--- BALANCE/FADER -------------------------------

#define STA_LEFT	0
#define STA_MIDDLE	1
#define STA_RIGHT	2
#define STA_FRONT	0
#define STA_REAR	2

//--- LIMITER -------------------------------------

/** SW Limiter modes (see \ref STA_SetMode) */
typedef enum STA_LimiterMode_e {
	STA_LMTR_OFF,	            /**< default: Inactive mode */
	STA_LMTR_ON,	            /**< Active mode */
} STA_LimiterMode;

/** SW Limiter parameter indices (see \ref STA_LimiterSetParams) */
typedef enum STA_LimiterParamIdx_e {
	STA_LMTR_IDX_PEAK_ATK_TIME,	/**< Limiter parameter index */
	STA_LMTR_IDX_PEAK_REL_TIME,	/**< Limiter parameter index */
	STA_LMTR_IDX_THRES,			/**< Limiter parameter index */
	STA_LMTR_IDX_HYSTERESIS,	/**< Limiter parameter index */
	STA_LMTR_IDX_ATTACK_TIME,	/**< Limiter parameter index */
	STA_LMTR_IDX_RELEASE_TIME,	/**< Limiter parameter index */
	STA_LMTR_IDX_ATTENUATION,	/**< Limiter parameter index */
	STA_LMTR_IDX_DYN_GAIN,		/**< Limiter parameter index */
	STA_LMTR_IDX_PEAK,			/**< Limiter parameter index */
	STA_LMTR_NUMBER_OF_IDX		/**< Number of indexed parameters */
} STA_LimiterParamIdx;

/** SW Limiter parameters (can be bitwise OR'ed) (see \ref STA_LimiterSetParams) */
typedef enum STA_LimiterParam_e {
	STA_LMTR_PEAK_ATK_TIME		= (u32)1 << STA_LMTR_IDX_PEAK_ATK_TIME,	/**< Peak Attack Time:                              [1:32767] ms/10. Ex: 21987 for 2198.7 ms */
	STA_LMTR_PEAK_REL_TIME		= (u32)1 << STA_LMTR_IDX_PEAK_REL_TIME,	/**< Peak Release Time:                             [1:32767] ms/10. Ex: 21987 for 2198.7 ms */
	STA_LMTR_THRES				= (u32)1 << STA_LMTR_IDX_THRES,			/**< Threshold from which the limiter apply:        [-1200:0] dB/10 (1 step = 0.1 dB). Ex: -1012 for -101.2 dB */
	STA_LMTR_HYSTERESIS			= (u32)1 << STA_LMTR_IDX_HYSTERESIS,	/**< Hysteresis:                                    [0:600]   dB/100 (1 step = 0.01 dB). Ex: 112 for 1.12 dB */
	STA_LMTR_ATTACK_TIME		= (u32)1 << STA_LMTR_IDX_ATTACK_TIME,	/**< Attack Time to reach 90% of the target level:  [1:32767] ms/10. Ex: 21987 for 2198.7 ms */
	STA_LMTR_RELEASE_TIME		= (u32)1 << STA_LMTR_IDX_RELEASE_TIME,	/**< Release Time to reach 90% of the target level: [1:32767] ms/10. Ex: 21987 for 2198.7 ms */
	STA_LMTR_ATTENUATION		= (u32)1 << STA_LMTR_IDX_ATTENUATION,	/**< Additionnal Attenuation:                       [0:1200]  dB/10 (1 step = 0.1 dB). Ex: 1012 for 101.2 dB */
	STA_LMTR_DYN_GAIN			= (u32)1 << STA_LMTR_IDX_DYN_GAIN,		/**< Dynamic gain of the Limiter in dB/10 (read only) */
	STA_LMTR_PEAK				= (u32)1 << STA_LMTR_IDX_PEAK			/**< Dynamic max peak of input signals in dB/10 (read only) */
} STA_LimiterParam;

//--- CLIPLIMITER -------------------------------------

/** Clip Limiter parameter modes (see \ref STA_SetMode) */
typedef enum STA_ClipLimiterMode_e {
	STA_CLIPLMTR_OFF,			/**< default, Inactive mode */
	STA_CLIPLMTR_ATT_REL_MODE,	/**< Active in Attack-Release mode */
	STA_CLIPLMTR_FULL_ATT_MODE	/**< Active in Full-Attack mode */
} STA_ClipLimiterMode;

/** Clip Limiter parameter indices (see \ref STA_ClipLimiterSetParams) */
typedef enum STA_ClipLimiterParamIdx_e {
	STA_CLIPLMTR_IDX_CLIP_BITMASK,			/**< Clip Limiter parameter index */
	STA_CLIPLMTR_IDX_CLIP_POLARITY,			/**< Clip Limiter parameter index */
	STA_CLIPLMTR_IDX_MAX_ATTENUATION,		/**< Clip Limiter parameter index */
	STA_CLIPLMTR_IDX_MIN_ATTENUATION,		/**< Clip Limiter parameter index */
	STA_CLIPLMTR_IDX_ATTACK_TIME,			/**< Clip Limiter parameter index */
	STA_CLIPLMTR_IDX_RELEASE_TIME,			/**< Clip Limiter parameter index */
	STA_CLIPLMTR_IDX_ADJ_ATTENUATION,		/**< Clip Limiter parameter index */
	STA_CLIPLMTR_NUMBER_OF_IDX				/**< Number of indexed parameters */
} STA_ClipLimiterParamIdx;

/** Clip Limiter parameters (can be bitwise OR'ed) (see \ref STA_ClipLimiterSetParams) */
typedef enum STA_ClipLimiterParam_e {
	STA_CLIPLMTR_CLIP_BITMASK       = (u32)1 << STA_CLIPLMTR_IDX_CLIP_BITMASK,		/**< Bitmask indicating on which bit of the 24bit-word is mapped the clip signal: 1<<bit (default=1<<18) */
	STA_CLIPLMTR_CLIP_POLARITY      = (u32)1 << STA_CLIPLMTR_IDX_CLIP_POLARITY,		/**< Logical level indicating clipping 0<<bit:Low (default), 1<<bit:High */
	STA_CLIPLMTR_MAX_ATTENUATION    = (u32)1 << STA_CLIPLMTR_IDX_MAX_ATTENUATION, 	/**< Maximum attenuation target to apply when it is clipping [0:1200]    dB/10. Ex: 327 for 32.7 dB */
	STA_CLIPLMTR_MIN_ATTENUATION    = (u32)1 << STA_CLIPLMTR_IDX_MIN_ATTENUATION,	/**< Minimum attenuation target to apply even it is not clipping [0:1200]    dB/10. Ex: 16 for 1.6 dB */
	STA_CLIPLMTR_ATTACK_TIME        = (u32)1 << STA_CLIPLMTR_IDX_ATTACK_TIME, 		/**< Convergence time to reach 90% of max attenuation target [0:4000000] ms/10. Ex: 48 for 4.8 ms */
	STA_CLIPLMTR_RELEASE_TIME       = (u32)1 << STA_CLIPLMTR_IDX_RELEASE_TIME,  	/**< Convergence time to reach 90% of min attenuation target [0:4000000] ms/10. Ex: 1500 for 150.0 ms */
	STA_CLIPLMTR_ADJ_ATTENUATION    = (u32)1 << STA_CLIPLMTR_IDX_ADJ_ATTENUATION   	/**< Adjustment attenuation for Full-Attack mode only [0:1200]    dB/10. Ex: 9 for 0.9 dB */
} STA_ClipLimiterParam;


//--- COMPANDER -------------------------------------

/** Compander modes (see \ref STA_SetMode) */
typedef enum STA_CompanderMode_e {
	STA_COMP_OFF,					/**< default: Inactive mode */
	STA_COMP_ON,					/**< Active mode */
} STA_CompanderMode;

/** Compander parameter indices (see \ref STA_CompanderSetParams) */
typedef enum STA_CompanderParamIdx_e {
	STA_COMP_IDX_MEAN_MAX_MODE,		/**< Base parameter index */
	STA_COMP_IDX_AVG_FACTOR,		/**< Base parameter index */
	STA_COMP_IDX_ATTENUATION,		/**< Base parameter index */
	STA_COMP_IDX_CPR_THRES,			/**< Compressor parameter index */
	STA_COMP_IDX_CPR_SLOPE,			/**< Compressor parameter index */
	STA_COMP_IDX_CPR_HYSTERESIS,	/**< Compressor parameter index */
	STA_COMP_IDX_CPR_ATK_TIME,		/**< Compressor parameter index */
	STA_COMP_IDX_CPR_REL_TIME,		/**< Compressor parameter index */
	STA_COMP_IDX_EXP_THRES,			/**< Expander parameter index */
	STA_COMP_IDX_EXP_SLOPE,			/**< Expander parameter index */
	STA_COMP_IDX_EXP_HYSTERESIS,	/**< Expander parameter index */
	STA_COMP_IDX_EXP_ATK_TIME,		/**< Expander parameter index */
	STA_COMP_IDX_EXP_REL_TIME,		/**< Expander parameter index */
	STA_COMP_NUMBER_OF_IDX			/**< Number of indexed parameters */
} STA_CompanderParamIdx;

/** Compander parameters (can be bitwise OR'ed) (see \ref STA_CompanderSetParams) */
typedef enum STA_CompanderParam_e {
	STA_COMP_MEAN_MAX_MODE		= (u32)1 << STA_COMP_IDX_MEAN_MAX_MODE,	/**< mean or max mode:  0=mean or 1=max, peak = mean(|CH0|..|CHi|) or max(|CH0|..|CHi|) */
	STA_COMP_AVG_FACTOR			= (u32)1 << STA_COMP_IDX_AVG_FACTOR,	/**< average factor:    >= 0          ms/10 (1 step = 0.1 ms). Ex: 453 for 45.3 ms */
	STA_COMP_ATTENUATION		= (u32)1 << STA_COMP_IDX_ATTENUATION,	/**< attenuation:   [0:240] dB/10 (1 step = 0.1 dB). Ex: 62 for 6.2 dB */
	STA_COMP_CPR_THRES			= (u32)1 << STA_COMP_IDX_CPR_THRES,		/**< Compressor's Threshold from which compression applies:         [-1920:0] dB/10 (1 step = 0.1 dB). Ex: -1612 for -161.2 dB */
	STA_COMP_CPR_SLOPE			= (u32)1 << STA_COMP_IDX_CPR_SLOPE,		/**< Compressor's Slope:             [-32768:0] 1/32768 (1 step = 1/32768). Ex: -16384 for -0.5 */
	STA_COMP_CPR_HYSTERESIS		= (u32)1 << STA_COMP_IDX_CPR_HYSTERESIS,/**< Compressor's Hysteresis:        [0:19200] dB/100 (1 step = 0.01 dB). Ex: 1612 for 16.12 dB */
	STA_COMP_CPR_ATK_TIME		= (u32)1 << STA_COMP_IDX_CPR_ATK_TIME,	/**< Compressor's Attack Time:  >= 0     ms/10 (1 step = 0.1 ms). Ex: 453 for 45.3 ms */
	STA_COMP_CPR_REL_TIME		= (u32)1 << STA_COMP_IDX_CPR_REL_TIME,	/**< Compressor's Release Time: >= 0     ms/10 (1 step = 0.1 ms). Ex: 453 for 45.3 ms */
	STA_COMP_EXP_THRES			= (u32)1 << STA_COMP_IDX_EXP_THRES,		/**< Expander's Threshold from which the expander applies:         [-1920:0] dB/10 (1 step = 0.1 dB). Ex: -1612 for -161.2 dB */
	STA_COMP_EXP_SLOPE			= (u32)1 << STA_COMP_IDX_EXP_SLOPE,		/**< Expander's Slope:             [-32768:0] 1/32768 (1 step = 1/32768). Ex: -16384 for -0.5 */
	STA_COMP_EXP_HYSTERESIS		= (u32)1 << STA_COMP_IDX_EXP_HYSTERESIS,/**< Expander's Hysteresis:        [0:19200] dB/100 (1 step = 0.01 dB). Ex: 1612 for 16.12 dB */
	STA_COMP_EXP_ATK_TIME		= (u32)1 << STA_COMP_IDX_EXP_ATK_TIME,	/**< Expander's Attack Time:  >= 0     ms/10 (1 step = 0.1 ms). Ex: 453 for 45.3 ms */
	STA_COMP_EXP_REL_TIME		= (u32)1 << STA_COMP_IDX_EXP_REL_TIME,	/**< Expander's Release Time: >= 0     ms/10 (1 step = 0.1 ms). Ex: 453 for 45.3 ms */
} STA_CompanderParam;

//--- SINEWAVE GENERATOR --------------------------

/** Sinewave modes (see \ref STA_SetMode) */
typedef enum STA_SinewaveMode_e {
	STA_SINE_OFF,			/**< default */
	STA_SINE_ON,
} STA_SinewaveMode;

//--- PCM CHIME ----------------------------------

/** PCM chime parameters
* @note The end of cycle of one loop is determined by the longest of left or right delays (see \ref STA_PCMPlay).
*/
typedef struct STA_PCMChimeParams_s {
	s32	 playCount;			/**< repeat count (< 0 means infinite loop) */
	u32  leftDelay;			/**< left  start delay in msec */
	u32  leftPostDelay;		/**< left  post  delay in msec */
	u32  rightDelay;		/**< right start delay in msec */
	u32  rightPostDelay;	/**< right post  delay in msec */
} STA_PCMChimeParams;

//--- POLYPHONIC CHIME / BEEP ---------------------

/** Polychime/Beep ramp types */
typedef enum STA_RampType_e {
	STA_RAMP_STC,			/**< static ramp (= hold time) */
	STA_RAMP_EXP,			/**< exponential ramp */
	STA_RAMP_LIN,			/**< linear ramp */
	STA_RAMP_SMP,			/**< static ramp (= hold time) in sample */
} STA_RampType;

/** Polychime/Beep ramp parameters */
typedef struct STA_RampParams_s {
	u32  type: 4;			/**< ramp type STA_RampType */
	u32  ampl:10;			/**< ramp target amplitude [0:1000](+/-0.1%) */
	u32  msec:18;			/**< ramp duration  (+/-1 ms) or number of samples (up to 262000 ~5.4ms) if type = STA_RAMP_SMP */
	u32  freq:32;			/**< ramp frequency (+/-0.01 Hz) */
} STA_RampParams;

/** Polychime parameters */
typedef struct STA_ChimeParams_s {
//	const STAModule *pMasterChime;		/**< pointer to the master's chime module for polychime synchro (set to NULL for simple chime)*/
	STA_RampParams	*ramps;				/**< ramp array */
	u16				numRamps;			/**< number of ramps */
	u16				postRepeatRampIdx;	/**< ...or any value > numRamps if no post-repeat ramps */
	s16 			repeatCount;		/**< repeat count (< 0 means infinite loop) */
} STA_ChimeParams;

/** Polychime/PCMchime set/get parameters */
//note: below Set/Get params works for both PolyChime and PCMChime
typedef enum STA_ChimeGenParam_e {
	STA_CHIME_REPEAT_COUNT,			/**< repeat count (< 0 means infinite loop; overwrite Chime's repeatCount) (r/w) */
	STA_CHIME_PLAYING,				/**< getter returns 0 if no ramp is being played. (r only) */
	STA_CHIME_POST_REPEAT_RAMPS,	/**< enable/disable the post repeat ramps (0=disabled, 1=enabled) */
	STA_CHIME_MASTER_CHIME			/**< set the master's chime module for polychime synchro (set to 0 for simple chime)*/
	//RFU: add more params to update beep chime dynamically
} STA_ChimeGenParam;


/*
The following STA_BeepParams can be used to program the chime gen with STA_ChimeGenBeep().
RFU: Possibility to modify dynamically and "seamlessly" the beep chime parameters (freq, peaktime....) using STA_ChimeGenSetParam().
*/
/** Beep parameters */
typedef struct STA_BeepParams_s {
	//--- same as sinewaveGen-----
	u32  freq;				/**< frequency (+/-0.01 Hz) */
	u32  peakTime:22;		/**< peak duration (msec) (<170000 ms) */
	u32  peakAmpl:10;		/**< peak amplitude [0:1000](+/-0.1%) */
	//--- "beep" params ----------
	u32  atkType:4;			/**< attack ramp type STA_RampType */
	u32  atkTime:28;		/**< attack ramp time (+/-1 ms) (<170000 ms)*/
	u32  relType:4;			/**< release ramp type STA_RampType */
	u32  relTime:28;		/**< release ramp time (+/-1 ms) (<170000 ms)*/
	u32  muteTime;			/**< mute time between pulse (<170000 ms)*/
	s32  repeatCount;		/**< repeat count ( < 0 means infinite loop) */
} STA_BeepParams;

//--- DCO DETECT MODULE  -------------------------

/** DCO modes (see \ref STA_SetMode) */
typedef enum STA_DCOMode_e {
	STA_DCO_OFF				= 0,    /**< default */
	STA_DCO_CALIBRATE		= 1<<0,
	STA_DCO_RUN				= 1<<1,
} STA_DCOMode;

/** DCO parameters */
typedef enum STA_DCOParam_e {
	STA_DCO_FRAC_DELAY,
	STA_DCO_WIN_THRES,
	STA_DCO_DCOFFSET,
	STA_DCO_ALERT_THRES,		//score alert threshold
	STA_DCO_SCORE_WINDOW,
	STA_DCO_SCORE,				//(get only)
	STA_DCO_STATS_MATCH11,		//(get only)
	STA_DCO_STATS_MATCH00,		//(get only)
	STA_DCO_STATS_MISSED,		//(get only)
	STA_DCO_STATS_FALSE,		//(get only)
	STA_DCO_SWZCF,				//(get only)
	STA_DCO_HWZCF,				//(get only)
} STA_DCOParam;

//--- PEAK DETECTOR  --------------------------

/** Peak detector flags (can be bitwise OR'ed) */
typedef enum STA_PeakFlag_e {
	STA_PEAK_LINEAR			= 0 << 0,
	STA_PEAK_DB				= 1 << 0,
	STA_PEAK_RESET			= 1 << 1
} STA_PeakFlag;

//--- USER MODULE  --------------------------

/** User module states */
typedef struct STA_UserModuleInfo_s {
	STA_ModuleType	m_type;				//STA module type
	u32				m_dspType;			//DSP module type (or dsp USER type)
	u32				m_nin;				//number of input channels
	u32				m_nout;				//number of output channels
	u32				m_sizeofParams;		//size of HOST params (in Bytes)
	u32				m_wsizeofData;		//size of DSP data  (XMEM) (in Words !), can be updated in AdjustDspMemSize()
	u32				m_wsizeofCoefs;		//size of DSP coefs (YMEM) (in Words !), can be updated in AdjustDspMemSize()
	void*			m_hostParams;		//ptr to HOST params allocated by STA_AddModule()
	void*			m_dspData;			//ptr to DSP data  (XMEM) allocated by STA_BuildFlow()
	void*			m_dspCoefs;			//ptr to DSP coefs (YMEM) allocated by STA_BuildFlow()
	u32				m_xdata;			//data word offset in DSP XMEM base
	u32				m_ycoef;			//coef word offset in DSP YMEM base
	u32				m_mainCycleCost;	//DSP cycles for main function
	u32				m_updateCycleCost;	//DSP cycles for update function
	void (*InitModule)		(struct STA_UserModuleInfo_s* info); //required for USR modules
	void (*AdjustDspMemSize)(struct STA_UserModuleInfo_s* info); //[opt.]called by BuildFlow() to adjust wsizeofData and wsizeofCoefs before to allocate DSP mems
	void (*InitDspData) 	(struct STA_UserModuleInfo_s* info); //called by BuildFlow() after that dspData and dspCoefs have received an address
	void (*UpdateDspCoefs)	(struct STA_UserModuleInfo_s* info); //called during dynamic update
	void*(*GetDspInAddr)	(const struct STA_UserModuleInfo_s* info, u32 in);   //called by BuildFlow()
	void*(*GetDspOutAddr)	(const struct STA_UserModuleInfo_s* info, u32 out);  //called by BuildFlow()
	void (*SetMode)			(struct STA_UserModuleInfo_s* info, u32 mode); //[opt.] called during dynamic update
} STA_UserModuleInfo;


//--- GET INFO  --------------------------

typedef enum STA_Info_e {
	STA_INFO_LIB_VERSION	= 1 << 4,
	STA_INFO_DSP_VERSION	= 2 << 4,
	STA_INFO_DCO_VERSION	= 3 << 4,
	STA_INFO_IS_NO_FLOAT	= 4 << 4,
	STA_INFO_SAMPLING_FREQ	= 5 << 4,
	STA_INFO_ERROR 			= 6 << 4,
	STA_INFO_LAST_ERROR 	= 7 << 4,
} STA_Info;


typedef enum STA_ModuleInfo_e {
	STA_INFO_ID,
	STA_INFO_TYPE,
	STA_INFO_NUM_IN_CHANNELS,
	STA_INFO_NUM_OUT_CHANNELS,
	STA_INFO_NUM_MIXER_IN_PER_CHANNELS,
	STA_INFO_NUM_BANDS,
	STA_INFO_IS_NCH_CHANGEABLE,
	STA_INFO_MODE,
	STA_INFO_SIZEOF_PARAMS,
	STA_INFO_WSIZEOF_XDATA,
	STA_INFO_WSIZEOF_YDATA,
	STA_INFO_MAX_DSP_CYCLES,
	STA_INFO_XDATA,
	STA_INFO_YDATA,
} STA_ModuleInfo;


#ifdef LINUX_OS
typedef struct {
	void *dev;
	int cutversion;
	void *audio_cr_base;
	void *audio_p4_base;
	void *dsp0_base;
	void *dsp1_base;
	void *dsp2_base;
} STA_Params;
#endif

//--- ENABLE / DISABLE  --------------------------

typedef enum STA_EnableParam_e {
	STA_FREE_CYCLE_COUNTER,
	STA_CALL_TRACE
} STA_EnableParam;


//=== API calls ==============================================================

//--------------
// API managment
//--------------

STA_API u32 STA_GetInfo(STA_Info info);

STA_API STA_ErrorCode STA_GetError(void);	//return the first error and reset it.

STA_API void STA_Init(void*);				//can pass an OS specifc handle (eg. linux device *dev)
STA_API void STA_Reset(void);				//clean all
STA_API void STA_Exit(void);				//clean all and exit

STA_API void STA_Enable(STA_EnableParam option);
STA_API void STA_Disable(STA_EnableParam option);
STA_API void STA_Push(STA_EnableParam option, s32 val);
STA_API void STA_Pop(STA_EnableParam option);


//-----------
// DSP
//-----------

STA_API void STA_Play(void);
STA_API void STA_Stop(void);
//note: start/stop DSPs having a valid flow, and start/stop the DMABus transfers

STA_API void STA_LoadDSP( STA_Dsp core );
STA_API void STA_LoadDSP_ext( STA_Dsp core, const char *Paddr, u32 Psize, const char *Xaddr, u32 Xsize, const char *Yaddr, u32 Ysize );

STA_API void STA_StartDSP( STA_Dsp core );
STA_API void STA_StopDSP( STA_Dsp core );
//note: STA_StartDSP() and STA_StopDSP() starts/stops a given dsp without checking a valid flow.
//		Please call STA_Play()/STA_Stop() instead.

STA_API u32  STA_WaitDspReady(u32 dspMask, u32 timerMsec);
/*RFU*/ STA_API void STA_Early_Init(void);

/**
* @anchor		STA_GetFreeCycles
*				Get the free cycles of the 3 dsps. Need to first call STA_Enable(STA_FREE_CYCLE_COUNTER).
* @param[out]	freeCycles[3]: array returning the free cycles for dsp0, 1 and 2.
* @return		the free cycles of DSP0
* @note			When enabling STA_FREE_CYCLE_COUNTER, the audio starts to glitch due to the measure per cycle overflow.
* @note			Synchronous function. */
STA_API u32  STA_GetFreeCycles(u32 freeCycles[3]);

STA_API void STA_PrintFreeCycles(u32 rateInSec);		//enable, get and print

STA_API u32 STA_GetMaxDspCycleCost( STA_Dsp core );
/**
* @anchor		STA_GetDspMemCost
*				Get the current used X and Y memory in words for a given DSP core.
* @param[in]	STA_Dsp core: core 0,1 or 2.
* @param[out]	u32 *xmemWords: pointer to provide X memory size info.
* @param[out]	u32 *ymemWords: pointer to provide Y memory size info.
* @note			The information will not be provided for a NULL pointer.
* @note			Synchronous function. */
STA_API void STA_GetDspMemCost(STA_Dsp core, u32 *xmemWords, u32* ymemWords);
/**
* @anchor		STA_GetDspMaxMem
*				Get the maximum available X and Y memory in words for a given DSP core.
* @param[in]	STA_Dsp core: core 0,1 or 2.
* @param[out]	u32 *xmemMaxWords: pointer to provide X memory size info.
* @param[out]	u32 *ymemMaxWords: pointer to provide Y memory size info.
* @note			The information will not be provided for a NULL pointer.
* @note			Synchronous function. */
STA_API void STA_GetDspMaxMem(STA_Dsp core, u32 *xmemMaxWords, u32* ymemMaxWords);

//-----------
// IRQ
//-----------

STA_API void STA_SetDspISR(STA_Dsp core, void (*isr)(STA_Dsp core));
STA_API void STA_SetFSyncISR(void (*isr)(void));

//-----------
// DMABUS
//-----------
STA_API int  STA_DMAAddTransfer( u16 srcAddr, u16 dstAddr, u32 numCh );
STA_API int  STA_DMAAddTransfer2( u16 srcAddr, u16 dstAddr, u32 numCh, u32 type);
STA_API void STA_DMAAddTransferv( const u16* srcDstNch, u32 size );
STA_API void STA_DMASetTransfer(u16 srcAddr, u16 dstAddr, u32 numCh, u32 dmaPos, u32 type);
STA_API void STA_DMAStart(void);
STA_API void STA_DMAStop(void);
STA_API void STA_DMAReset(void);    //empty and stop
STA_API void STA_DMASetFSyncCK1(u32 ck1sel);
STA_API void STA_DMASetFSyncCK2(u32 ck2sel);

//-----------
// AIF ROUTING
//-----------
//STA_API void STA_ConnectASRC( STA_ASRC asrc, STA_AIMUXSel inSel, STA_AOMUXSel outSel );

//-----------
// BUILD FLOW
//-----------
STA_API STAModule STA_AddModule( STA_Dsp dsp, STA_ModuleType type );
STA_API STAModule STA_AddModule2(STA_Dsp dsp, STA_ModuleType type, u32 id, u32 tags, const char* name);

STA_API void STA_AddModulev( STA_Dsp dsp, const STA_ModuleType* types, u32 num, STAModule* modules );
//return: if 'modules' is not NULL, the new modules'handles are returned in this array

STA_API STAModule STA_AddUserModule(  STA_Dsp dsp, STA_ModuleType type, const STA_UserModuleInfo *info );
STA_API STAModule STA_AddUserModule2( STA_Dsp dsp, STA_ModuleType type, const STA_UserModuleInfo *info, u32 id, u32 tags, const char* name);

STA_API void STA_DeleteModule( STAModule* module );

STA_API void STA_Connect( STAModule from, u32 chout, STAModule to, u32 chin );
STA_API void STA_Disconnect( STAModule from, u32 chout, STAModule to, u32 chin );

STA_API void STA_ReconnectFrom( STAModule oldFrom, u32 oldChout, STAModule to, u32 chin, STAModule newFrom, u32 newChout );
STA_API void STA_ReconnectTo( STAModule from, u32 chout, STAModule oldTo, u32 oldChin, STAModule newTo, u32 newChin );

STA_API void STA_Connectv( const STAConnector* ctors, u32 num, STAConnection* connections );
//return: if 'connections' is not NULL, the new connections'handles are returned in this array

STA_API void STA_ConnectFromIndices( const STAConnector* ctors, u32 num, const STAModule* mods, STAConnection* cons );
//note: 'ctors' gives the indices used to find the corresponding module handle from 'mods'
//return: if 'cons' is not NULL, the new connections'handles are returned in this array


STA_API void STA_BuildFlow(void);


//--------------
// MODULE COMMON
//--------------

/**
* @anchor		STA_SetMode
*				Set a mode to a STA module (supporting different modes).
* @param[in]	module:   a valid STA module
* @param[in]	mode:     the mode to set depending on the module type (see \ref STA_LoudMode_e, \ref STA_ToneMode_e, \ref STA_EqMode_e, \ref STA_LimiterMode_e, \ref STA_ClipLimiterMode_e, \ref STA_CompanderMode_e, \ref STA_SinewaveMode_e, \ref STA_DCOMode_e).
* @return		void
* @note			Synchronous function. */
STA_API void STA_SetMode(STAModule module, u32 mode);
STA_API u32  STA_GetMode(STAModule module);

STA_API STAModule STA_SetNumChannels( STAModule module, u32 nch );
//Note: Must be called before STA_BuildFlow()
//param: module = STA_GAIN_xxx,
//				= STA_MIXE_xxx  (nch upto STA_MAX_MIXER_CH)
//return: return the new handle !!!

STA_API void STA_SetNumOfUpdateSlots( STA_Dsp dsp, u32 numOfUpdateSlots);
//Note: Must be called before STA_BuildFlow()
//param numOfUpdateSlots <= STA_MAX_UPDATE_SLOTS

STA_API void STA_SetUpdateSlot(STAModule module, u32 slot);
//Used for LIMITER and COMPANDER
//param slot < numOfUpdateSlots

STA_API u32 STA_GetModuleInfo(STAModule module, STA_ModuleInfo info);

STA_API const char* STA_GetModuleName(STAModule module, char* name);
//param 'name': if not null must point to a memory owned by the caller
//returns the module name (pointing to a char[] owned by STA.

//--------------------
// EQ / BIQUAD FILTER
//--------------------
/*****************************************************************************************************************
 note: All the below STA_SetFilterXXXX apis apply to any module types based on biquad filters (bands),
       mainly EQ, TONECTRL, LOUNDESS and SPECTRUM_METER modules.
******************************************************************************************************************/

/**
* @anchor		STA_SetFilterNumBands
*				Sets the number of bands for the given biquad-based module \n
*				/!\ only works for SPECTRUM_METER for now.
* @param[in]	module:    a valid module of type STA_SPECMTR_xxx
* @param[in]	numBands:  the number of bands
* @return		the new STAModule handle, which may have changed when changing the number of bands
* @note			Must be called before the BuildFlow()
* @note			Synchronous function. */
STA_API STAModule STA_SetFilterNumBands(STAModule module, u32 numBands);

/**
* @anchor		STA_SetFilterParam
*				Sets the value of the given STA_FilterParam param to the given band of a given biquad-based filter module
*				(mainly EQ, ToneCtrl, Loudness and spectrum meter).
* @param[in]	module:   a valid module of type STA_EQ_xxx, STA_TONE_xxx, STA_LOUD_xxx, STA_SPECMTR_xxx
* @param[in]	band:     the band index in the range [0:N-1], where N is number of bands of the STA module
* @param[in]	param:    the parameter to set between Gain, Quality factor (Q), Frequency cut or filter Type (see \ref STA_FilterParam_e enum)
* @param[in]	val:      the parameter's value (see \ref STA_FilterParam_e range)
* @return		void
* @note			STA_TYPE param refer to the biquad filter types, which are also applicable to the toneCrl and Loudness's internal biquad filters.
* @note			Gain is not used for HP, LP and ALLP.
* @note			Synchronous function. */
STA_API void STA_SetFilterParam(STAModule module, u32 band, STA_FilterParam param, s32 val);

/**
* @anchor		STA_GetFilterParam
*				Gets the value of the given STA_FilterParam param from the given band of a given biquad-based filter module.
* @param[in]	module:   a valid module of type STA_EQ_xxx, STA_TONE_xxx, STA_LOUD_xxx, STA_SPECMTR_xxx
* @param[in]	band:     the band index in the range [0:N-1], where N is number of bands of the STA module
* @param[in]	param:    the parameter to get between Gain, Quality factor (Q), Frequency cut or filter Type (see \ref STA_FilterParam_e enum)
* @return		the parameter's value (see \ref STA_FilterParam_e range)
* @note			STA_TYPE param refer to the biquad filter types, which are also applicable to the toneCrl and Loudness's internal biquad filters.
* @note			Gain is not used for HP, LP and ALLP.
* @note			Synchronous function. */
STA_API s32  STA_GetFilterParam(STAModule module, u32 band, STA_FilterParam param);

/**
* @anchor		STA_SetFilterParams
* 				Sets the values of the given STA_FilterParam param to all bands of a given biquad-based filter module.
* @param[in]    module:   a valid module of type STA_EQ_xxx, STA_TONE_xxx, STA_LOUD_xxx, STA_SPECMTR_xxx
* @param[in]	param:    the parameter to set between Gain, Quality factor (Q), Frequency cut or filter Type (see \ref STA_FilterParam_e enum)
* @param[in]	values:   array of numbands size containing the parameter's values to set for all bands (see \ref STA_FilterParam_e range)
* @return		void
* @note			STA_TYPE param refer to the biquad filter types, which are also applicable to the toneCrl and Loudness's internal biquad filters.
* @note			Gain is not used for HP, LP and ALLP.
* @note			Synchronous function. */
STA_API void STA_SetFilterParams(STAModule module, STA_FilterParam param, const s16* values);

/**
* @anchor		STA_GetFilterParams
* 				Gets the param values from all bands of a given biquad-based filter module
* @param[in]	module:   a valid module of type STA_EQ_xxx, STA_TONE_xxx, STA_LOUD_xxx, STA_SPECMTR_xxx
* @param[in]	param:    the parameter to set between Gain, Quality factor (Q), Frequency cut or filter Type (see \ref STA_FilterParam_e enum)
* @param[out]	values:   array of numbands size to return the parameter's values for each bands (see \ref STA_FilterParam_e range)
* @return 		void
* @note			STA_TYPE param refer to the biquad filter types, which are also applicable to the toneCrl and Loudness's internal biquad filters.
* @note			Gain is not used for HP, LP and ALLP.
* @note			Synchronous function. */
STA_API void STA_GetFilterParams(STAModule module, STA_FilterParam param, s16* values);

/**
* @anchor		STA_SetFilter
* 				set the filter G,Q,F parameters of a given band.
* @param[in]	module:   a valid module of type STA_EQ_xxx, STA_TONE_xxx, STA_LOUD_xxx, STA_SPECMTR_xxx
* @param[in]	band:     the band index in the range [0:N-1], where N is number of bands of the STA module
* @param[in]	G:        the Gain.           range:[-240:240] dB/10 (1 step = 0.1dB). Ex: -123 for -12.3dB. Low and High pass filter range:[-240:60]
* @param[in]	Q:        the Quality factor. range:[1:100] 1/10 (1 step = 0.1). Ex: 45 for 4.5 quality
* @param[in]	F:        the cut or center Frequency.  range:[20:20000] Hz, (1 step = 1 Hz)
* @return		void
* @note			Gain is not used for HP, LP and ALLP.
* @note			Synchronous function. */
STA_API void STA_SetFilter(STAModule module, u32 band, s32  G, s32  Q, s32  F);

/**
* @anchor		STA_GetFilter
* 				Get the filter G,Q,F parameters of a given band.
* @param[in]	module:   a valid module of type STA_EQ_xxx, STA_TONE_xxx, STA_LOUD_xxx, STA_SPECMTR_xxx
* @param[in]	band:     the band index in the range [0:N-1], where N is number of bands of the STA module
* @param[out]	G:        the Gain.           range:[-240:240] dB/10 (1 step = 0.1dB). Ex: -123 for -12.3dB. Low and High pass filter range:[-240:60]
* @param[out]	Q:        the Quality factor. range:[1:100] 1/10 (1 step = 0.1). Ex: 45 for 4.5 quality
* @param[out]	F:        the cut or center Frequency.  range:[20:20000] Hz, (1 step = 1 Hz)
* @return		void
* @note			Gain is not used for HP, LP and ALLP.
* @note			Synchronous function. */
STA_API void STA_GetFilter(STAModule module, u32 band, s32* G, s32* Q, s32* F);

/**
* @anchor		STA_SetFilterv
* 				set the filter G,Q,F parameters for a given band, passed using an array[3].
* @param[in]	module:   a valid module of type STA_EQ_xxx, STA_TONE_xxx, STA_LOUD_xxx, STA_SPECMTR_xxx
* @param[in]	band:     the band index in the range [0:N-1], where N is number of bands of the STA module
* @param[in]	GQF:      an array[3] containing the G,Q,F parameters' value: \n
* 				GQF[0]:   the Gain.           range:[-240:240] dB/10 (1 step = 0.1dB). Ex: -123 for -12.3dB. Low and High pass filter range:[-240:60] \n
* 				GQF[1]:   the Quality factor. range:[1:100] 1/10 (1 step = 0.1). Ex: 45 for 4.5 quality \n
* 				GQF[2]:   the cut Frequency.  range:[20:20000] Hz, (1 step = 1 Hz)
* @return		void
* @note			Gain is not used for HP, LP and ALLP.
* @note			Synchronous function. */
STA_API void STA_SetFilterv(STAModule module, u32 band, const s16 GQF[3]);

/**
* @anchor		STA_SetFilterType
* 				Sets the biquad filter type of the given band.
* @param[in]	module:   a valid module of type STA_EQ_xxx, STA_TONE_xxx, STA_LOUD_xxx, STA_SPECMTR_xxx
* @param[in]	band:     the band index in the range [0:N-1], where N is number of bands of the STA module
* @param[in]	type:     the filter type (see \ref STA_FilterType_e).
* @return		void
* @note			Synchronous function. */
STA_API void STA_SetFilterType(STAModule module, u32 band, STA_FilterType type);

/**
* @anchor		STA_GetFilterType
* 				Gets the biquad filter type of the given band.
* @param[in]	module:   a valid module of type STA_EQ_xxx, STA_TONE_xxx, STA_LOUD_xxx, STA_SPECMTR_xxx
* @param[in]	band:     the band index in the range [0:N-1], where N is number of bands of the STA module
* @return		the filter type (see \ref STA_FilterType_e).
* @note			Synchronous function. */
STA_API STA_FilterType STA_GetFilterType(STAModule module, u32 band);

/**
* @anchor		STA_SetFilterGains
* 				Sets the gains of all bands from an array of gains and for the selected (masked) band. \n
* 				Usefull to set an EQ preset for example.
* @param[in]	module:    a valid module of type STA_EQ_xxx, STA_TONE_xxx, STA_LOUD_xxx, STA_SPECMTR_xxx
* @param[in]	bandmask:  bitmask to mask the bands to be updated (bit i for the band i).
* @param[in]	Gains:     an array of numband size containing the gains' values to set to each band (Gains[i] for the band i)
* @return		void
* @note			Gain is not used for HP, LP and ALLP.
* @note			Synchronous function. */
STA_API void STA_SetFilterGains(STAModule module, u32 bandmask, const s16* Gains);

/**
* @anchor		STA_SetFilterHalvedCoefs
* 				Sets the 6 biquad parameters (5 coefs + 1 bshift) values of the given band.
* @param[in]	module:   a valid module of type STA_EQ_xxx, STA_TONE_xxx, STA_LOUD_xxx, STA_SPECMTR_xxx
* @param[in]	band:     the band index in the range [0:N-1], where N is number of bands of the STA module
* @param[in]	coefs:    array[6] containing the biquad coefficients (in \ref q23) to set to the given band.
* @return		void
* @note			Synchronous function.
* @note			The 6 coefs must be halved and b-shitfed: \n
*				coefs[6] = {(b0/2)>>bshift, (b1/2)>>bshift, (b2/2)>>bshift, a1/2, a2/2, bshift},
*				with a0 = 1.0 (implicitly) */
STA_API void STA_SetFilterHalvedCoefs( STAModule module, u32 band, const q23* coefs );

/**
* @anchor		STA_GetFilterHalvedCoefs
* 				Gets the 6 biquad parameters (5 coefs + 1 bshift) values of the given band.
* @param[in]	module:   a valid module of type STA_EQ_xxx, STA_TONE_xxx, STA_LOUD_xxx, STA_SPECMTR_xxx
* @param[in]	band:     the band index in the range [0:N-1], where N is number of bands of the STA module
* @param[out]	coefs:    array[6] to return the biquad coefficients (in \ref q23) of the given band.
* @return		void
* @note			Synchronous function.
* @note			The 6 coefs must be halved and b-shitfed: \n
*				coefs[6] = {(b0/2)>>bshift, (b1/2)>>bshift, (b2/2)>>bshift, a1/2, a2/2, bshift},
*				with a0 = 1.0 (implicitly) */
STA_API void STA_GetFilterHalvedCoefs( STAModule module, u32 band, q23* coefs );

/**
* @anchor		STA_SetFilterHalvedCoefsAll
* 				Sets the 6 biquad parameters (5 coefs + 1 bshift) values of ALL masked bands directly in the DSP format (q23).
* @param[in]	module:    a valid module of type STA_EQ_xxx, STA_TONE_xxx, STA_LOUD_xxx, STA_SPECMTR_xxx
* @param[in]	bandmask:  bitmask to mask the bands to be updated (bit i for the band i).
* @param[in]	allCoefs:  pointer to all biquad coefficients (in \ref q23).
* @return		void
* @note			Synchronous function.
* @note			The BANDS*6 coefs must be halved and b-shitfed: \n
*				coefs[BANDS*6] = {(b0/2)>>bshift, (b1/2)>>bshift, (b2/2)>>bshift, a1/2, a2/2, bshift, ...
*				continues for next bands }, with all a0 = 1.0 (implicitly)  */
STA_API void STA_SetFilterHalvedCoefsAll( STAModule module, u32 bandmask, const q23* allCoefs );

/**
* @anchor		STA_GetFilterHalvedCoefsAll
* 				Gets the 6 biquad parameters (5 coefs + 1 bshift) values of ALL masked bands directly in the DSP format (q23).
* @param[in]	module:    a valid module of type STA_EQ_xxx, STA_TONE_xxx, STA_LOUD_xxx, STA_SPECMTR_xxx
* @param[in]	bandmask:  bitmask to mask the bands to be updated (bit i for the band i).
* @param[out]	allCoefs:  pointer to all biquad coefficients (in \ref q23).
* @return		void
* @note			Synchronous function.
* @note			The BANDS*6 coefs must be halved and b-shitfed: \n
*				coefs[BANDS*6] = {(b0/2)>>bshift, (b1/2)>>bshift, (b2/2)>>bshift, a1/2, a2/2, bshift, ...
*				continue for next bands }, with all a0 = 1.0 (implicitly) */
STA_API void STA_GetFilterHalvedCoefsAll( STAModule module, u32 bandmask, q23* allCoefs );


//-------------
// MUX SELECTOR
//-------------
/**
* @anchor		STA_MuxSet
* 				Sets the internal connections of the given Muxer.
* @param[in]	module:  a valid module of type STA_MUX_xxx
* @param[in]	sels:    an array of size equals to the number of output channels containing, for each of the output channel,
*	 					 the input channel from 0 to STA_MAX_MUX_INS-1 to which it is connected. \n
*						 By default all outputs are connected to input 0.
* @return		void
* @note			Synchronous function. */
STA_API void STA_MuxSet(STAModule module, const u8* sels);
STA_API void STA_MuxGet(STAModule module, u8* sels);

/**
* @anchor		STA_MuxSetOutChannel
* 				Sets a single internal connection of the given Muxer
* @param[in]	module:  a valid module of type STA_MUX_xxx
* @param[in]	inch:    input channel (from 0 to STA_MAX_MUX_INS-1)
* @param[in]	outch:   output channel (from 0 to the max imposed by the MUX's type)
* @return		void
* @note			Synchronous function. */
STA_API void STA_MuxSetOutChannel(STAModule module, u32 inch, u32 outch);


//-----------
// GAIN
//-----------

/**
* @anchor		STA_GainSetGain
* 				Sets the gain to the given channel of the given Gain module.
* @param[in]	module:  a valid module of type STA_GAIN_xxx
* @param[in]	ch:      the channel to set.
* @param[in]	Gain:    value of the gain to set [-1200:240] dB/10 (1 step = 0.1dB). Ex: -612 for -61.2dB
* @return		void
* @warning		for STATIC gain: \n
*				if STA_GainSetGain() is called before BuildFlow(), sets initGain/initScale, \n
*				if called after, then updates directly the gain/scale on DSP side.
* @note			Synchronous function. */
STA_API void STA_GainSetGain( STAModule module, u32 ch, s32 Gain );
STA_API s32  STA_GainGetGain( STAModule module, u32 ch);

/**
* @anchor		STA_GainSetGains
* 				Sets the gains of the all masked channels of the given Gain module with the same value.
* @param[in]	module:   a valid module of type STA_GAIN_xxx
* @param[in]	chMask:   bitmask to select the channels to be updated (bit i for channel i).
* @param[in]	Gain:     value of the gain to set [-1200:240] dB/10 (1 step = 0.1dB). Ex: -612 for -61.2dB
* @return		void
* @warning		for STATIC gain: \n
*				if STA_GainSetGain() is called before BuildFlow(), sets initGain/initScale, \n
*				if called after, then updates directly the gain/scale on DSP side
* @note			Synchronous function */
STA_API void STA_GainSetGains( STAModule module, u32 chMask, s32 Gain );

/**
* @anchor		STA_SmoothGainSetTC
* 				Set the ramp's up and down time constants of the given channel of the given smooth gain module.
* @param[in]	module:    a valid module of type STA_GAIN_SMOOTH_xxx or STA_GAIN_LINEAR_xxx
* @param[in]	ch:        the channel to set.
* @param[in]	msUp:      up time [0:10000] in milliseconds
* @param[in]	msDown:    down time [0:10000] in milliseconds
* @return		void
* @note			Synchronous function */
STA_API void STA_SmoothGainSetTC( STAModule module, u32 ch, u32 msUp, u32 msDown );

/**
* @anchor		STA_SmoothGainSetRamp
* 				Set the ramp's up and down time constants of the masked channels of the given smooth gain module.
* @param[in]	module:    a valid module of type STA_GAIN_SMOOTH_xxx or STA_GAIN_LINEAR_xxx
* @param[in]	chMask:    bitmask to select the channels to be updated (bit i for channel i).
* @param[in]	msUp:      up time [0:10000] in milliseconds
* @param[in]	msDown:    down time [0:10000] in milliseconds
* @return		void
* @note			Synchronous function */
STA_API void STA_SmoothGainSetRamp( STAModule module, u32 chMask, u32 msUp, u32 msDown );
STA_API void STA_SmoothGainGetRamp( STAModule module, u32 ch, u32* msUp, u32* msDown );

/**
* @anchor		STA_GainSetLinearGainsAndShifts \n
* 				/!\ ONLY for STATIC gain: set the DSP coefs directly, without conversion: \n
*					sample_out = (sample_in * linGain) << leftShift. \n
* @param[in]	module:     a valid module of type STA_GAIN_STATIC_xxx
* @param[in]	chMask:     bitmask to select the channels to be updated (bit i for channel i).
* @param[in]	linGain:    linear gain in \ref q23
* @param[in]	leftShift:  left shift if positive, or an arithmetic right shift if negative.
* @warning		ONLY for STATIC gain.
* @note			Useful for example to use a static gain as bitshifter or to apply a polarity: \n
* 				- To use as bitshifter set 'linGain = 0x400000' and 'leftShift = shift + 1' \n
* 				- To use as positive polarity set 'linGain = 0x400000' and 'leftShift = 1'  \n
* 				- To use as negative polarity set 'linGain = -0x400000' and 'leftShift = 1'
* @note			Synchronous function. */
STA_API void STA_GainSetLinearGainsAndShifts( STAModule module, u32 chMask, q23 linGain, s32 leftShift);

/**
* @anchor		STA_GainSetMaxGains
* 				Sets the allowed max positive gain (in dB) from 0 to +96 dB (+24 dB by default) for the given channel(s).
* @param[in]	module:    a valid module of type STA_GAIN_SMOOTH_xxx or STA_GAIN_LINEAR_xxx
* @param[in]	chMask:    bitmask to select the channels to be updated (bit i for channel i).
* @param[in]	maxGain:   new max gain [0:96] dB
* @return		void
* @note			Due to dsp 24bit fixed precision setting a max Gain above +24 dB decreases the quality of the end-of-ramp.
* @note			Synchronous function */
STA_API void STA_GainSetMaxGains( STAModule module, u32 chMask, u32 maxGain );
STA_API void STA_GainSetMaxGain( STAModule module, u32 ch, u32 maxGain );
STA_API u32  STA_GainGetMaxGain( STAModule module, u32 ch);

/**
* @anchor		STA_GainSetPolarity
* 				Sets the polariry (+/-1) for the given channel(s).
* @param[in]	module:    a valid module of type STA_GAIN_SMOOTH_xxx or STA_GAIN_LINEAR_xxx
* @param[in]	ch:    	   the channel to be updated
* @param[in]	polarity:  the polarity as +1 or -1
* @return		void
* @note			Synchronous function */
STA_API void STA_GainSetPolarity( STAModule module, u32 ch, s32 polarity );
STA_API s32  STA_GainGetPolarity( STAModule module, u32 ch);

//------------
// BIT SHIFTER
//------------
/**
* @anchor		STA_BitShifterSetLeftShifts
* 				Sets the bit-shifting to apply to the channel i: out[i] = in[i] << leftShift[i] \n
*				which does a lshift if leftShift[i] is positve, or an arithmetic rshift if negative.
* @param[in]	module:    a valid module of type STA_BITSHIFTER_NCH
* @param[in]	chmask:    the masked channels to be updated (bit i for the channel i)
* @param[in]	leftShift: the left shift
* @return		void
* @note			Synchronous function */
STA_API void STA_BitShifterSetLeftShifts( STAModule module, u32 chmask, s32 leftShift);

//--------------
// PEAK DETECTOR
//--------------
STA_API void STA_PeakDetectorResetLevel( STAModule module, u32 channel);
//DSP function reset the peak of the given channel
//param: channel: range is [0..N-1] with N set using STA_SetNumChannels before STA_BuildFlow

STA_API s32 STA_PeakDetectorGetLevel( STAModule module, u32 channel, u32 flags);
//DSP function returns the peak of the requested channel
//note: use STA_SetMode(mod, ch) to enable/disable the peak detection up to the first ch channels
//      (e.g ch=0 disable detection on all channels)
//param: channel: range is [0..N-1] with N set using STA_SetNumChannels before STA_BuildFlow
//param: flags: bitwise | of
//   STA_PEAK_LINEAR: returns the peak in linear amplitude (q23)
//   STA_PEAK_DB:	  returns the peak in dB/10
//   STA_PEAK_RESET:  reset the peak


//-----------
// TONE
//-----------
STA_API void STA_ToneSetTypes( STAModule module, STA_ToneFilterType bassType, STA_ToneFilterType trebType );

STA_API void STA_ToneSetAutoBassQLevels( STAModule module, s32 Gstart, s32 Gstop, s32 Qstart, s32 Qstop );
//param G:  +/-24*STA_BIQ_STEP dB,    (step 1/STA_BIQ_STEP dB)
//param Q:       [1:100]/10 Ex: 39 for 3.9 quality factor

//-----------
// LOUDNESS
//-----------
STA_API void STA_LoudSetTypes( STAModule module, STA_LoudFilterType bassType, STA_LoudFilterType trebType );

STA_API void STA_LoudSetAutoBassQLevels( STAModule module, s32 Gstart, s32 Gstop, s32 Qstart, s32 Qstop );
//param G:  +/-24*STA_BIQ_STEP dB,    (step 1/STA_BIQ_STEP dB)
//param Q:       [1:100] /10 Ex: 39 for 3.9 quality factor

STA_API void STA_LoudSetAutoGLevels( STAModule module, STA_LoudFilter filter, s32 Vstart, s32 Vstop, s32 Gmin, s32 Gmax);
//param gains:   +/-24*STA_BIQ_STEP dB,    (step 1/STA_BIQ_STEP dB)
//param volumes: [-1200:0] dB/10,         (step 0.1 dB) Ex: -612 for -61.2 dB
//note: only applies to single channel Loudness (STA_LOUD_STATIC_DP)


//-----------
// MIXER
//-----------
/*
* NOTE about MIXERs:
*	A mixer of type MIXE_iINS_NCH mixer has N output channels (=nout). Each out channel is a weigthed combination of i inputs per channels (=ninpc).
*	But the module only exposes 2 insets of ninpc channels to mix separately the Lefts and the Rights.
*	Thus,
*	the number of external input channels =    2 * ninpc (=nin)
*	the number of internal input gains    = nout * ninpc
*/

/**
* @anchor		STA_MixerSetRelativeInGain
*				Set the input gain of the relative input channel of the given output channel.
* @param[in]	module:   a valid STA module of type STA_MIXE_xxx
* @param[in]	outCh:    the output channel (0 to nout-1)
* @param[in]	inRel:    the relative input channel (0 to ninpc-1)
* @param[in]	inGain:   the input gain in [-1200:0] dB/10 (1 step = 0.1dB). Ex: -612 for -61.2 dB
* @return		void */
STA_API void STA_MixerSetRelativeInGain(STAModule module, u32 outCh, u32 inRel, s32 inGain);
STA_API s32  STA_MixerGetRelativeInGain(STAModule module, u32 outCh, u32 inRel);

/**
* @anchor		STA_MixerSetAbsoluteInGain
*				Set the input gain of the absolute input channel.
* @param[in]	module:   a valid STA module of type STA_MIXE_xxx
* @param[in]	inAbs:    the absolute input channel (0 to nout*ninpc-1)
* @param[in]	inGain:   the input gain in [-1200:0] dB/10 (1 step = 0.1dB). Ex: -612 for -61.2 dB
* @return		void */
STA_API void STA_MixerSetAbsoluteInGain(STAModule module, u32 inAbs, s32 inGain);
STA_API s32  STA_MixerGetAbsoluteInGain(STAModule module, u32 inAbs);

#define STA_MixerSetInGain  (STA_MixerSetAbsoluteInGain)
#define STA_MixerGetInGain  (STA_MixerGetAbsoluteInGain)

/** \anchor STA_MixerSetInGains */
/** param Gains: [-1200:0] dB/10 (1 step = 0.1dB). Ex: -612 for -61.2 dB */
STA_API void STA_MixerSetInGains(STAModule module, u32 outMask, u32 inMask, s32 inGain);

/** \anchor STA_MixerSetChannelInGains */
/** param Gains: [-1200:0] dB/10 (1 step = 0.1dB). Ex: -612 for -61.2 dB */
STA_API void STA_MixerSetChannelInGains(STAModule module, u32 outch, const s16* inGains);

/**
* @anchor		STA_MixerSetOutGain
*				Set the output gain of the corresponding output channel.
* @param[in]	module:   a valid STA module of type STA_MIXE_xxx
* @param[in]	outch:    the output channel (0 to nout-1)
* @param[in]	Gain:     the output gain in [-1200:0] dB/10 (1 step = 0.1dB). Ex: -612 for -61.2 dB
* @return		void */
STA_API void STA_MixerSetOutGain(STAModule module, u32 outch, s32 Gain);
STA_API s32  STA_MixerGetOutGain(STAModule module, u32 outch);

/** \anchor STA_MixerSetOutGains */
/** param Gain: [-1200:0] dB/10 (1 step = 0.1dB). Ex: -612 for -61.2 dB */
STA_API void STA_MixerSetOutGains(STAModule module, u32 outMask, s32 Gain);
//param Gain: -120/+24 dB x10,   (step 0.1 dB)

/** \anchor STA_MixerSetInRamp */
/** param ts: [0:1000000000] ms (1 step = 1ms) */
STA_API void STA_MixerSetInRamp(STAModule module, u32 inch, u32 msUp, u32 msDown);

/** \anchor STA_MixerSetInRamps */
/** param ts: [0:1000000000] ms (1 step = 1ms) */
STA_API void STA_MixerSetInRamps(STAModule module, u32 outMask, u32 inMask, u32 ts);
//RFU:  void STA_MixerSetInRamps(STAModule module, u32 outMask, u32 inMask, u32 msUp, u32 msDown);

/** \anchor STA_MixerSetOutRamp */
/** param ts: [0:1000000000] ms (1 step = 1ms) */
STA_API void STA_MixerSetOutRamp(STAModule module, u32 outch, u32 msUp, u32 msDown);

/** \anchor STA_MixerSetOutRamps */
/** param ts: [0:1000000000] ms (1 step = 1ms) */
STA_API void STA_MixerSetOutRamps(STAModule module, u32 outMask, u32 ts);
//RFU:  void STA_MixerSetOutRamps(STAModule module, u32 outMask, u32 msUp, u32 msDown);


/** \anchor  STA_MixerSetMute  \deprecated (OBSOLETE: no effects) */
STA_API void STA_MixerSetMute(STAModule module, u32 chMask, bool mute, bool hard, s32 depth);

/** \anchor  STA_MixerSetMuteOrVolumeTS  \deprecated (OBSOLETE: no effects) */
STA_API void STA_MixerSetMuteOrVolumeTS(STAModule module, u32 chMask, bool setvol, const u16 ts);

/** \anchor  STA_MixerSetVolume \deprecated (OBSOLETE: no effects) */
STA_API void STA_MixerSetVolume(STAModule module, u32 chMask, const s16 volume);

//DEPRECATED
STA_API void STA_MixerBindTo(STAModule module, u32 chm, STAModule bafalo, u32 chb);
//Bind a mixer channel to Balance, Fader or Loudness (not dual)
//note: chb is only used for Balance and Fader

//DEPRECATED
STA_API void STA_MixerSetBalance(STAModule mixerBalance, s32 balance);
//To use with Balance or Fader bound to mixer.
//Negative balance sets attenuation to the left/rear, while positive balance to the right/front
//param balance: [-1200:1200] dB/10 (1 step = 0.1dB). Ex: -612 for -61.2 dB

//DEPRECATED
STA_API void STA_MixerSetBalanceLimit(STAModule mixerBalance, s32 limit);
//To use with Balance or Fader bound to mixer.
//param limit: [-1200:0] dB/10 (1 step = 0.1dB). Ex: -612 for -61.2 dB

//DEPRECATED
STA_API void STA_MixerSetBalanceCenter(STAModule mixerBalance, s32 start, s32 stop, s32 maxi);
//To use with Balance or Fader bound to mixer.
//param start/stop/max: [-1200:0] dB/10 (1 step = 0.1dB). Ex: -612 for -61.2 dB

//-----------
// COMPANDER
//-----------
/**
* @anchor		STA_CompanderSetParams
* 				Set the parameters of the given compander module
* @param[in]	module:    a valid module of type STA_COMP_xxx
* @param[in]	paramMask: bitmask to select the params to update (see \ref STA_CompanderParam_e).
* @param[in] 	*params:   array containing the params values presented in the order of the corresponding bit (see \ref STA_CompanderParamIdx_e).
* @return		void
* @note			E.g.: To modify compressor's slope, set bit paramMask |= STA_COMP_CPR_SLOPE and set params[STA_COMP_IDX_CPR_SLOPE] = value.
* @note			Synchronous function. */
STA_API void STA_CompanderSetParams(STAModule module, u32 paramMask, const s16 *params);

//-----------
// LIMITER
//-----------
/**
* @anchor		STA_LimiterSetParams
* 				Set the parameters of the given soft limiter module
* @param[in]	module:    a valid module of type STA_LMTR_xxx
* @param[in]	paramMask: bitmask to select the params to update (see \ref STA_LimiterParam_e).
* @param[in]	*params:   array containing the params values presented in the order of the corresponding bit (see \ref STA_LimiterParamIdx_e).
* @return		void
* @note			E.g.: To modify hysteresis, set bit paramMask |= STA_LMTR_HYSTERESIS and set params[STA_LMTR_IDX_HYSTERESIS] = value.
* @note			Synchronous function. */
STA_API void STA_LimiterSetParams(STAModule module, u32 paramMask, const s16 *params);
STA_API s32  STA_LimiterGetParam(STAModule module, u32 param);

//-------------
// CLIP LIMITER
//-------------
/**
* @anchor		STA_ClipLimiterSetParams
* 				Set the parameters of the given Clip Limiter module
* @param[in]	module:    a valid module of type STA_CLIPLMTR_xxx
* @param[in]	paramMask: bitmask to select the params to update (see \ref STA_ClipLimiterParam_e).
* @param[in]	params:    array containing the params values presented in the order of the corresponding bit (see \ref STA_ClipLimiterParamIdx_e).
* @return		void
* @note			E.g.: To modify attack time, set bit paramMask |= STA_CLIPLMTR_ATTACK_TIME and set params[STA_CLIPLMTR_IDX_ATTACK_TIME] = value.
* @note			Synchronous function. */
STA_API void STA_ClipLimiterSetParams(STAModule module, u32 paramMask, const s32 *params);
STA_API s32  STA_ClipLimiterGetParam(STAModule module, u32 param);

//-----------
// DELAY
//-----------
STA_API void STA_DelaySetLengths(STAModule module, u32 chmask, u32 delayLength);
STA_API void STA_DelaySetLength(STAModule module, u32 ch, u32 delayLength);
STA_API u32  STA_DelayGetLength(STAModule module, u32 ch);
//Sets the same delay length (in number of samples) to the masked channels
//note: must be called before STA_Buildflow()
//param delayLength: 1 to ??? (max depend on the available dsp XMEM or YMEM i.e. depending on the Delay type)

STA_API void STA_DelaySetDelays(STAModule module, u32 chmask, u32 delay);
STA_API void STA_DelaySetDelay(STAModule module, u32 ch, u32 delay);
STA_API u32  STA_DelayGetDelay(STAModule module, u32 ch);
//Sets the current delay to the masked channels
//note: delay must be inferior or equal to delayLength-1 of the corresponding channel.
//param delay: 0 to delayLength - 1

//-------------
// SINEWAVE GEN
//-------------
/**
* @anchor		STA_SinewaveGenSet
* 				Starts the sinewave generator playing a continuous sinewave at the given frequency, duration and L,R gains.
* @param[in]	module:       a valid module of type STA_SINE_xxx
* @param[in]	freq:         10/20kHz [1000:2000000] Hz/100 (1 step = 0.01 Hz).  Ex: 52030 for 520,30 Hz
* @param[in]	msec:         duration in millisecond (<174762 ms) (set -1 or lower for infinite duration)
* @param[in]	gainL, gainR: [-1200:0] dB/10 (1 step = 0.1 dB). Ex: -612 for -61.2 dB
* @return		void
* @note			Synchronous function. */
STA_API void STA_SinewaveGenSet(STAModule module, u32 freq, u32 msec, s32 gainL, s32 gainR);

/**
* @anchor		STA_SinewaveGenPlay
* 				Starts the sinewave generator playing a continous sinewave at the given frequency, duration and L,R amplitudes.
* @param[in]	module:       a valid module of type STA_SINE_xxx
* @param[in]	freq:         10/20kHz [1000:2000000] Hz/100 (1 step = 0.01 Hz).  Ex: 52030 for 520,30 Hz
* @param[in]	msec:         duration in millisecond (<174762 ms) (set -1 or lower for infinite duration)
* @param[in]	ampL, ampR:   amplitude in [-1000:1000] (1 step = 0.1% full scale). Ex: 624 for 62.4%
* @return		void
* @note			Synchronous function. */
STA_API void STA_SinewaveGenPlay(STAModule module, u32 freq, u32 msec, s32 ampL, s32 ampR);

//-------------
// PCM-CHIME
//-------------
/**
* @anchor		STA_PCMSetMaxDataSize
*				Sets the PCM chime player's dsp buffer size (in bytes), hence the max size of the PCM data that can be loaded and played.
* @param[in]	module:       a valid module of type STA_PCMCHIME_xxx
* @param[in]	maxbytesize:  maximum size in bytes (should be a multiple of 4 bytes)
* @return		void
* @warning		Must be called BEFORE the BuildFlow() to reserve the DSP memory.
* @note			Synchronous function. */
STA_API void STA_PCMSetMaxDataSize(STAModule module, u32 maxbytesize);

/**
* @anchor		STA_PCMLoadData_16bit
* 				Loads pcm data into the PCM chime player's DSP memory. Data must be 2x16bit, signed, little endian, sampled at 48 kHz.
* @param[in]	module:       a valid module of type STA_PCMCHIME_xxx
* @param[in]	data:         a pointer to the pcm data
* @param[in]	bytesize:     size of the data in bytes (should be a multiple of 4 bytes)
* @return		void
* @warning		The max size of the PCM chime's dsp buffer must have been set with \ref STA_PCMSetMaxDataSize before the BuildFlow().
* @warning		Must be called AFTER the BuildFLow() as loading directly into the DSP memory.
* @note			When loading into the DSP, the data are internally converted into 2x12bit PCM format.
* @note			Synchronous function. */
STA_API void STA_PCMLoadData_16bit(STAModule module, const void* data, u32 bytesize);

/**
* @anchor		STA_PCMPlay
* 				Starts playing the PCM waveform previously loaded with \ref STA_PCMLoadData_16bit.
* @param[in]	module:       a valid module of type STA_PCMCHIME_xxx
* @param[in]	params:       pointer to the PCM chime parameters.
* @return		void
* @note			Synchronous function. */
STA_API void STA_PCMPlay(STAModule module, const STA_PCMChimeParams* params);

/**
* @anchor		STA_PCMStop
* 				Stops playing the PCM waveform.
* @param[in]	module:       a valid module of type STA_PCMCHIME_xxx
* @return		void
* @note			Synchronous function. */
STA_API void STA_PCMStop(STAModule module);

//note: STA_ChimeGenSetParam() / STA_ChimeGenGetParam() also works for both PolyChime and PCMChime

//------------------
// POLY-CHIME / BEEP
//------------------
STA_API void STA_ChimeGenSetMaxNumRamps(STAModule module, u32 maxNumRamps);
STA_API u32  STA_ChimeGenGetMaxNumRamps(STAModule module);
//must be called BEFORE BuildFlow()

STA_API void STA_ChimeGenPlay(STAModule module, const STA_ChimeParams *params);
STA_API void STA_ChimeGenBeep(STAModule module, const STA_BeepParams  *params);
STA_API void STA_ChimeGenStop(STAModule module, STA_RampType stopRampType, u32 stopRampTime);
STA_API void STA_ChimeGenSetParam(STAModule module, STA_ChimeGenParam param, s32 value);
STA_API s32  STA_ChimeGenGetParam(STAModule module, STA_ChimeGenParam param);

//--------------------
// SPECTRUM METER
//--------------------
/**
* @anchor		STA_SpectrumMeterGetPeaks
* 				Reads the spectrum peaks (for each band) in linear (q23) or in scaled dB (negative integer
*				with a scaling factor set with \ref STA_SpectrumMeterSetdBscale) depending on the peakFlag. \n
*				In dB, the peaks are in the range -120/0 dB.
* @param[in]	module:    a valid module of type STA_SPECMTR_xxx
* @param[in]	peaks:     pointer to an array to receive the peak values. Its size must be at least equal to the number of bands.
* @param[in]	peakFlag:  = STA_PEAK_LINEAR or STA_PEAK_DB
* @return		void
* @note			Synchronous function. */
STA_API void STA_SpectrumMeterGetPeaks(STAModule module, s32* peaks, u32 peakFlag);

STA_API void STA_SpectrumMeterSetdBscale(STAModule module, s32 dBscale);
STA_API void STA_SpectrumMeterSetDecayFactor(STAModule module, u32 msec);

//--------------------
// DC-OFFSET DETECTION
//--------------------

STA_API void STA_DCOSetMode(STAModule module, u32 chmask, u32 modemask);
//chmask: channel mask. Masked channels are applied with the given mode.
//modemask: bitwise combo of STA_DCOMode.

STA_API void STA_DCOSetParam(STAModule module, u32 chmask, STA_DCOParam param, s32 val);
//chmask: channel mask. Masked channels are updated with the same value for the given param.
//param:					value:
//	STA_DCO_FRAC_DELAY	-> [0 to 100*STA_MAX_DCO_DELAY]	(step: +-0.01 sample)
//	STA_DCO_WIN_THRES	-> [0 to 1000] 					(step: +-0.1% full scale)
//	STA_DCO_DCOFFSET	-> [-1000 to 1000] 				(step: +-0.1% full scale)
//	STA_DCO_ALERT_THRES	-> [0 to SCORE_WINDOW]
//	STA_DCO_SCORE_WINDOW

STA_API s32 STA_DCOGetParam(STAModule module, u32 ch, STA_DCOParam param);
//param: same as for STA_DCOSetParam + the following parameters:
//	STA_DCO_SCORE
//	STA_DCO_STATS_MATCH11
//	STA_DCO_STATS_MATCH00
//	STA_DCO_STATS_MISSED
//	STA_DCO_STATS_FALSE

//-------------
// UTILITY APIS
//-------------
STA_API s32 STA_DBtoLinear(s32 db, s32 dbscale, s32 *rshift, bool clamp);

//Description: Reciprocal of db = dbscale * 20 * log(g)
//Input      : db:      Gain in dB, prescaled by 'dbscale'
//           : dbscale: dbscale = 1 to 10
//           : clamp:   if true, the linear gain is clamped to ONE (0x7FFFFF)
//Output     : rshift:  if != NULL, returned right shift used to scale the linear gain below ONE (0x7FFFFF)
//Return     : linear gain in fixed (signed.24) scaled by >> rshift



#ifdef __cplusplus
}
#endif

#endif /* STAUDIO_H_ */
