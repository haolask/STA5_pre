/***********************************************************************************/
/*!
*
*  \file      module.c
*
*  \brief     <i><b> STAudioLib Module manager </b></i>
*
*  \details   Each filters on DSP is represented by a Module type, which contains
*             all the internal parameters.
*
*  \author    Quarre Christophe
*
*  \author    (original version) Quarre Christophe
*
*  \version
*
*  \date      2013/04/22
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
* Copyright (c) 2014 STMicroelectronics - All Rights Reserved
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

#include "internal.h"


//some defines for modules info
#define COMP_DLAY_SIZE				80
#define LMTR_DLAY_SIZE				80
//#define DLAY_SIZE					500	//same as HIT2

#define MCH							(STA_MAX_MIXER_CH) /*MIXER_MAX_CH*/
#define MXI							(STA_MAX_MUX_INS)

#ifndef MIXER_VER_3
#define WSIZEOF_MIXE_DATA(ninpc, nout)		(WSIZEOF(T_MixerNchData) + 2*ninpc + nout) /*+ 2insets*8ins + Nout*/
#endif

#define WSIZEOF_COMP_1CH_DATA		(WSIZEOF(T_CompanderMonoData)  +  2/*I/O*/)
#define WSIZEOF_COMP_2CH_DATA		(WSIZEOF(T_CompanderStereoData)+  4/*I/O*/)
#define WSIZEOF_COMP_4CH_DATA		(WSIZEOF(T_CompanderQuadroData)+  8/*I/O*/)
#define WSIZEOF_COMP_6CH_DATA		(WSIZEOF(T_Compander6ChData)   + 12/*I/O*/)
#define WSIZEOF_COMP_COEF(nch)		(WSIZEOF(T_CompanderCoeffs) + (nch)*COMP_DLAY_SIZE)

#define WSIZEOF_LMTR_1CH_DATA		(WSIZEOF(T_Limiter2MonoData)  + 2/*I/O*/)
#define WSIZEOF_LMTR_2CH_DATA		(WSIZEOF(T_Limiter2StereoData)+ 4/*I/O*/)
#define WSIZEOF_LMTR_4CH_DATA		(WSIZEOF(T_Limiter2QuadroData)+ 8/*I/O*/)
#define WSIZEOF_LMTR_6CH_DATA		(WSIZEOF(T_Limiter2SixChData)   +12/*I/O*/)
#define WSIZEOF_LMTR_COEF(nch)		(WSIZEOF(T_Limiter2Coeffs) + (nch)*LMTR_DLAY_SIZE)

#define SIZEOF_SPECMTR_PARAM(nbiq)	(sizeof(tSpectrumMeter) + (nbiq)*sizeof(tBiquad))
#define SIZEOF_SPECMTR_DATA(nbiq)   ((WSIZEOF(T_Biquad_Optim_SP_Chained_Data)+3)*(nbiq))

//#### /!\ the line entries MUST match the order of STA_ModuleType ####


const tModuleInfo g_modulesInfo[STA_MOD_LAST] = {
//	DSP TYPE				I O	 nfilters(biquads) or ninPerCh(mixer)
//							| |  |
  { NO_DSP_TYPE,/*XIN*/	128,128,{0},  0,					0,								0 							},
  { NO_DSP_TYPE,/*XOUT*/128,128,{0},  0,					0,								0 							},
  { MUX_2OUT,			  MXI,2,{0},  sizeof(tMux),			WSIZEOF(T_Mux2outData),			WSIZEOF(T_Mux2outParam)		},
  { MUX_4OUT,			  MXI,4,{0},  sizeof(tMux),			WSIZEOF(T_Mux4outData),			WSIZEOF(T_Mux4outParam)		},
  { MUX_6OUT,			  MXI,6,{0},  sizeof(tMux),			WSIZEOF(T_Mux6outData),			WSIZEOF(T_Mux6outParam)		},
  { MUX_8OUT,			  MXI,8,{0},  sizeof(tMux),			WSIZEOF(T_Mux8outData),			WSIZEOF(T_Mux8outParam)		},
  { MUX_10OUT,			  MXI,10,{0}, sizeof(tMux),			WSIZEOF(T_Mux10outData),		WSIZEOF(T_Mux10outParam)	},
  { CD_DEEMPH, 				2,2,{0},  0,					WSIZEOF(T_CDDeemphasisData)+4,	WSIZEOF(T_CDDeemphasisParam)}, //+4 data for ins[2]+out[2]
  { GAIN_STATIC_2NCH, 		2,2,{0},  sizeof(tGain), 		WSIZEOF(T_GainData)+2*2, 		0/*set in GAIN_init()*/		}, //Parametric sizes: set sizes at init when num of ch is known
  { GAIN_SMOOTH_NCH, 		2,2,{0},  sizeof(tSmoothGain)*2,0,/*set in GAINS_init()*/		0/*set in GAINS_init()*/	}, //Parametric sizes: set sizes at init when num of ch is known
  { GAIN_LINEAR_NCH, 		2,2,{0},  sizeof(tSmoothGain)*2,0,/*set in GAINS_init()*/		0/*set in GAINS_init()*/	}, //Parametric sizes: set sizes at init when num of ch is known
  { LOUD_STATIC_DP,			1,1,{2},  sizeof(tLoudness),	WSIZEOF(T_Loudness_Data),		WSIZEOF(T_Loudness_Coefs)*2	}, //coef x2 (front and back)
  { LOUD_STATIC_DUAL_DP,	2,2,{2},  sizeof(tLoudness),	WSIZEOF(T_LoudnessDual_Data),	WSIZEOF(T_Loudness_Coefs)*2	},
  { TONE_STATIC_DP,			1,1,{3},  sizeof(tTone),		WSIZEOF(T_Tone_Data),	 		WSIZEOF(T_Tone_Coefs)*2		},
  { TONE_STATIC_DUAL_DP,	2,2,{3},  sizeof(tTone),		WSIZEOF(T_ToneDual_Data),	 	WSIZEOF(T_Tone_Coefs)*2		},

  { EQUA_STATIC_1BAND_DP,	2,2,{1},  sizeof(tEqualizer),	WSIZEOF(T_Eq_1band_DP_Data)*2,	WSIZEOF(T_Eq_1band_Coefs)*2},  //all EQ data are x2ch by default, and coef x2 (front and back)
  { EQUA_STATIC_2BANDS_DP,	2,2,{2},  sizeof(tEqualizer),	WSIZEOF(T_Eq_2bands_DP_Data)*2,	WSIZEOF(T_Eq_2bands_Coefs)*2}, //all EQ data are x2ch by default, and coef x2 (front and back)
  { EQUA_STATIC_3BANDS_DP,	2,2,{3},  sizeof(tEqualizer),	WSIZEOF(T_Eq_3bands_DP_Data)*2,	WSIZEOF(T_Eq_3bands_Coefs)*2}, //all EQ data are x2ch by default, and coef x2 (front and back)
  { EQUA_STATIC_4BANDS_DP,	2,2,{4},  sizeof(tEqualizer),	WSIZEOF(T_Eq_4bands_DP_Data)*2,	WSIZEOF(T_Eq_4bands_Coefs)*2},
  { EQUA_STATIC_5BANDS_DP,	2,2,{5},  sizeof(tEqualizer),	WSIZEOF(T_Eq_5bands_DP_Data)*2,	WSIZEOF(T_Eq_5bands_Coefs)*2},
  { EQUA_STATIC_6BANDS_DP,	2,2,{6},  sizeof(tEqualizer),	WSIZEOF(T_Eq_6bands_DP_Data)*2,	WSIZEOF(T_Eq_6bands_Coefs)*2},
  { EQUA_STATIC_7BANDS_DP,	2,2,{7},  sizeof(tEqualizer),	WSIZEOF(T_Eq_7bands_DP_Data)*2,	WSIZEOF(T_Eq_7bands_Coefs)*2},
  { EQUA_STATIC_8BANDS_DP,	2,2,{8},  sizeof(tEqualizer),	WSIZEOF(T_Eq_8bands_DP_Data)*2,	WSIZEOF(T_Eq_8bands_Coefs)*2},
  { EQUA_STATIC_9BANDS_DP,	2,2,{9},  sizeof(tEqualizer),	WSIZEOF(T_Eq_9bands_DP_Data)*2,	WSIZEOF(T_Eq_9bands_Coefs)*2},
  { EQUA_STATIC_10BANDS_DP,	2,2,{10}, sizeof(tEqualizer),	WSIZEOF(T_Eq_10bands_DP_Data)*2,WSIZEOF(T_Eq_10bands_Coefs)*2},
  { EQUA_STATIC_11BANDS_DP,	2,2,{11}, sizeof(tEqualizer),	WSIZEOF(T_Eq_11bands_DP_Data)*2,WSIZEOF(T_Eq_11bands_Coefs)*2},
  { EQUA_STATIC_12BANDS_DP,	2,2,{12}, sizeof(tEqualizer),	WSIZEOF(T_Eq_12bands_DP_Data)*2,WSIZEOF(T_Eq_12bands_Coefs)*2},
  { EQUA_STATIC_13BANDS_DP,	2,2,{13}, sizeof(tEqualizer),	WSIZEOF(T_Eq_13bands_DP_Data)*2,WSIZEOF(T_Eq_13bands_Coefs)*2},
  { EQUA_STATIC_14BANDS_DP,	2,2,{14}, sizeof(tEqualizer),	WSIZEOF(T_Eq_14bands_DP_Data)*2,WSIZEOF(T_Eq_14bands_Coefs)*2},
  { EQUA_STATIC_15BANDS_DP,	2,2,{15}, sizeof(tEqualizer),	WSIZEOF(T_Eq_15bands_DP_Data)*2,WSIZEOF(T_Eq_15bands_Coefs)*2},
  { EQUA_STATIC_16BANDS_DP,	2,2,{16}, sizeof(tEqualizer),	WSIZEOF(T_Eq_16bands_DP_Data)*2,WSIZEOF(T_Eq_16bands_Coefs)*2},

  { EQUA_STATIC_1BAND_SP,	2,2,{1},  sizeof(tEqualizer),	WSIZEOF(T_Eq_1band_SP_Data)*2,	WSIZEOF(T_Eq_1band_SP_Coefs)*2},  //all EQ data are x2ch by default, and coef x2 (front and back)
  { EQUA_STATIC_2BANDS_SP,	2,2,{2},  sizeof(tEqualizer),	WSIZEOF(T_Eq_2bands_SP_Data)*2,	WSIZEOF(T_Eq_2bands_SP_Coefs)*2}, //all EQ data are x2ch by default, and coef x2 (front and back)
  { EQUA_STATIC_3BANDS_SP,	2,2,{3},  sizeof(tEqualizer),	WSIZEOF(T_Eq_3bands_SP_Data)*2,	WSIZEOF(T_Eq_3bands_SP_Coefs)*2}, //all EQ data are x2ch by default, and coef x2 (front and back)
  { EQUA_STATIC_4BANDS_SP,	2,2,{4},  sizeof(tEqualizer),	WSIZEOF(T_Eq_4bands_SP_Data)*2,	WSIZEOF(T_Eq_4bands_SP_Coefs)*2},
  { EQUA_STATIC_5BANDS_SP,	2,2,{5},  sizeof(tEqualizer),	WSIZEOF(T_Eq_5bands_SP_Data)*2,	WSIZEOF(T_Eq_5bands_SP_Coefs)*2},
  { EQUA_STATIC_6BANDS_SP,	2,2,{6},  sizeof(tEqualizer),	WSIZEOF(T_Eq_6bands_SP_Data)*2,	WSIZEOF(T_Eq_6bands_SP_Coefs)*2},
  { EQUA_STATIC_7BANDS_SP,	2,2,{7},  sizeof(tEqualizer),	WSIZEOF(T_Eq_7bands_SP_Data)*2,	WSIZEOF(T_Eq_7bands_SP_Coefs)*2},
  { EQUA_STATIC_8BANDS_SP,	2,2,{8},  sizeof(tEqualizer),	WSIZEOF(T_Eq_8bands_SP_Data)*2,	WSIZEOF(T_Eq_8bands_SP_Coefs)*2},
  { EQUA_STATIC_9BANDS_SP,	2,2,{9},  sizeof(tEqualizer),	WSIZEOF(T_Eq_9bands_SP_Data)*2,	WSIZEOF(T_Eq_9bands_SP_Coefs)*2},
  { EQUA_STATIC_10BANDS_SP,	2,2,{10}, sizeof(tEqualizer),	WSIZEOF(T_Eq_10bands_SP_Data)*2,WSIZEOF(T_Eq_10bands_SP_Coefs)*2},
  { EQUA_STATIC_11BANDS_SP,	2,2,{11}, sizeof(tEqualizer),	WSIZEOF(T_Eq_11bands_SP_Data)*2,WSIZEOF(T_Eq_11bands_SP_Coefs)*2},
  { EQUA_STATIC_12BANDS_SP,	2,2,{12}, sizeof(tEqualizer),	WSIZEOF(T_Eq_12bands_SP_Data)*2,WSIZEOF(T_Eq_12bands_SP_Coefs)*2},
  { EQUA_STATIC_13BANDS_SP,	2,2,{13}, sizeof(tEqualizer),	WSIZEOF(T_Eq_13bands_SP_Data)*2,WSIZEOF(T_Eq_13bands_SP_Coefs)*2},
  { EQUA_STATIC_14BANDS_SP,	2,2,{14}, sizeof(tEqualizer),	WSIZEOF(T_Eq_14bands_SP_Data)*2,WSIZEOF(T_Eq_14bands_SP_Coefs)*2},
  { EQUA_STATIC_15BANDS_SP,	2,2,{15}, sizeof(tEqualizer),	WSIZEOF(T_Eq_15bands_SP_Data)*2,WSIZEOF(T_Eq_15bands_SP_Coefs)*2},
  { EQUA_STATIC_16BANDS_SP,	2,2,{16}, sizeof(tEqualizer),	WSIZEOF(T_Eq_16bands_SP_Data)*2,WSIZEOF(T_Eq_16bands_SP_Coefs)*2},

#ifdef MIXER_VER_3
  { MIXE_2INS_NCH,			4,2,{2},  sizeof(tMixer3)*MCH,	WSIZEOF(T_Mixer3_NchData)+4,	WSIZEOF(T_Mixer3_2inNchParams)},
  { MIXE_3INS_NCH,			6,2,{3},  sizeof(tMixer3)*MCH,	WSIZEOF(T_Mixer3_NchData)+6,	WSIZEOF(T_Mixer3_3inNchParams)},
  { MIXE_4INS_NCH,			8,2,{4},  sizeof(tMixer3)*MCH,	WSIZEOF(T_Mixer3_NchData)+8,	WSIZEOF(T_Mixer3_4inNchParams)},
  { MIXE_5INS_NCH,		   10,2,{5},  sizeof(tMixer3)*MCH,	WSIZEOF(T_Mixer3_NchData)+10,	WSIZEOF(T_Mixer3_5inNchParams)},
  { MIXE_6INS_NCH,		   12,2,{6},  sizeof(tMixer3)*MCH,	WSIZEOF(T_Mixer3_NchData)+12,	WSIZEOF(T_Mixer3_6inNchParams)},
  { MIXE_7INS_NCH,		   14,2,{7},  sizeof(tMixer3)*MCH,	WSIZEOF(T_Mixer3_NchData)+14,	WSIZEOF(T_Mixer3_7inNchParams)},
  { MIXE_8INS_NCH,		   16,2,{8},  sizeof(tMixer3)*MCH,	WSIZEOF(T_Mixer3_NchData)+16,	WSIZEOF(T_Mixer3_8inNchParams)},
  { MIXE_9INS_NCH,		   18,2,{9},  sizeof(tMixer3)*MCH,	WSIZEOF(T_Mixer3_NchData)+18,	WSIZEOF(T_Mixer3_9inNchParams)},
#else
  { MIXE_2INS_NCH,			4,2,{2},  sizeof(tMixer)*MCH,	WSIZEOF_MIXE_DATA(2, MCH),		WSIZEOF(T_Mixer2insNchParam)},
  { MIXE_3INS_NCH,			6,2,{3},  sizeof(tMixer)*MCH,	WSIZEOF_MIXE_DATA(3, MCH),		WSIZEOF(T_Mixer3insNchParam)},
  { MIXE_4INS_NCH,			8,2,{4},  sizeof(tMixer)*MCH,	WSIZEOF_MIXE_DATA(4, MCH),		WSIZEOF(T_Mixer4insNchParam)},
  { MIXE_5INS_NCH,		   10,2,{5},  sizeof(tMixer)*MCH,	WSIZEOF_MIXE_DATA(5, MCH),		WSIZEOF(T_Mixer5insNchParam)},
  { MIXE_6INS_NCH,		   12,2,{6},  sizeof(tMixer)*MCH,	WSIZEOF_MIXE_DATA(6, MCH),		WSIZEOF(T_Mixer6insNchParam)},
  { MIXE_7INS_NCH,		   14,2,{7},  sizeof(tMixer)*MCH,	WSIZEOF_MIXE_DATA(7, MCH),		WSIZEOF(T_Mixer7insNchParam)},
  { MIXE_8INS_NCH,		   16,2,{8},  sizeof(tMixer)*MCH,	WSIZEOF_MIXE_DATA(8, MCH),		WSIZEOF(T_Mixer8insNchParam)},
  { MIXE_9INS_NCH,		   18,2,{9},  sizeof(tMixer)*MCH,	WSIZEOF_MIXE_DATA(9, MCH),		WSIZEOF(T_Mixer9insNchParam)},
#endif

  { NO_DSP_TYPE/*BALANCE*/,	3,3,{0},  sizeof(tBalance),	 	0,								0							},
  { NO_DSP_TYPE/*FADER*/,	3,3,{0},  sizeof(tFader),		0,								0							},
  { COMP_MONO,				1,1,{0},  sizeof(tCompander),	WSIZEOF_COMP_1CH_DATA,			WSIZEOF_COMP_COEF(1) 		},
  { COMP_STEREO,			2,2,{0},  sizeof(tCompander)*2,	WSIZEOF_COMP_2CH_DATA,			WSIZEOF_COMP_COEF(2) 		},
  { COMP_QUADRO,			4,4,{0},  sizeof(tCompander)*4,	WSIZEOF_COMP_4CH_DATA,			WSIZEOF_COMP_COEF(4) 		},
  { COMP_6CH,				6,6,{0},  sizeof(tCompander)*6,	WSIZEOF_COMP_6CH_DATA,			WSIZEOF_COMP_COEF(6) 		},
  { LMTR_MONO,				1,3,{0},  sizeof(tLimiter),		WSIZEOF_LMTR_1CH_DATA,			WSIZEOF_LMTR_COEF(1) 		},
  { LMTR_STEREO,			2,4,{0},  sizeof(tLimiter)*2,	WSIZEOF_LMTR_2CH_DATA,			WSIZEOF_LMTR_COEF(2) 		},
  { LMTR_QUADRO,			4,6,{0},  sizeof(tLimiter)*4,	WSIZEOF_LMTR_4CH_DATA,			WSIZEOF_LMTR_COEF(4) 		},
  { LMTR_6CH,				6,8,{0},  sizeof(tLimiter)*6,	WSIZEOF_LMTR_6CH_DATA,			WSIZEOF_LMTR_COEF(6) 		},
  { LMTR_CLIP_NCH,          3,2,{0},  sizeof(tClipLimiter),	0,/*set in CLIPLMTR_init()*/    0/*set in CLIPLMTR_init()*/ }, //Parametric sizes: set sizes at init when num of ch is known
  { DLAY_Y_2CH,			    2,2,{0},  sizeof(tDelay),		WSIZEOF(T_DelayData)*2+2*2,		WSIZEOF(T_DelayYParam)*2	}, //sizes adjusted in DLAY_AdjustDspMemSize()
  { DLAY_Y_4CH,			    4,4,{0},  sizeof(tDelay),		WSIZEOF(T_DelayData)*4+2*4,		WSIZEOF(T_DelayYParam)*4	}, //sizes adjusted in DLAY_AdjustDspMemSize()
  { DLAY_Y_6CH,			    6,6,{0},  sizeof(tDelay),		WSIZEOF(T_DelayData)*6+2*6,		WSIZEOF(T_DelayYParam)*6	}, //sizes adjusted in DLAY_AdjustDspMemSize()
  { DLAY_X_2CH,			    2,2,{0},  sizeof(tDelay),		WSIZEOF(T_DelayData)*2+3*2,		WSIZEOF(T_DelayXParam)*2 	}, //sizes adjusted in DLAY_AdjustDspMemSize()
  { DLAY_X_4CH,			    4,4,{0},  sizeof(tDelay),		WSIZEOF(T_DelayData)*4+3*4,		WSIZEOF(T_DelayXParam)*4 	}, //sizes adjusted in DLAY_AdjustDspMemSize()
  { DLAY_X_6CH,			    6,6,{0},  sizeof(tDelay),		WSIZEOF(T_DelayData)*6+3*6,		WSIZEOF(T_DelayXParam)*6 	}, //sizes adjusted in DLAY_AdjustDspMemSize()
  { SINE_2CH,			    0,2,{0},  sizeof(tSinewaveGen),	WSIZEOF(T_SinewaveData)+2,		WSIZEOF(T_SinewaveCoeffs) 	}, //+2 for dout[2]
  { PCMCHIME_12BIT_Y_2CH,	0,2,{0},  sizeof(tPCMChime),	WSIZEOF(T_PCMChime_12bit_Data), WSIZEOF(T_PCMChime_12bit_Params)},//sizes adjusted in PCM_AdjustDspMemSize()
  { PCMCHIME_12BIT_X_2CH,	0,2,{0},  sizeof(tPCMChime),	WSIZEOF(T_PCMChime_12bit_Data), WSIZEOF(T_PCMChime_12bit_Params)},//sizes adjusted in PCM_AdjustDspMemSize()
  { POLYCHIME,				0,1,{0},  sizeof(tChimeGen),	WSIZEOF(T_PolyChime_Data), 		WSIZEOF(T_PolyChime_Params) }, //sizes adjusted in CHIME_AdjustDspMemSize()
  { BITSHIFTER_NCH, 		2,2,{0},  sizeof(tBitShift),	WSIZEOF(T_BitShiftData)+2*2,	0/*set in BITSHIFT_Init()*/	}, //Parametric sizes: set sizes at init when num of ch is known
  { PEAKDETECT_NCH, 		2,0,{0},  sizeof(tPeakDetect), 	WSIZEOF(T_PeakDetectData)+2, 	0/*set in PEAKDETECT_Init()*/},//Parametric sizes: set sizes at init when num of ch is known
  { SPECMTR_NBANDS_1BIQ,	1,0,{9},  SIZEOF_SPECMTR_PARAM(9*1),SIZEOF_SPECMTR_DATA(9), 	0/*set in SPECMTR_Init()*/	}, //Parametric sizes
  { DCO_1CH,			    2,3,{0},  sizeof(tDCO),			WSIZEOF(T_Dco1chData),			WSIZEOF(T_Dco1chParams) 	}, //in=in0,hwZCF,  out=swZCF,score
  { DCO_4CH_MONOZCF,	    5,3,{0},  sizeof(tDCO)*4,		WSIZEOF(T_Dco1chData)*4,		WSIZEOF(T_Dco1chParams)*4 	}, //in=ins,hwZCF,  out=swZCF,score
};

static void SINE_Init( tModule* mod );

#ifndef EARLY_AUDIO
static void MUX_Init( tModule* mod );
static void DEEM_Init( tModule* mod );
static void BITSHIFT_Init( tModule* mod );
static void PEAKDETECT_Init( tModule* mod );
static void LOUD_Init( tModule* mod );
static void TONE_Init( tModule* mod );
static void EQUA_Init( tModule* mod );
static void COMP_Init( tModule* mod );
static void LMTR_Init( tModule* mod );
static void DLAY_Init( tModule* mod );
static void CLIP_Init( tModule* mod );

static void EQUA_InitBands(tModule* mod);
static void MOD_SwapCoefBuffers(tModule* mod);
#endif /* !EARLY_AUDIO */


//===========================================================================
// MODULE (base of all modules)
//===========================================================================

void MOD_SetModuleInfo(tModule *mod, const tModuleInfo* info)
{
	if (!mod || !info) {goto _ret;}
	mod->m_dspType 		= (u32)(int)info->dspType;
	mod->m_nin			= info->nin;
	mod->m_nout			= info->nout;
	mod->m_nfilters 	= info->nfilters;
	mod->m_ninpc		= info->ninpc;
	mod->m_sizeofParams = info->sizeofParams;
	mod->m_wsizeofData	= info->wsizeofData;
	mod->m_wsizeofCoefs = info->wsizeofCoefs;

_ret: return;
}


void MOD_Init( tModule* mod, STA_ModuleType type, u32 id, tDSP* dsp, const STA_UserModuleInfo *info, const char* name, u32 tags)
{
	if (!mod) {goto _ret;}

	//common internal init
	mod->m_type 	= type;
	mod->m_id   	= id;
	mod->m_dsp	 	= dsp;
	mod->m_refCode 	= STA_MODULE_REF_CODE;
	mod->m_dirtyDspCoefs = 0xFFFFFFFF;
	mod->m_tags		= tags;

	//name
	strncpy(mod->m_name, (name)?name:"NONAME!", STA_MAX_MODULE_NAME_SIZE-1);

	//USER init
	if (info)
	{
		void* sub = mod->m_sub; //bak

		memcpy(mod, info, sizeof(STA_UserModuleInfo));

		//return the allocated mem for user host param
		mod->m_type 		= type; //(was overwritten by the previous memcpy)
		mod->m_sub 			= sub;  //= m_hostParams
		mod->m_dspData		= NULL;
		mod->m_dspCoefs		= NULL;
		mod->m_xdata	 	= 0; //invalid offset
		mod->m_ycoef	 	= 0; //invalid offset

		//user init
		if (mod->InitModule) {
			mod->InitModule(mod);}
	}
	//CORE modules init
	else
	{
		MOD_SetModuleInfo(mod, &g_modulesInfo[type]);
	}

	//init sub-params specific to module type

	//EQ
	if (STA_EQUA_STATIC_1BAND_DP <= type && type <= STA_EQUA_STATIC_16BANDS_SP)
	{
#ifndef EARLY_AUDIO
		EQUA_Init(mod);
#endif /* !EARLY_AUDIO */
	}
	//Others
	else { switch (type) {
	case STA_GAIN_STATIC_NCH:		GAIN_Init(mod); break;
	case STA_SINE_2CH:				SINE_Init(mod); break;
#ifndef EARLY_AUDIO
	case STA_MUX_2OUT:
	case STA_MUX_4OUT:
	case STA_MUX_6OUT:
	case STA_MUX_8OUT:
	case STA_MUX_10OUT:				MUX_Init(mod);  break;
	case STA_CD_DEEMPH:				DEEM_Init(mod); break;
	case STA_GAIN_LINEAR_NCH:
	case STA_GAIN_SMOOTH_NCH:		GAINS_Init(mod); break;
	case STA_BITSHIFTER_NCH:		BITSHIFT_Init(mod); break;
	case STA_PEAKDETECT_NCH:		PEAKDETECT_Init(mod); break;
	case STA_LOUD_STATIC_DP:
	case STA_LOUD_STATIC_STEREO_DP:	LOUD_Init(mod); break;
	case STA_TONE_STATIC_DP:
	case STA_TONE_STATIC_STEREO_DP:	TONE_Init(mod); break;
	case STA_MIXE_2INS_NCH:
	case STA_MIXE_3INS_NCH:
	case STA_MIXE_4INS_NCH:
	case STA_MIXE_5INS_NCH:
	case STA_MIXE_6INS_NCH:
	case STA_MIXE_7INS_NCH:
	case STA_MIXE_8INS_NCH:
	case STA_MIXE_9INS_NCH:			MIXE_Init(mod); break;
	case STA_MIXE_BALANCE:
	case STA_MIXE_FADER:			BALN_Init(mod); break;
	case STA_COMP_1CH:
	case STA_COMP_2CH:
	case STA_COMP_4CH:
	case STA_COMP_6CH:				COMP_Init(mod); break;
	case STA_LMTR_1CH:
	case STA_LMTR_2CH:
	case STA_LMTR_4CH:
	case STA_LMTR_6CH:				LMTR_Init(mod); break;
	case STA_CLIPLMTR_NCH:			CLIP_Init(mod); break;
	case STA_DLAY_Y_2CH:
	case STA_DLAY_Y_4CH:
	case STA_DLAY_Y_6CH:
	case STA_DLAY_X_2CH:
	case STA_DLAY_X_4CH:
	case STA_DLAY_X_6CH:			DLAY_Init(mod); break;
	case STA_DCO_1CH:
	case STA_DCO_4CH_MONOZCF:		DCO_Init(mod); break;
	case STA_PCMCHIME_12BIT_Y_2CH:
	case STA_PCMCHIME_12BIT_X_2CH:	PCM_Init(mod); break;
	case STA_CHIMEGEN:				CHIME_Init(mod); break;
	case STA_SPECMTR_NBANDS_1BIQ:	SPECMTR_Init(mod); break;
#endif /* !EARLY_AUDIO */
	default:
		sta_assert(1);
	}}

_ret: return;
}
//----------------------------------------------------------------------------

//returns the dsp address of the given 'in' data
void* MOD_GetDspInAddr(const tModule* mod, u32 in)
{
	u32 armAddr = 0;
	u32 dspAddr = 0;

	if (!mod) {goto _ret;}{

	volatile void* const m_data = mod->m_dspData;


	sta_assert(m_data);
	sta_assert(in < mod->m_nin);

	//note: some modules returns directly the DSP address,
	//others are converted from ARM to DSP address.
	switch (mod->m_type) {
	case STA_XOUT:
		//as (in < STA_XOUT_SIZE) armAddr = &((s32*) m_data)[in]; break;
		dspAddr = (u32)(mod->m_xdata + in); break;

	case STA_MUX_2OUT:
	case STA_MUX_4OUT:
	case STA_MUX_6OUT:
	case STA_MUX_8OUT:
	case STA_MUX_10OUT:
		armAddr = (u32) &((const volatile T_MuxData*) m_data)->ins[in]; break;

	case STA_CD_DEEMPH:
		if (in == 0)    {dspAddr = (u32) ((const volatile T_CDDeemphasisData*) m_data)->p_inL; break;}
		sta_assert(in == 1); dspAddr = (u32) ((const volatile T_CDDeemphasisData*) m_data)->p_inR; break;

	case STA_GAIN_STATIC_NCH:
	case STA_GAIN_SMOOTH_NCH:
	case STA_GAIN_LINEAR_NCH:
	case STA_BITSHIFTER_NCH:
		dspAddr = (u32) ((tDspPtr)((const volatile T_GainData*)m_data)->ins + in); break;

	case STA_PEAKDETECT_NCH:
		dspAddr = (u32) ((tDspPtr)((const volatile T_PeakDetectData*)m_data)->ins + in); break;

	case STA_LOUD_STATIC_DP:		 sta_assert(in == 0); armAddr = (u32) &((const volatile T_Loudness_Data*)     m_data)->in.xn_high; break;
	case STA_LOUD_STATIC_STEREO_DP:	 sta_assert(in < 2);  armAddr = (u32) &((const volatile T_LoudnessDual_Data*) m_data)->ch[in].in.xn_high; break;
	case STA_TONE_STATIC_DP:		 sta_assert(in == 0); armAddr = (u32) &((const volatile T_Tone_Data*)         m_data)->in.xn_high; break;
	case STA_TONE_STATIC_STEREO_DP:	 sta_assert(in < 2);  armAddr = (u32) &((const volatile T_ToneDual_Data*)     m_data)->ch[in].in.xn_high; break;

	case STA_EQUA_STATIC_1BAND_DP:	 armAddr = (u32) &(((const volatile T_Eq_1band_DP_Data*)   m_data)[in]).in.xn_high; break;
	case STA_EQUA_STATIC_2BANDS_DP:	 armAddr = (u32) &(((const volatile T_Eq_2bands_DP_Data*)  m_data)[in]).in.xn_high; break;
	case STA_EQUA_STATIC_3BANDS_DP:	 armAddr = (u32) &(((const volatile T_Eq_3bands_DP_Data*)  m_data)[in]).in.xn_high; break;
	case STA_EQUA_STATIC_4BANDS_DP:	 armAddr = (u32) &(((const volatile T_Eq_4bands_DP_Data*)  m_data)[in]).in.xn_high; break;
	case STA_EQUA_STATIC_5BANDS_DP:	 armAddr = (u32) &(((const volatile T_Eq_5bands_DP_Data*)  m_data)[in]).in.xn_high; break;
	case STA_EQUA_STATIC_6BANDS_DP:	 armAddr = (u32) &(((const volatile T_Eq_6bands_DP_Data*)  m_data)[in]).in.xn_high; break;
	case STA_EQUA_STATIC_7BANDS_DP:	 armAddr = (u32) &(((const volatile T_Eq_7bands_DP_Data*)  m_data)[in]).in.xn_high; break;
	case STA_EQUA_STATIC_8BANDS_DP:	 armAddr = (u32) &(((const volatile T_Eq_8bands_DP_Data*)  m_data)[in]).in.xn_high; break;
	case STA_EQUA_STATIC_9BANDS_DP:	 armAddr = (u32) &(((const volatile T_Eq_9bands_DP_Data*)  m_data)[in]).in.xn_high; break;
	case STA_EQUA_STATIC_10BANDS_DP: armAddr = (u32) &(((const volatile T_Eq_10bands_DP_Data*) m_data)[in]).in.xn_high; break;
	case STA_EQUA_STATIC_11BANDS_DP: armAddr = (u32) &(((const volatile T_Eq_11bands_DP_Data*) m_data)[in]).in.xn_high; break;
	case STA_EQUA_STATIC_12BANDS_DP: armAddr = (u32) &(((const volatile T_Eq_12bands_DP_Data*) m_data)[in]).in.xn_high; break;
	case STA_EQUA_STATIC_13BANDS_DP: armAddr = (u32) &(((const volatile T_Eq_13bands_DP_Data*) m_data)[in]).in.xn_high; break;
	case STA_EQUA_STATIC_14BANDS_DP: armAddr = (u32) &(((const volatile T_Eq_14bands_DP_Data*) m_data)[in]).in.xn_high; break;
	case STA_EQUA_STATIC_15BANDS_DP: armAddr = (u32) &(((const volatile T_Eq_15bands_DP_Data*) m_data)[in]).in.xn_high; break;
	case STA_EQUA_STATIC_16BANDS_DP: armAddr = (u32) &(((const volatile T_Eq_16bands_DP_Data*) m_data)[in]).in.xn_high; break;

	case STA_EQUA_STATIC_1BAND_SP:   armAddr = (u32) &(((const volatile T_Eq_1band_SP_Data*)   m_data)[in]).xn; break;
	case STA_EQUA_STATIC_2BANDS_SP:  armAddr = (u32) &(((const volatile T_Eq_2bands_SP_Data*)  m_data)[in]).xn; break;
	case STA_EQUA_STATIC_3BANDS_SP:  armAddr = (u32) &(((const volatile T_Eq_3bands_SP_Data*)  m_data)[in]).xn; break;
	case STA_EQUA_STATIC_4BANDS_SP:	 armAddr = (u32) &(((const volatile T_Eq_4bands_SP_Data*)  m_data)[in]).xn; break;
	case STA_EQUA_STATIC_5BANDS_SP:	 armAddr = (u32) &(((const volatile T_Eq_5bands_SP_Data*)  m_data)[in]).xn; break;
	case STA_EQUA_STATIC_6BANDS_SP:	 armAddr = (u32) &(((const volatile T_Eq_6bands_SP_Data*)  m_data)[in]).xn; break;
	case STA_EQUA_STATIC_7BANDS_SP:	 armAddr = (u32) &(((const volatile T_Eq_7bands_SP_Data*)  m_data)[in]).xn; break;
	case STA_EQUA_STATIC_8BANDS_SP:	 armAddr = (u32) &(((const volatile T_Eq_8bands_SP_Data*)  m_data)[in]).xn; break;
	case STA_EQUA_STATIC_9BANDS_SP:	 armAddr = (u32) &(((const volatile T_Eq_9bands_SP_Data*)  m_data)[in]).xn; break;
	case STA_EQUA_STATIC_10BANDS_SP: armAddr = (u32) &(((const volatile T_Eq_10bands_SP_Data*) m_data)[in]).xn; break;
	case STA_EQUA_STATIC_11BANDS_SP: armAddr = (u32) &(((const volatile T_Eq_11bands_SP_Data*) m_data)[in]).xn; break;
	case STA_EQUA_STATIC_12BANDS_SP: armAddr = (u32) &(((const volatile T_Eq_12bands_SP_Data*) m_data)[in]).xn; break;
	case STA_EQUA_STATIC_13BANDS_SP: armAddr = (u32) &(((const volatile T_Eq_13bands_SP_Data*) m_data)[in]).xn; break;
	case STA_EQUA_STATIC_14BANDS_SP: armAddr = (u32) &(((const volatile T_Eq_14bands_SP_Data*) m_data)[in]).xn; break;
	case STA_EQUA_STATIC_15BANDS_SP: armAddr = (u32) &(((const volatile T_Eq_15bands_SP_Data*) m_data)[in]).xn; break;
	case STA_EQUA_STATIC_16BANDS_SP: armAddr = (u32) &(((const volatile T_Eq_16bands_SP_Data*) m_data)[in]).xn; break;

	case STA_SPECMTR_NBANDS_1BIQ:    armAddr = (u32) &((const volatile T_SpeMeter_1biq_Data*) m_data)->xn; break;

	case STA_MIXE_2INS_NCH:
	case STA_MIXE_3INS_NCH:
	case STA_MIXE_4INS_NCH:
	case STA_MIXE_5INS_NCH:
	case STA_MIXE_6INS_NCH:
	case STA_MIXE_7INS_NCH:
	case STA_MIXE_8INS_NCH:
	case STA_MIXE_9INS_NCH:
		//note: all out channels are using the same 2 contiguous insets: insetL[0..nin-1] | insetR[nin..2nin-1]
#ifdef MIXER_VER_3
		dspAddr = (u32) ((tDspPtr)((const volatile T_Mixer3_NchData*)m_data)->chData[0].inset + in); break;
#else
		dspAddr = (u32) ((tDspPtr)*((const volatile T_MixerNchData*)m_data)->ins + in); break;
#endif

	case STA_COMP_1CH:		sta_assert(in == 0); dspAddr = (u32) ((const volatile T_CompanderMonoData*)   m_data)->din; break;
	case STA_COMP_2CH:		sta_assert(in < 2);  dspAddr = (u32) ((const volatile T_CompanderStereoData*) m_data)->din[in]; break;
	case STA_COMP_4CH:		sta_assert(in < 4);  dspAddr = (u32) ((const volatile T_CompanderQuadroData*) m_data)->din[in]; break;
	case STA_COMP_6CH:		sta_assert(in < 6);  dspAddr = (u32) ((const volatile T_Compander6ChData*)    m_data)->din[in]; break;
	case STA_LMTR_1CH:		sta_assert(in == 0); dspAddr = (u32) ((const volatile T_Limiter2MonoData*)    m_data)->din; break;
	case STA_LMTR_2CH:		sta_assert(in < 2);  dspAddr = (u32) ((const volatile T_Limiter2StereoData*)  m_data)->din[in]; break;
	case STA_LMTR_4CH:		sta_assert(in < 4);  dspAddr = (u32) ((const volatile T_Limiter2QuadroData*)  m_data)->din[in]; break;
	case STA_LMTR_6CH:		sta_assert(in < 6);  dspAddr = (u32) ((const volatile T_Limiter2SixChData*)   m_data)->din[in]; break;

	case STA_DLAY_Y_2CH:
	case STA_DLAY_Y_4CH:
	case STA_DLAY_Y_6CH:
	case STA_DLAY_X_2CH:
	case STA_DLAY_X_4CH:
	case STA_DLAY_X_6CH:
		dspAddr = (u32) ((const volatile T_DelayData*)m_data)[in].p_in; break;

	case STA_CLIPLMTR_NCH:
		dspAddr = (u32) ((tDspPtr)((const volatile T_ClipLimiterData*)m_data)->din + in); break;

	case STA_DCO_1CH:
	case STA_DCO_4CH_MONOZCF:
		if (in == mod->m_nin-1) {armAddr = (u32) &((const volatile T_Dco1chData*) m_data)[0].DCO_REG;} //store in ch0
		else					{armAddr = (u32) &((const volatile T_Dco1chData*) m_data)[in].indata;}
		break;

	default:
		if (mod->GetDspInAddr){
			dspAddr = (u32) mod->GetDspInAddr(mod, in);
		} else {sta_assert(0); /*should not happen*/}
		break;
	}


	if (armAddr) {
		dspAddr = (u32)ARM2DSP(mod->m_dsp->m_xram, armAddr);
	}

	}
_ret: return (void*)dspAddr;
}
//----------------------------------------------------------------------------

//returns the dsp address of the given 'out' data
void* MOD_GetDspOutAddr(const tModule* mod, u32 out)
{
	u32 armAddr = 0;
	u32 dspAddr = 0;

	if (!mod) {goto _ret;}{

	const volatile void* m_data = mod->m_dspData;
	const volatile void* m_coef = mod->m_dspCoefs;


	sta_assert(m_data);
	sta_assert(out < mod->m_nout);

	//note: some modules returns directly the DSP address,
	//others are converted from ARM to DSP address.
	switch (mod->m_type)
	{
	case STA_XIN:
//		as (out < STA_XIN_SIZE) armAddr = &((s32*) m_data)[out]; break;
		dspAddr = (u32)(mod->m_xdata + out); break;

	case STA_MUX_2OUT:
	case STA_MUX_4OUT:
	case STA_MUX_6OUT:
	case STA_MUX_8OUT:
	case STA_MUX_10OUT:
		armAddr = (u32) &((const volatile T_MuxData*) m_data)->out[out]; break;

	case STA_CD_DEEMPH:
		if (out == 0)     {dspAddr = (u32)((const volatile T_CDDeemphasisData*) m_data)->p_outL; break;}
		sta_assert(out == 1); {dspAddr = (u32)((const volatile T_CDDeemphasisData*) m_data)->p_outR; break;}

	case STA_GAIN_STATIC_NCH:
	case STA_GAIN_SMOOTH_NCH:
	case STA_GAIN_LINEAR_NCH:
	case STA_BITSHIFTER_NCH:
			dspAddr = ( (tDspPtr)((const volatile T_GainData*) m_data)->out + out); break;

	case STA_LOUD_STATIC_DP:		 sta_assert(out == 0); armAddr = (u32) &((const volatile T_Loudness_Data*)     m_data)->data[1].yn_high; break;
	case STA_LOUD_STATIC_STEREO_DP:	 sta_assert(out < 2);  armAddr = (u32) &((const volatile T_LoudnessDual_Data*) m_data)->ch[out].data[1].yn_high; break;
	case STA_TONE_STATIC_DP:		 sta_assert(out == 0); armAddr = (u32) &((const volatile T_Tone_Data*)         m_data)->data[2].yn_high; break;
	case STA_TONE_STATIC_STEREO_DP:	 sta_assert(out < 2);  armAddr = (u32) &((const volatile T_ToneDual_Data*)     m_data)->ch[out].data[2].yn_high; break;

	case STA_EQUA_STATIC_1BAND_DP:   armAddr = (u32) &((const volatile T_Eq_1band_DP_Data*)   m_data)[out].data[mod->m_nfilters-1].yn_high; break;
	case STA_EQUA_STATIC_2BANDS_DP:  armAddr = (u32) &((const volatile T_Eq_2bands_DP_Data*)  m_data)[out].data[mod->m_nfilters-1].yn_high; break;
	case STA_EQUA_STATIC_3BANDS_DP:  armAddr = (u32) &((const volatile T_Eq_3bands_DP_Data*)  m_data)[out].data[mod->m_nfilters-1].yn_high; break;
	case STA_EQUA_STATIC_4BANDS_DP:  armAddr = (u32) &((const volatile T_Eq_4bands_DP_Data*)  m_data)[out].data[mod->m_nfilters-1].yn_high; break;
	case STA_EQUA_STATIC_5BANDS_DP:  armAddr = (u32) &((const volatile T_Eq_5bands_DP_Data*)  m_data)[out].data[mod->m_nfilters-1].yn_high; break;
	case STA_EQUA_STATIC_6BANDS_DP:  armAddr = (u32) &((const volatile T_Eq_6bands_DP_Data*)  m_data)[out].data[mod->m_nfilters-1].yn_high; break;
	case STA_EQUA_STATIC_7BANDS_DP:  armAddr = (u32) &((const volatile T_Eq_7bands_DP_Data*)  m_data)[out].data[mod->m_nfilters-1].yn_high; break;
	case STA_EQUA_STATIC_8BANDS_DP:  armAddr = (u32) &((const volatile T_Eq_8bands_DP_Data*)  m_data)[out].data[mod->m_nfilters-1].yn_high; break;
	case STA_EQUA_STATIC_9BANDS_DP:  armAddr = (u32) &((const volatile T_Eq_9bands_DP_Data*)  m_data)[out].data[mod->m_nfilters-1].yn_high; break;
	case STA_EQUA_STATIC_10BANDS_DP: armAddr = (u32) &((const volatile T_Eq_10bands_DP_Data*) m_data)[out].data[mod->m_nfilters-1].yn_high; break;
	case STA_EQUA_STATIC_11BANDS_DP: armAddr = (u32) &((const volatile T_Eq_11bands_DP_Data*) m_data)[out].data[mod->m_nfilters-1].yn_high; break;
	case STA_EQUA_STATIC_12BANDS_DP: armAddr = (u32) &((const volatile T_Eq_12bands_DP_Data*) m_data)[out].data[mod->m_nfilters-1].yn_high; break;
	case STA_EQUA_STATIC_13BANDS_DP: armAddr = (u32) &((const volatile T_Eq_13bands_DP_Data*) m_data)[out].data[mod->m_nfilters-1].yn_high; break;
	case STA_EQUA_STATIC_14BANDS_DP: armAddr = (u32) &((const volatile T_Eq_14bands_DP_Data*) m_data)[out].data[mod->m_nfilters-1].yn_high; break;
	case STA_EQUA_STATIC_15BANDS_DP: armAddr = (u32) &((const volatile T_Eq_15bands_DP_Data*) m_data)[out].data[mod->m_nfilters-1].yn_high; break;
	case STA_EQUA_STATIC_16BANDS_DP: armAddr = (u32) &((const volatile T_Eq_16bands_DP_Data*) m_data)[out].data[mod->m_nfilters-1].yn_high; break;

	case STA_EQUA_STATIC_1BAND_SP:   armAddr = (u32) &((const volatile T_Eq_1band_SP_Data*)   m_data)[out].data[mod->m_nfilters-1].yn_1; break;
	case STA_EQUA_STATIC_2BANDS_SP:  armAddr = (u32) &((const volatile T_Eq_2bands_SP_Data*)  m_data)[out].data[mod->m_nfilters-1].yn_1; break;
	case STA_EQUA_STATIC_3BANDS_SP:  armAddr = (u32) &((const volatile T_Eq_3bands_SP_Data*)  m_data)[out].data[mod->m_nfilters-1].yn_1; break;
	case STA_EQUA_STATIC_4BANDS_SP:  armAddr = (u32) &((const volatile T_Eq_4bands_SP_Data*)  m_data)[out].data[mod->m_nfilters-1].yn_1; break;
	case STA_EQUA_STATIC_5BANDS_SP:  armAddr = (u32) &((const volatile T_Eq_5bands_SP_Data*)  m_data)[out].data[mod->m_nfilters-1].yn_1; break;
	case STA_EQUA_STATIC_6BANDS_SP:  armAddr = (u32) &((const volatile T_Eq_6bands_SP_Data*)  m_data)[out].data[mod->m_nfilters-1].yn_1; break;
	case STA_EQUA_STATIC_7BANDS_SP:  armAddr = (u32) &((const volatile T_Eq_7bands_SP_Data*)  m_data)[out].data[mod->m_nfilters-1].yn_1; break;
	case STA_EQUA_STATIC_8BANDS_SP:  armAddr = (u32) &((const volatile T_Eq_8bands_SP_Data*)  m_data)[out].data[mod->m_nfilters-1].yn_1; break;
	case STA_EQUA_STATIC_9BANDS_SP:  armAddr = (u32) &((const volatile T_Eq_9bands_SP_Data*)  m_data)[out].data[mod->m_nfilters-1].yn_1; break;
	case STA_EQUA_STATIC_10BANDS_SP: armAddr = (u32) &((const volatile T_Eq_10bands_SP_Data*) m_data)[out].data[mod->m_nfilters-1].yn_1; break;
	case STA_EQUA_STATIC_11BANDS_SP: armAddr = (u32) &((const volatile T_Eq_11bands_SP_Data*) m_data)[out].data[mod->m_nfilters-1].yn_1; break;
	case STA_EQUA_STATIC_12BANDS_SP: armAddr = (u32) &((const volatile T_Eq_12bands_SP_Data*) m_data)[out].data[mod->m_nfilters-1].yn_1; break;
	case STA_EQUA_STATIC_13BANDS_SP: armAddr = (u32) &((const volatile T_Eq_13bands_SP_Data*) m_data)[out].data[mod->m_nfilters-1].yn_1; break;
	case STA_EQUA_STATIC_14BANDS_SP: armAddr = (u32) &((const volatile T_Eq_14bands_SP_Data*) m_data)[out].data[mod->m_nfilters-1].yn_1; break;
	case STA_EQUA_STATIC_15BANDS_SP: armAddr = (u32) &((const volatile T_Eq_15bands_SP_Data*) m_data)[out].data[mod->m_nfilters-1].yn_1; break;
	case STA_EQUA_STATIC_16BANDS_SP: armAddr = (u32) &((const volatile T_Eq_16bands_SP_Data*) m_data)[out].data[mod->m_nfilters-1].yn_1; break;

	case STA_MIXE_2INS_NCH:
	case STA_MIXE_3INS_NCH:
	case STA_MIXE_4INS_NCH:
	case STA_MIXE_5INS_NCH:
	case STA_MIXE_6INS_NCH:
	case STA_MIXE_7INS_NCH:
	case STA_MIXE_8INS_NCH:
	case STA_MIXE_9INS_NCH:
#ifdef MIXER_VER_3
		armAddr = (u32) &(((const volatile T_Mixer3_NchData*)m_data)->chData[out].out); break;
#else
		dspAddr = ((const volatile T_MixerNchData*)m_data)->out[out]; break;
#endif

	case STA_COMP_1CH:		sta_assert(out == 0); dspAddr = (u32)((const volatile T_CompanderMonoData*)  m_data)->dout; break;
	case STA_COMP_2CH:		sta_assert(out < 2);  dspAddr = (u32)((const volatile T_CompanderStereoData*)m_data)->dout[out]; break;
	case STA_COMP_4CH:		sta_assert(out < 4);  dspAddr = (u32)((const volatile T_CompanderQuadroData*)m_data)->dout[out]; break;
	case STA_COMP_6CH:		sta_assert(out < 6);  dspAddr = (u32)((const volatile T_Compander6ChData*)   m_data)->dout[out]; break;

	case STA_LMTR_1CH:		if (out == 0) {dspAddr = (u32)  ((const volatile T_Limiter2MonoData*)m_data)->dout; break;}
							if (out == 1) {armAddr = (u32) &((const volatile T_Limiter2Data*)	 m_data)->g_1; break;}
							else		  {armAddr = (u32) &((const volatile T_Limiter2Data*)	 m_data)->peak_sig;} break;

	case STA_LMTR_2CH:		if (out <  2) {dspAddr = (u32)  ((const volatile T_Limiter2StereoData*) m_data)->dout[out]; break;}
							if (out == 2) {armAddr = (u32) &((const volatile T_Limiter2Data*)	m_data)->g_1; break;}
							else		  {armAddr = (u32) &((const volatile T_Limiter2Data*)	m_data)->peak_sig;} break;

	case STA_LMTR_4CH:		if (out <  4) {dspAddr = (u32)  ((const volatile T_Limiter2QuadroData*) m_data)->dout[out]; break;}
							if (out == 4) {armAddr = (u32) &((const volatile T_Limiter2Data*)	m_data)->g_1; break;}
							else		  {armAddr = (u32) &((const volatile T_Limiter2Data*)	m_data)->peak_sig;}	break;

	case STA_LMTR_6CH:		if (out <  6) {dspAddr = (u32)  ((const volatile T_Limiter2SixChData*) m_data)->dout[out]; break;}
							if (out == 6) {armAddr = (u32) &((const volatile T_Limiter2Data*)	m_data)->g_1; break;}
							else		  {armAddr = (u32) &((const volatile T_Limiter2Data*)	m_data)->peak_sig;} break;

	case STA_CLIPLMTR_NCH:
		dspAddr = ((tDspPtr)((const volatile T_ClipLimiterData*) m_data)->dout + out); break;

	case STA_DLAY_Y_2CH:
	case STA_DLAY_Y_4CH:
	case STA_DLAY_Y_6CH:
	case STA_DLAY_X_2CH:
	case STA_DLAY_X_4CH:
	case STA_DLAY_X_6CH:
		dspAddr =  (u32)((const volatile T_DelayData*) m_data)[out].p_out; break;

	case STA_SINE_2CH:
		dspAddr =  (u32)((const volatile T_SinewaveData*) m_data)->dout[out]; break;

	case STA_PCMCHIME_12BIT_Y_2CH:
	case STA_PCMCHIME_12BIT_X_2CH:
		armAddr = (u32) &((const volatile  T_PCMChime_12bit_Data*)m_data)->out[out]; break;

	case STA_CHIMEGEN:
		armAddr = (u32) &((const volatile  T_PolyChime_Data*)m_data)->out; break;

	case STA_DCO_1CH:
	case STA_DCO_4CH_MONOZCF: //all outputs are stored in DCO_ch0
		if (out == 0) {armAddr = (u32) &((const volatile T_Dco1chData*)  m_data)[0].dspZCFout; break;}		//swZCFout
		if (out == 1) {armAddr = (u32) &((const volatile T_Dco1chData*)  m_data)[0].meterData.score; break;}
		else		  {armAddr = (u32) &((const volatile T_Dco1chParams*)m_coef)[0].dcoParams.hwZCF;}  break;	//hwZCFout = (ZCFin >> 18) & 1

	default:
		if (mod->GetDspOutAddr) {
			dspAddr = (u32)mod->GetDspOutAddr(mod, out);
		} else {sta_assert(0); /*should not happen*/}
		break;
	}

	if (armAddr) {
		dspAddr = (u32)ARM2DSP(mod->m_dsp->m_xram, armAddr);
	}

	}
_ret: return (void*)dspAddr;
}



STA_ErrorCode MOD_SetMode(tModule* mod, u32 mode)
{
	STA_ErrorCode err = STA_NO_ERROR;
	s32 goto_ret = 0;

	if (!mod) {goto _ret;}

//	#if (STA_LMTR_ON != 1) || (STA_COMP_ON != 1) || (STA_EQ_ON != 1) || (STA_SINE_ON != 1)
//	#error "Module having only ON/OFF mode, but mode STA_XXX_ON different to 1!"
//	#endif

	//check the module's type

	//EQ
	if (STA_EQUA_STATIC_1BAND_DP <= mod->m_type && mod->m_type <= STA_EQUA_STATIC_16BANDS_SP)
	{
		if (mode > (u32)STA_EQ_ON) {SetError(STA_INVALID_PARAM); goto _ret;}
	}
	//Others
	else { switch (mod->m_type) {
	//modules with only ON/OFF
	case STA_LMTR_1CH:
	case STA_LMTR_2CH:
	case STA_LMTR_4CH:
	case STA_LMTR_6CH:
	case STA_COMP_1CH:
	case STA_COMP_2CH:
	case STA_COMP_4CH:
	case STA_COMP_6CH:
	case STA_SINE_2CH:				if (mode > 1) 					 {err = STA_INVALID_PARAM;} break;
	case STA_CD_DEEMPH:				if (mode > (u32)STA_DEEMPH_AUTO) {err = STA_INVALID_PARAM;} break;
	case STA_LOUD_STATIC_DP:
	case STA_LOUD_STATIC_STEREO_DP:	if (mode > (u32)STA_LOUD_MANUAL) {err = STA_INVALID_PARAM;} break;
	case STA_TONE_STATIC_DP:
	case STA_TONE_STATIC_STEREO_DP:	if (mode > (u32)STA_TONE_MANUAL) {err = STA_INVALID_PARAM;} break;
	case STA_CLIPLMTR_NCH:			if (mode > (u32)STA_CLIPLMTR_FULL_ATT_MODE) {err = STA_INVALID_PARAM;} break;
	case STA_PEAKDETECT_NCH:		if (mode > mod->m_nin)			 {err = STA_INVALID_PARAM;} break;

	default:
		if (mod->SetMode) {
			mod->SetMode(mod, mode);
			goto_ret = 1;
		} else {
			err = STA_INVALID_PARAM;
		}
		break; //no mode to set for this module type
	}}

	if (err != STA_NO_ERROR) {SetError(err); goto _ret;}
	if (goto_ret) {goto _ret;}


	mod->m_mode = mode;			//set mode
//	mod->m_dirtyDspCoefs = 1;	//most probably but can't set this here because we don't know which filter/band might be concerned

	UPDATE_DSPCOEFS(mod);

_ret: return err;
}

#if !defined EARLY_AUDIO
static void MOD_SwapCoefBuffers(tModule* mod)
{
	if (mod && mod->m_coefsShadowing)
	{
		//swap buffers
		volatile void* tmp   = mod->m_pDspCoefBack;
		mod->m_pDspCoefBack  = mod->m_pDspCoefFront;
		mod->m_pDspCoefFront = tmp;

		//update the coef pointer in the functTable
		if (mod->m_pFuncTableYcoef) {
			DSP_WRITE(mod->m_pFuncTableYcoef, (mod->m_pDspCoefFront == mod->m_dspCoefs) ? (s32*)mod->m_ycoef : (s32*)(mod->m_ycoef + mod->m_wsizeofCoefs/2));
		}
	}
}
#endif /* !EARLY_AUDIO */

void MOD_UpdateModuleAndDspCycles(tModule* mod)
{
	if (mod && mod->m_dsp)
	{
		//update dsp cycles
		mod->m_dsp->m_mainCycleCost -= mod->m_mainCycleCost;
		//compute module cycles
		MOD_ComputeModuleCycles(mod);
		//update dsp cycles
		mod->m_dsp->m_mainCycleCost += mod->m_mainCycleCost;
	}
}

void MOD_ComputeModuleCycles(tModule* mod)
{
	if (!mod) {goto _ret;}

	if (mod->m_type < STA_MOD_LAST)
	{
	    mod->m_mainCycleCost = 0;
	    mod->m_updateCycleCost = 0;
	}

	switch (mod->m_type)
	{
	case STA_MUX_2OUT:
	    mod->m_mainCycleCost = MUX_2OUT_MAIN_CYCLES;
	    break;

	case STA_MUX_4OUT:
	    mod->m_mainCycleCost = MUX_4OUT_MAIN_CYCLES;
	    break;

	case STA_MUX_6OUT:
	    mod->m_mainCycleCost = MUX_6OUT_MAIN_CYCLES;
	    break;

	case STA_MUX_8OUT:
	    mod->m_mainCycleCost = MUX_8OUT_MAIN_CYCLES;
	    break;

	case STA_MUX_10OUT:
	    mod->m_mainCycleCost = MUX_10OUT_MAIN_CYCLES;
	    break;

	case STA_PEAKDETECT_NCH:
	    mod->m_mainCycleCost = PEAKDETECT_NCH_MAIN_CYCLES+(PEAKDETECT_NCH_PER_CH_CYCLES*mod->m_nin);
	    break;

	case STA_GAIN_STATIC_NCH:
	    mod->m_mainCycleCost = GAIN_STATIC_NCH_MAIN_CYCLES+(GAIN_STATIC_NCH_PER_STEREO_CH_CYCLES*mod->m_nout)/2;
	    break;

	case STA_GAIN_SMOOTH_NCH:
	    mod->m_mainCycleCost = GAIN_SMOOTH_NCH_MAIN_CYCLES+(GAIN_SMOOTH_NCH_PER_CH_CYCLES*mod->m_nout);
	    break;

	case STA_GAIN_LINEAR_NCH:
	    mod->m_mainCycleCost = GAIN_LINEAR_NCH_MAIN_CYCLES+(GAIN_LINEAR_NCH_PER_CH_CYCLES*mod->m_nout);
	    break;

	case STA_CD_DEEMPH:
	    mod->m_mainCycleCost = CD_DEEMPH_2CH_MAIN_CYCLES;
	    break;

	case STA_LOUD_STATIC_DP:
	    mod->m_mainCycleCost = LOUDNESS_DP_1CH_MAIN_CYCLES;
	    break;

	case STA_LOUD_STATIC_STEREO_DP:
	    mod->m_mainCycleCost = LOUDNESS_DP_2CH_MAIN_CYCLES;
	    break;

	case STA_TONE_STATIC_DP:
	    mod->m_mainCycleCost = TONE_DP_1CH_MAIN_CYCLES;
	    break;

	case STA_TONE_STATIC_STEREO_DP:
	    mod->m_mainCycleCost = TONE_DP_2CH_MAIN_CYCLES;
	    break;

	case STA_EQUA_STATIC_1BAND_DP:
	case STA_EQUA_STATIC_2BANDS_DP:
	case STA_EQUA_STATIC_3BANDS_DP:
	case STA_EQUA_STATIC_4BANDS_DP:
	case STA_EQUA_STATIC_5BANDS_DP:
	case STA_EQUA_STATIC_6BANDS_DP:
	case STA_EQUA_STATIC_7BANDS_DP:
	case STA_EQUA_STATIC_8BANDS_DP:
	case STA_EQUA_STATIC_9BANDS_DP:
	case STA_EQUA_STATIC_10BANDS_DP:
	case STA_EQUA_STATIC_11BANDS_DP:
	case STA_EQUA_STATIC_12BANDS_DP:
	case STA_EQUA_STATIC_13BANDS_DP:
	case STA_EQUA_STATIC_14BANDS_DP:
	case STA_EQUA_STATIC_15BANDS_DP:
	case STA_EQUA_STATIC_16BANDS_DP:
	    mod->m_mainCycleCost = EQ_DP_MAIN_CYCLES+(EQ_DP_PER_CH_CYCLES*mod->m_nout)+(EQ_DP_PER_BAND_PER_CH_CYCLES*mod->m_nfilters*mod->m_nout);
	    break;

	case STA_EQUA_STATIC_1BAND_SP:
	case STA_EQUA_STATIC_2BANDS_SP:
	case STA_EQUA_STATIC_3BANDS_SP:
	case STA_EQUA_STATIC_4BANDS_SP:
	case STA_EQUA_STATIC_5BANDS_SP:
	case STA_EQUA_STATIC_6BANDS_SP:
	case STA_EQUA_STATIC_7BANDS_SP:
	case STA_EQUA_STATIC_8BANDS_SP:
	case STA_EQUA_STATIC_9BANDS_SP:
	case STA_EQUA_STATIC_10BANDS_SP:
	case STA_EQUA_STATIC_11BANDS_SP:
	case STA_EQUA_STATIC_12BANDS_SP:
	case STA_EQUA_STATIC_13BANDS_SP:
	case STA_EQUA_STATIC_14BANDS_SP:
	case STA_EQUA_STATIC_15BANDS_SP:
	case STA_EQUA_STATIC_16BANDS_SP:
	    mod->m_mainCycleCost = EQ_SP_MAIN_CYCLES+(EQ_SP_PER_CH_CYCLES*mod->m_nout)+(EQ_SP_PER_BAND_PER_CH_CYCLES*mod->m_nfilters*mod->m_nout);
	    break;

	case STA_DLAY_X_2CH:
	    mod->m_mainCycleCost = DELAY_X_2CH_MAIN_CYCLES;
	    break;

	case STA_DLAY_X_4CH:
	    mod->m_mainCycleCost = DELAY_X_4CH_MAIN_CYCLES;
	    break;

	case STA_DLAY_X_6CH:
	    mod->m_mainCycleCost = DELAY_X_6CH_MAIN_CYCLES;
	    break;

	case STA_DLAY_Y_2CH:
	    mod->m_mainCycleCost = DELAY_Y_2CH_MAIN_CYCLES;
	    break;

	case STA_DLAY_Y_4CH:
	    mod->m_mainCycleCost = DELAY_Y_4CH_MAIN_CYCLES;
	    break;

	case STA_DLAY_Y_6CH:
	    mod->m_mainCycleCost = DELAY_Y_6CH_MAIN_CYCLES;
	    break;

	case STA_LMTR_1CH:
	    mod->m_mainCycleCost = LIMITER_1CH_MAIN_CYCLES;
	    mod->m_updateCycleCost = LIMITER_UPDATE_CYCLES;
	    break;

	case STA_LMTR_2CH:
	    mod->m_mainCycleCost = LIMITER_2CH_MAIN_CYCLES;
	    mod->m_updateCycleCost = LIMITER_UPDATE_CYCLES;
	    break;

	case STA_LMTR_4CH:
	    mod->m_mainCycleCost = LIMITER_4CH_MAIN_CYCLES;
	    mod->m_updateCycleCost = LIMITER_UPDATE_CYCLES;
	    break;

	case STA_LMTR_6CH:
	    mod->m_mainCycleCost = LIMITER_6CH_MAIN_CYCLES;
	    mod->m_updateCycleCost = LIMITER_UPDATE_CYCLES;
	    break;

	case STA_CLIPLMTR_NCH:
	    mod->m_mainCycleCost = CLIP_LIMITER_NCH_MAIN_CYCLES+(CLIP_LIMITER_NCH_PER_CH_CYCLES*mod->m_nout);
	    break;

	case STA_COMP_1CH:
	    mod->m_mainCycleCost = COMPANDER_1CH_MAIN_CYCLES;
	    mod->m_updateCycleCost = COMPANDER_UPDATE_CYCLES;
	    break;

	case STA_COMP_2CH:
	    mod->m_mainCycleCost = COMPANDER_2CH_MAIN_CYCLES;
	    mod->m_updateCycleCost = COMPANDER_UPDATE_CYCLES;
	    break;

	case STA_COMP_4CH:
	    mod->m_mainCycleCost = COMPANDER_4CH_MAIN_CYCLES;
	    mod->m_updateCycleCost = COMPANDER_UPDATE_CYCLES;
	    break;

	case STA_COMP_6CH:
	    mod->m_mainCycleCost = COMPANDER_6CH_MAIN_CYCLES;
	    mod->m_updateCycleCost = COMPANDER_UPDATE_CYCLES;
	    break;

	case STA_MIXE_2INS_NCH:
	case STA_MIXE_3INS_NCH:
	case STA_MIXE_4INS_NCH:
	case STA_MIXE_5INS_NCH:
	case STA_MIXE_6INS_NCH:
	case STA_MIXE_7INS_NCH:
	case STA_MIXE_8INS_NCH:
	case STA_MIXE_9INS_NCH:
	    mod->m_mainCycleCost = MIXER3_MAIN_CYCLES+(MIXER3_PER_CH_CYCLES*mod->m_nout)+(MIXER3_PER_INPUT_PER_CH_CYCLES*mod->m_ninpc*mod->m_nout);
	    break;

	case STA_SINE_2CH:
	    mod->m_mainCycleCost = SINE_2CH_MAIN_CYCLES;
	    break;

	case STA_CHIMEGEN:
	    mod->m_mainCycleCost = CHIMEGEN_MAIN_CYCLES;
	    break;

	case STA_PCMCHIME_12BIT_X_2CH:
	    mod->m_mainCycleCost = PCMCHIME_12BIT_X_2CH_MAIN_CYCLES;
	    break;

	case STA_PCMCHIME_12BIT_Y_2CH:
	    mod->m_mainCycleCost = PCMCHIME_12BIT_Y_2CH_MAIN_CYCLES;
	    break;

	case STA_SPECMTR_NBANDS_1BIQ:
	    mod->m_mainCycleCost = SPECMTR_NBANDS_1BIQ_MAIN_CYCLES+(SPECMTR_NBANDS_1BIQ_PER_BAND_CYCLES*mod->m_nfilters);
	    break;

	case STA_BITSHIFTER_NCH:
	    mod->m_mainCycleCost = BITSHIFTER_NCH_MAIN_CYCLES+(BITSHIFTER_NCH_PER_CH_CYCLES*mod->m_nout);
	    break;

	default: break;
	}

	mod->m_mainCycleCost += MAIN_LOOP_PER_MODULE_CYCLES;

_ret: return;
}

#if !defined EARLY_AUDIO
//===========================================================================
// MUX / SELECTOR
//===========================================================================
static void MUX_InitDspData(tModule* mod);

static void MUX_Init(tModule* mod)
{
	if (!mod) {goto _ret;}{

	tMux* const mux = mod->m_sub;
	u32 i;

	mod->InitDspData = &MUX_InitDspData;		//called by BuildFlow() when populating DSP mems

	FORi(mod->m_nout) {
		mux->m_sels[i] = 0;	//init: all outputs are connected to in[0] = 0 (which could be a "mute" input)
	}

	}
_ret: return;
}

static void MUX_InitDspData(tModule* mod)
{
	if (!mod) {goto _ret;}{

	tMux*       		 const mux   = mod->m_sub;
	volatile T_MuxData*  const data  = mod->m_dspData; 	//ARM addr
	volatile T_MuxParam* const param = mod->m_dspCoefs;	//ARM addr
	u32 i;

	sta_assert(mod->m_dspData && mod->m_dspCoefs);

	//init XMEM data
	DSP_WRITE(&data->ins[0], 0);	//(for mute)

	//init YMEM coefs
	FORi(mod->m_nout) {
		DSP_WRITE(&param->sel[i], mux->m_sels[i]);
	}

	}
_ret: return;
}

//===========================================================================
// CD DEEMPHASIS
//===========================================================================
static void DEEM_InitDspData(tModule* mod);
static void DEEM_UpdateDspMode(tModule* mod);

static void DEEM_Init(tModule* mod)
{
	if (!mod) {goto _ret;}
	mod->InitDspData    = &DEEM_InitDspData;
	mod->UpdateDspCoefs = &DEEM_UpdateDspMode;
	mod->m_mode			= (u32)STA_DEEMPH_OFF;

_ret: return;
}

static void DEEM_InitDspData(tModule* mod)
{
	if (!mod) {goto _ret;}{

	volatile T_CDDeemphasisData*  const data = mod->m_dspData;  //ARM addr
	volatile T_CDDeemphasisParam* const coef = mod->m_dspCoefs;	//ARM addr
	tDspPtr  xdata = (tDspPtr)mod->m_xdata; //DSP addr

	sta_assert(mod->m_dspData && mod->m_dspCoefs);

	//init XMEM data
	//XMEM Layout: T_CDDeemphasisData | inL | inR | outL | outR
	xdata += WSIZEOF(T_CDDeemphasisData);
	DSP_WRITE(&data->p_inL,				xdata);			//DSP addr
	DSP_WRITE(&data->p_inR,				(s32*)(xdata + 1));
	DSP_WRITE(&data->p_outL,			(s32*)(xdata + 2));
	DSP_WRITE(&data->p_outR,			(s32*)(xdata + 3));
	DSP_WRITE(&data->deemFilterDelay[0], 0);
	DSP_WRITE(&data->deemFilterDelay[1], 0);
	DSP_WRITE(&data->deemFilterDelay[2], 0);
	DSP_WRITE(&data->deemFilterDelay[3], 0);

	//init YMEM coefs
	DSP_WRITE(&coef->deemIsOn, (mod->m_mode != (u32)STA_DEEMPH_OFF) ? 1 : 0);
	DSP_WRITE(&coef->deemFilterCoeffs[0], FRAC(-0.089243133770362)); //b1
	DSP_WRITE(&coef->deemFilterCoeffs[1], FRAC(0.448465451161873));	 //b0
	DSP_WRITE(&coef->deemFilterCoeffs[2], FRAC(0.640777682608490));	 //a1

	//NOTE: DSP should not call audio_cd_deemphasis_init()

	}
_ret: return;
}


//TODO DEEMP_Init(): OSAL_s32TimerSetTime and STA_DEEMPH_AUTO
static void DEEM_UpdateDspMode(tModule* mod)
{
	int isOn = 0;

	if (!mod) {goto _ret;}

	sta_assert(DSPCOEFS(mod));

	//stop timer
	//OSAL_s32TimerSetTime(timerAudioDeemphasis1, 0, 0);

	switch (mod->m_mode) {
	case STA_DEEMPH_ON:		isOn = 1; break;
	case STA_DEEMPH_OFF:	isOn = 0; break;
	case STA_DEEMPH_AUTO:	isOn = 1; 			//TMP
		//TODO
//		isOn = (0x00000020 & AIF_1.SPDIFCSR.reg) ? 1 : 0;
		//start timer
//		OSAL_s32TimerSetTime(timerAudioDeemphasis1, 500, 500);
		break;
	default: sta_assert(0); break;
	}

	DSP_WRITE(&((volatile T_CDDeemphasisParam*)mod->m_dspCoefs)->deemIsOn, isOn);

_ret: return;
}

//===========================================================================
// PEAK DETECTOR
//===========================================================================
static void PEAKDETECT_InitDspData(tModule* mod);
static void PEAKDETECT_UpdateDspCoefs(tModule* mod);

//must be called before BuildFlow()
void PEAKDETECT_SetNch(tModule* mod, u32 nch)
{
	if (!mod) {goto _ret;}
	sta_assert(!mod->m_dspCoefs);				//must be called before BuildFlow()

	mod->m_nin  = nch;
	mod->m_nout = 0;

	mod->m_wsizeofCoefs = WSIZEOF(T_PeakDetectNchParams) + 1 + nch; //numCh + Peaks[n]
	mod->m_dsp->m_xmemPoolEstimatedSize -= mod->m_wsizeofData;
	mod->m_wsizeofData  = WSIZEOF(T_PeakDetectData) + nch;	      //in[n]
	mod->m_dsp->m_xmemPoolEstimatedSize += mod->m_wsizeofData;

_ret: return;
}

static void PEAKDETECT_Init(tModule* mod)
{
	if (!mod) {goto _ret;}

	mod->InitDspData    = &PEAKDETECT_InitDspData;		//called by BuildFlow() when populating DSP mems
	mod->UpdateDspCoefs = &PEAKDETECT_UpdateDspCoefs;
	mod->m_mode = 0;			//disable by default

	PEAKDETECT_SetNch(mod, 2); //by default

	//note: cannot pre-init m_peaks[] before the buildflow()

_ret: return;
}

void PEAKDETECT_Reset(tModule* mod, u32 nch)
{
	if (!mod) {goto _ret;}{

	tPeakDetect* const peakdetect = mod->m_sub;
	volatile T_PeakDetectNchParams*	const coef = mod->m_dspCoefs;	//ARM addr
	s32 oldnumCh;

	sta_assert(mod->m_dspCoefs);

	oldnumCh = DSP_READ(&coef->numCh);
	DSP_WRITE(&coef->numCh, 0);					//disable the peak detection

	//if (flags & STA_PEAK_USESLEEP)
		STA_SLEEP(1);

	DSP_WRITE(&peakdetect->m_peaks[nch], 0);	//reset
	DSP_WRITE(&coef->numCh, oldnumCh); 			//restore mode

	}
_ret: return;
}

static void PEAKDETECT_InitDspData(tModule* mod)
{
	if (!mod) {goto _ret;}{

	tPeakDetect*					const peakdetect = mod->m_sub;
	volatile T_PeakDetectData*		const data = mod->m_dspData;	//ARM addr
	volatile T_PeakDetectNchParams*	const coef = mod->m_dspCoefs;	//ARM addr
	tDspPtr  xdata = (tDspPtr)mod->m_xdata; //DSP addr
	tDspPtr  ycoef = (tDspPtr)mod->m_ycoef; //DSP addr
	u32 i;

	sta_assert(data && coef);

	//init XMEM data
	//XMEM layout: T_PeakDetectData | ins[nch]
	xdata += WSIZEOF(T_PeakDetectData);
	DSP_WRITE(&data->ins, (frac*)(xdata));								//DSP addr

	//init YMEM coefs
	//YMEM layout: T_PeakDetectNchParams | gains[nch] | scales[nch]
	ycoef += WSIZEOF(T_PeakDetectNchParams);
	DSP_WRITE(&coef->numCh, mod->m_mode);//mod->m_nin;
	DSP_WRITE(&coef->pch,   (T_PeakDetect1chParams*)(ycoef));		//DSP addr

	//store the arm address of the dsp Peaks
	peakdetect->m_peaks = (vs32*)((u32)coef + sizeof(T_PeakDetectNchParams));

	FORi(mod->m_nin) {DSP_WRITE(&peakdetect->m_peaks[i], 0);}

	}
_ret: return;
}

static void PEAKDETECT_UpdateDspCoefs(tModule* mod)
{
	if (!mod) {goto _ret;}

	//set mode = number of real active channels
	sta_assert(DSPCOEFS(mod));

	DSP_WRITE(&((volatile T_PeakDetectNchParams*)mod->m_dspCoefs)->numCh, mod->m_mode);

_ret: return;
}

//===========================================================================
// BIT SHIFTER
//===========================================================================
static void BITSHIFT_InitDspData(tModule* mod);

static void BITSHIFT_Init(tModule* mod)
{
	if (!mod) {goto _ret;}

	mod->InitDspData  = &BITSHIFT_InitDspData;		//called by BuildFlow() when populating DSP mems

	BITSHIFT_SetNch(mod, 2); //by default

	//note: cannot pre-init the leftShifts before the buildflow()

_ret: return;
}

//must be called before BuildFlow()
void BITSHIFT_SetNch(tModule* mod, u32 nch)
{
	if (!mod) {goto _ret;}

	sta_assert(!mod->m_dspCoefs);				//must be called before BuildFlow()

	nch = (nch + 1) & 0xFFFFFFFE; //must be 2n ch

	mod->m_nin  = nch;
	mod->m_nout = nch;

	mod->m_wsizeofCoefs = WSIZEOF(T_BitShiftNchParams) + 1 + nch; //halvedNumCh + lefShift[n]
	mod->m_dsp->m_xmemPoolEstimatedSize -= mod->m_wsizeofData;
	mod->m_wsizeofData  = WSIZEOF(T_BitShiftData) + nch*2;	      //in[n]+out[n]
	mod->m_dsp->m_xmemPoolEstimatedSize += mod->m_wsizeofData;

_ret: return;
}

static void BITSHIFT_InitDspData(tModule* mod)
{
	if (!mod) {goto _ret;}{

	tBitShift*						const bitshift = mod->m_sub;
	volatile T_BitShiftData*		const data = mod->m_dspData;	//ARM addr
	volatile T_BitShiftNchParams*	const coef = mod->m_dspCoefs;	//ARM addr
	tDspPtr  xdata = (tDspPtr)mod->m_xdata; //DSP addr
	tDspPtr  ycoef = (tDspPtr)mod->m_ycoef; //DSP addr
	u32 i;

	sta_assert(data && coef);

	//init XMEM data
	//XMEM layout: T_BitShiftData | ins[nch] | out[nch]
	xdata += WSIZEOF(T_BitShiftData);
	DSP_WRITE(&data->ins, (frac*)(xdata));								//DSP addr
	DSP_WRITE(&data->out, (frac*)(xdata + mod->m_nin));

	//init YMEM coefs
	//YMEM layout: T_BitShiftNchParams | gains[nch] | scales[nch]
	ycoef += WSIZEOF(T_BitShiftNchParams);
	DSP_WRITE(&coef->halvedNumCh, (s32)mod->m_nin / 2);
	DSP_WRITE(&coef->leftShift,   (s32*)(ycoef));		//DSP addr

	//store the arm address of the dsp leftShift
	bitshift->m_leftShifts  = (vs32*)((u32)coef + sizeof(T_BitShiftNchParams));	//ARM addr

	FORi(mod->m_nin) {
		DSP_WRITE(&bitshift->m_leftShifts[i], 4); //(note: init with 0+4 due to the DSP implementation)
	}

	}
_ret: return;
}


//must be called after BuildFlow()
void BITSHIFT_SetLeftShifts(tModule* mod, u32 chmask, s32 leftShift)
{
	if (!mod) {goto _ret;}{

	tBitShift* const bitshift = mod->m_sub;
	u32 i;

	sta_assert(DSPCOEFS(mod));
	sta_assert(bitshift->m_leftShifts);

	//update dsp coefs
	FORMASKEDi(mod->m_nin, chmask) {
		DSP_WRITE(&bitshift->m_leftShifts[i], leftShift + 4); //(note: +4 due to the DSP implementation)
	}

	}
_ret: return;
}

//===========================================================================
// LOUDNESS
//===========================================================================
static void LOUD_InitDspData(tModule* mod);
static void LOUD_UpdateDspCoefs(tModule* mod);

//TODO(low) LOUD: bind to Mixers

static void LOUD_Init(tModule* mod)
{
	if (!mod) {goto _ret;}{

	tLoudness* const loud = mod->m_sub;

	mod->InitDspData    = &LOUD_InitDspData; 	//DSP will init the biquads as bypass
	mod->UpdateDspCoefs = &LOUD_UpdateDspCoefs;
	mod->m_pFilters  	= loud->m_filters;
	mod->m_coefsShadowing = (u32)TRUE;
	mod->m_mode 	 = (u32)STA_LOUD_OFF; //DSP will init the biquads as bypass
	loud->m_bassType = STA_LOUD_BANDBOOST_2;
	loud->m_trebType = STA_LOUD_BANDBOOST_2;

	LOUD_SetAutoBassQLevels(mod, -24*BGS, 24*BGS, 30, 30);				//as in ADR3
	LOUD_SetAutoGLevels(mod, STA_LOUD_BASS, -200, 0, 15*BGS, -15*BGS);	//as in ADR3
	loud->m_autoGLevels[STA_LOUD_TREB] = loud->m_autoGLevels[STA_LOUD_BASS];

	//init biquads as in ADR3
	BIQ_Init(&loud->m_filters[STA_LOUD_BASS], STA_BIQUAD_BAND_BOOST_2, 0, 30, 200);
	BIQ_Init(&loud->m_filters[STA_LOUD_TREB], STA_BIQUAD_BAND_BOOST_2, 0, 30, 10000);

	// /!\ DSP will init the biquads as bypass

	}
_ret: return;
}

static void LOUD_InitDspData(tModule* mod)
{
	if (!mod) {goto _ret;}{
//	tLoudness* const loud = mod->m_sub;

	sta_assert(mod->m_dspCoefs);

	// /!\ DSP will init the biquads as bypass
//	LOUD_UpdateDspCoefs(mod);

	//TODO: init dsp here (both Front and Back coef buffers)

	}
_ret: return;
}

void LOUD_SetTypes( tModule* mod, STA_LoudFilterType bassType, STA_LoudFilterType trebType )
{
	if (!mod) {goto _ret;}{

	tLoudness* const loud = mod->m_sub;

	STA_FilterType bassBiqType = (bassType == STA_LOUD_SHELVING_1) ? STA_BIQUAD_BASS_SHELV_1 :
								 (bassType == STA_LOUD_SHELVING_2) ? STA_BIQUAD_BASS_SHELV_2 :
								/* STA_LOUD_BANDBOOST_2*/ 			 STA_BIQUAD_BAND_BOOST_2;

	STA_FilterType trebBiqType = (trebType == STA_LOUD_SHELVING_1) ? STA_BIQUAD_TREB_SHELV_1 :
								 (trebType == STA_LOUD_SHELVING_2) ? STA_BIQUAD_TREB_SHELV_2 :
								 /* STA_LOUD_BANDBOOST_2*/ 			 STA_BIQUAD_BAND_BOOST_2;

	BIQ_SetFilterType(&loud->m_filters[STA_LOUD_BASS], bassBiqType);
	BIQ_SetFilterType(&loud->m_filters[STA_LOUD_TREB], trebBiqType);

	loud->m_bassType = bassType;
	loud->m_trebType = trebType;

	mod->m_dirtyDspCoefs = 0xF;

	//update dsp coefs
	if (DSPCOEFS(mod)) {
		LOUD_UpdateDspCoefs(mod);
	}

	}
_ret: return;
}

void LOUD_SetAutoBassQLevels(tModule* mod, s32 Gstart, s32 Gstop, s32 Qstart, s32 Qstop)
{
	if (!mod) {goto _ret;}

	sta_assert((-24*BGS <= Gstart) && (Gstart <= 24*BGS));
	sta_assert((-24*BGS <= Gstop ) && (Gstop  <= 24*BGS));
	sta_assert((      1 <= Qstart) && (Qstart <= 100));
	sta_assert((      1 <= Qstop ) && (Qstop  <= 100));

	_STA_SetLinLimiter(&((tLoudness*)mod->m_sub)->m_autoBassQLevel, Gstart, Gstop, Qstart, Qstop);

	//update dsp coefs
	if (DSPCOEFS(mod) && mod->m_mode == (u32)STA_LOUD_AUTO) {
		LOUD_UpdateDspCoefs(mod);
	}

_ret: return;
}

void LOUD_SetAutoGLevels(tModule* mod, STA_LoudFilter filter, s32 Vstart, s32 Vstop, s32 Gmin, s32 Gmax)
{
	if (!mod) {goto _ret;}

	sta_assert((  -1200 <= Vstart) && (Vstart <= 0));
	sta_assert((  -1200 <= Vstop)  && (Vstop  <= 0));
	sta_assert((-24*BGS <= Gmin)   && (Gmin   <= 24*BGS));
	sta_assert((-24*BGS <= Gmax)   && (Gmax   <= 24*BGS));

	_STA_SetLinLimiter(&((tLoudness*)mod->m_sub)->m_autoGLevels[filter], Vstart, Vstop, Gmin, Gmax);

	//update dsp coefs
	if (DSPCOEFS(mod) && mod->m_mode == (u32)STA_LOUD_AUTO) {
		LOUD_UpdateDspCoefs(mod);
	}

_ret: return;
}

static void LOUD_UpdateDspCoefs(tModule* mod)
{
	if (!mod) {goto _ret;}{

	tLoudness* 		  			const loud      = mod->m_sub;
	tBiquad* 		  			const m_filters = mod->m_pFilters;
	volatile T_Loudness_Coefs*	const dspCoef   = mod->m_pDspCoefBack;

	s32 goto_swap_coefs = 0;
	s32 goto_ret = 0;

	sta_assert(DSPCOEFS(mod));

	//update dsp coefs
	switch (mod->m_mode)
	{
	case STA_LOUD_OFF:
		BIQ_SetDspCoefs2bypass(&dspCoef->LP_coefs); //bass
		BIQ_SetDspCoefs2bypass(&dspCoef->HP_coefs); //treble
		mod->m_dirtyDspCoefs = 0xF; //DSP is dirtied with pass-through coefs
		goto_swap_coefs = 1;
		break;

	case STA_LOUD_AUTO:
		//set auto Gains from mixers'volumes
		if (loud->m_pMixer) {
			BIQ_SetGain(&m_filters[0],_STA_GetInterpolatedValue(&loud->m_autoGLevels[0], loud->m_pMixer->m_volume));
			BIQ_SetGain(&m_filters[1],_STA_GetInterpolatedValue(&loud->m_autoGLevels[1], loud->m_pMixer->m_volume));
		}
		//set auto bass Q
		BIQ_SetQual(&m_filters[0], _STA_GetInterpolatedValue(&loud->m_autoBassQLevel, BIQ_GetGain(&m_filters[0])));
		break;

	case STA_LOUD_MANUAL:
		break;

	default: goto_ret = 1; break;
	}

	if (goto_swap_coefs) {goto swap_coefs;}
	if (goto_ret) {goto _ret;}


	//update the biquad coefs
	if (m_filters[0].m_dirtyBiquad) {
		BIQ_UpdateCoefs(&m_filters[0]);
		mod->m_dirtyDspCoefs |= 1; //dirty dsp
	}
	if (m_filters[1].m_dirtyBiquad) {
		BIQ_UpdateCoefs(&m_filters[1]);
		mod->m_dirtyDspCoefs |= 1 << 1; //dirty dsp
	}


	//update dsp
	//note: because of the dual coef buffer, we need to update all the coefs each time.
//	if (mod->m_dirtyDspCoefs & (1<<0))
	BIQ_UpdateDspCoefs(&m_filters[0], &dspCoef->LP_coefs);
//	if (mod->m_dirtyDspCoefs & (1<<1))
	BIQ_UpdateDspCoefs(&m_filters[1], &dspCoef->HP_coefs);

	mod->m_dirtyDspCoefs = 0;


swap_coefs:
	MOD_SwapCoefBuffers(mod);

	}
_ret: return;
}


//===========================================================================
// TONE
//===========================================================================
static void TONE_UpdateDspCoefs(tModule* mod);
static void TONE_InitDspData(tModule* mod);

static void TONE_Init(tModule* mod)
{
	if (!mod) {goto _ret;}{

	tTone* const tone = mod->m_sub;

	mod->InitDspData    = &TONE_InitDspData; 	//DSP will init the biquads as bypass
	mod->UpdateDspCoefs = &TONE_UpdateDspCoefs;
	mod->m_pFilters     = tone->m_filters;
	mod->m_coefsShadowing = (u32)TRUE;
	mod->m_mode 	    = (u32)STA_TONE_OFF; //DSP will init the biquads as bypass
	tone->m_bassType    = STA_TONE_BANDBOOST_2;
	tone->m_trebType    = STA_TONE_BANDBOOST_2;
	TONE_SetAutoBassQLevels(mod, -24*BGS, 24*BGS, 1, 60);		//as in ADR3

	//init biquads as in ADR3
	BIQ_Init(&tone->m_filters[0], STA_BIQUAD_BAND_BOOST_2, 0, 30, 200);
	BIQ_Init(&tone->m_filters[1], STA_BIQUAD_BAND_BOOST_2, 0, 30, 3000);
	BIQ_Init(&tone->m_filters[2], STA_BIQUAD_BAND_BOOST_2, 0, 30, 16000);

	}
_ret: return;
}

static void TONE_InitDspData(tModule* mod)
{
	if (!mod) {goto _ret;}{

	sta_assert(mod->m_dspCoefs);

	// /!\ DSP will init the biquads as bypass
//	TONE_UpdateDspCoefs(mod);

	}
_ret: return;
}

void TONE_SetTypes( tModule* mod, STA_ToneFilterType bassType, STA_ToneFilterType trebType )
{
	if (!mod) {goto _ret;}{

	tTone* const tone = mod->m_sub;

	STA_FilterType bBqType = (bassType == STA_TONE_SHELVING_1) ? STA_BIQUAD_BASS_SHELV_1 :
							 (bassType == STA_TONE_SHELVING_2) ? STA_BIQUAD_BASS_SHELV_2 :
							 /* STA_TONE_BANDBOOST_2*/ 			 STA_BIQUAD_BAND_BOOST_2;

	STA_FilterType tBqType = (trebType == STA_TONE_SHELVING_1) ? STA_BIQUAD_TREB_SHELV_1 :
							 (trebType == STA_TONE_SHELVING_2) ? STA_BIQUAD_TREB_SHELV_2 :
							 /* STA_TONE_BANDBOOST_2*/ 			 STA_BIQUAD_BAND_BOOST_2;

	if (bassType == STA_TONE_DUALMODE) {
		//TODO: TONE_DUALMODE: if G<0 bqType = STA_BIQUAD_BASS_SHELV_2
		STA_PRINTF("STA: STA_TONE_DUALMODE not yet implemented...%s\n","");
		//which G ?
	}

	BIQ_SetFilterType(&tone->m_filters[0], bBqType);
	BIQ_SetFilterType(&tone->m_filters[2], tBqType);

	tone->m_bassType = bassType;
	tone->m_trebType = trebType;

	//update dsp coefs
	if (DSPCOEFS(mod)) {
		TONE_UpdateDspCoefs(mod);
	}

	}
_ret: return;
}

void TONE_SetAutoBassQLevels(tModule* mod, s32 Gstart, s32 Gstop, s32 Qstart, s32 Qstop)
{
	sta_assert((-24*BGS <= Gstart) && (Gstart <= 24*BGS));
	sta_assert((-24*BGS <= Gstop)  && (Gstop  <= 24*BGS));
	sta_assert((      1 <= Qstart) && (Qstart <= 100));
	sta_assert((      1 <= Qstop)  && (Qstop  <= 100));

	if (!mod) {goto _ret;}

	_STA_SetLinLimiter(&((tTone*)mod->m_sub)->m_autoBassQLevel, Gstart, Gstop, Qstart, Qstop);

	//update dsp coefs
	if (DSPCOEFS(mod) && mod->m_mode == (u32)STA_TONE_AUTO_BASS_Q) {
		TONE_UpdateDspCoefs(mod);
	}

_ret: return;
}

static void TONE_UpdateDspCoefs(tModule* mod)
{
	if (!mod) {goto _ret;}{

	tTone*					const tone      = mod->m_sub;
	tBiquad*				const m_filters = mod->m_pFilters;
	volatile T_Tone_Coefs*	const dspCoef   = mod->m_pDspCoefBack;

	s32 goto_swap_coefs = 0;
	s32 goto_ret = 0;

	sta_assert(DSPCOEFS(mod));

	//update dsp coefs
	switch (mod->m_mode)
	{
	case STA_TONE_OFF:
		BIQ_SetDspCoefs2bypass(&dspCoef->coefs[0]); //bass
		BIQ_SetDspCoefs2bypass(&dspCoef->coefs[1]); //middle
		BIQ_SetDspCoefs2bypass(&dspCoef->coefs[2]); //treble
		mod->m_dirtyDspCoefs = 0xF; //DSP is dirtied with pass-through coefs
		goto_swap_coefs = 1;
		break;

	case STA_TONE_AUTO_BASS_Q:
		BIQ_SetQual(&m_filters[0], _STA_GetInterpolatedValue(&tone->m_autoBassQLevel, BIQ_GetGain(&m_filters[0])));
		break;

	case STA_TONE_MANUAL:
		break;

	default: goto_ret = 1; break;
	}

	if (goto_swap_coefs) {goto swap_coefs;}
	if (goto_ret) {goto _ret;}


	//update the biquads coefs
	if (m_filters[0].m_dirtyBiquad) {
		BIQ_UpdateCoefs(&m_filters[0]); mod->m_dirtyDspCoefs |= 1;}
	if (m_filters[1].m_dirtyBiquad) {
		BIQ_UpdateCoefs(&m_filters[1]); mod->m_dirtyDspCoefs |= 1 << 1;}
	if (m_filters[2].m_dirtyBiquad) {
		BIQ_UpdateCoefs(&m_filters[2]); mod->m_dirtyDspCoefs |= 1 << 2;}

	//update the dsp
	//note: because of the dual coef buffer, we need to update all the coefs each time.
//	if (mod->m_dirtyDspCoefs & (1<<0))
	BIQ_UpdateDspCoefs(&m_filters[0], &dspCoef->coefs[0]);
//	if (mod->m_dirtyDspCoefs & (1<<1))
	BIQ_UpdateDspCoefs(&m_filters[1], &dspCoef->coefs[1]);
//	if (mod->m_dirtyDspCoefs & (1<<2))
	BIQ_UpdateDspCoefs(&m_filters[2], &dspCoef->coefs[2]);

	mod->m_dirtyDspCoefs = 0;


swap_coefs:
	MOD_SwapCoefBuffers(mod);

	}
_ret: return;
}

//===========================================================================
// EQUALIZER DP
//===========================================================================
static void EQUA_InitDspData(tModule* mod);
static void EQUA_UpdateDspCoefs(tModule* mod);
static void EQUA_UpdateDspCoefs_sp(tModule* mod);


//The following bands' frequencies are just proposal...

static const u16 g_eq_bands_1[1]   = {                           1000}; //src: Olivier
static const u16 g_eq_bands_2[2]   = {                           1000,           10000}; //src: Olivier
static const u16 g_eq_bands_3[3]   = {              200,                 3000,           16000}; //src: ?? like ADR3 TONE default
static const u16 g_eq_bands_4[4]   = {              200,         1000,   3000,  6300          }; //src: ?? chris
static const u16 g_eq_bands_5[5]   = {                 300, 500, 1200, 2500,    6300          }; //src: ?? chris
//static const u16 g_eq_bands_5[5] = {        63, 125,           1000,          6000,  12000  }; //src: ?? chris
static const u16 g_eq_bands_6[6]   = {                 300, 500, 1200,       4000, 8000, 16000}; //src: ADR3
static const u16 g_eq_bands_7[7]   = {        63, 150,      400, 1000, 2500,    6300,    16000}; //src: Stef's car
static const u16 g_eq_bands_8[8]   = {            125, 250, 500, 1000, 2000, 4000, 8000, 16000}; //src: GS2 Music Player
static const u16 g_eq_bands_9[9]   = {    31, 63, 125, 250, 500, 1000, 2000, 4000, 8000       }; //src: Chris PC "SRC Premium Sound PRO" settings
//static const u16 g_eq_bands_9[9] = {        50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000}; //src: Ryo
static const u16 g_eq_bands_10[10] = {    31, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000}; //src: OCTAVE BAND ISO 266 (http://digitalprosound.digitalmedianet.com/articles/viewarticle.jsp?id=8953)
static const u16 g_eq_bands_11[11] = {20, 31, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000}; 	//TODO: find suitable 11 bands
static const u16 g_eq_bands_12[12] = {    31, 63, 110, 220, 350, 700, 1600, 3200, 4800, 7000, 10000, 12000};  //src: http://www.terrywest.nl/equalizers.html
//static u16 const g_eq_bands_13[13] = {    31, 63, 110, 220, 350, 700, 1600, 3200, 4800, 7000, 10000, 12000, 14000}; //src: http://www.terrywest.nl/equalizers.html
//static u16 const g_eq_bands_13[13] = {        63, 100, 160, 250, 400, 630, 1000, 1600, 2500, 4000, 6300, 10000, 16000};  //src: google
static const u16 g_eq_bands_13[13] = {20, 34, 60, 105, 185, 327, 577, 1017, 1794, 3165, 5582, 9846, 17367}; //src: chris's equi-ratio repartition between [15:24000]hz
static const u16 g_eq_bands_14[14] = {20, 32, 54, 92, 155, 263, 445, 755, 1278, 2165, 3667, 6211, 10520, 17819}; //src: chris's equi-ratio repartition between [15:24000]hz
//static const u16 g_eq_bands_15[15] = {20, 30, 50, 81, 133, 218, 356, 582, 952, 1557, 2547, 4165, 6811, 11138, 18214}; //src: chris's equi-ratio repartition between [15:24000]hz
static const u16 g_eq_bands_15[15] = {    31, 50, 80, 125, 200, 315, 500, 800, 1300, 2000, 3200, 5000, 8000, 12500, 20000}; //src: ctm radio
//static const u16 g_eq_bands_16[16] = {20, 32, 50, 80, 126, 200, 317, 502, 796, 1260, 2000, 3170, 5024, 7960, 12600, 20000}; //src: http://www.voxengo.com/product/marvelgeq/
static const u16 g_eq_bands_16[16] = {20, 30, 50, 80, 125, 200, 300, 500, 800, 1000, 2000, 3000, 5000, 8000, 13000, 20000}; //src: http://www.voxengo.com/product/marvelgeq/


const u16* const g_eq_bands[16] = {g_eq_bands_1, g_eq_bands_2, g_eq_bands_3, g_eq_bands_4, g_eq_bands_5, g_eq_bands_6, g_eq_bands_7, g_eq_bands_8,
	g_eq_bands_9, g_eq_bands_10, g_eq_bands_11, g_eq_bands_12, g_eq_bands_13, g_eq_bands_14, g_eq_bands_15, g_eq_bands_16};


static void EQUA_InitBands(tModule* mod)
{
	if (!mod) {goto _ret;}{

	tEqualizer* const eq = mod->m_sub;
	const u32 nbands = mod->m_nfilters;
	u32 i;

	sta_assert(1 <= nbands && nbands <= STA_MAX_EQ_NUM_BANDS);

	//init biquads according to predefined bands. (each as pass-through)
	for (i = 0; i < nbands; i++)
	{
		BIQ_Init(&eq->m_bands[i], STA_BIQUAD_BAND_BOOST_2, 0, 30, g_eq_bands[nbands-1][i]);

		eq->m_bands[i].m_simplePrec = eq->m_simplePrec;
	}

	}
_ret: return;
}

static void EQUA_Init(tModule* mod)
{
	if (!mod) {goto _ret;}{

	tEqualizer* const eq = mod->m_sub;

	eq->m_simplePrec = ((STA_EQUA_STATIC_1BAND_SP <= mod->m_type) && (mod->m_type <= STA_EQUA_STATIC_16BANDS_SP)) ? 1 : 0;

	mod->AdjustDspMemSize = &EQUA_AdjustDspMemSize;
	mod->InitDspData    = &EQUA_InitDspData;	//in order to set coef->numBands post-build flow
	mod->UpdateDspCoefs = (eq->m_simplePrec) ? &EQUA_UpdateDspCoefs_sp : &EQUA_UpdateDspCoefs;
	mod->m_pFilters		= eq->m_bands;
	mod->m_mode			= (u32)STA_EQ_OFF;		//DSP will init the biquads as bypass
	mod->m_coefsShadowing = (u32)TRUE;

	EQUA_InitBands(mod);

	}
_ret: return;
}

//called during BuildFLow()
void EQUA_AdjustDspMemSize(tModule* mod)
{
	if (!mod) {goto _ret;}

	//update the xmem
    mod->m_dsp->m_xmemPoolEstimatedSize -= mod->m_wsizeofData;
	mod->m_wsizeofData = (g_modulesInfo[mod->m_type].wsizeofData/2) * mod->m_nin; //(note: /2 because the size in this table was for 2 ch)
    mod->m_dsp->m_xmemPoolEstimatedSize += mod->m_wsizeofData;

_ret: return;
}

static void EQUA_InitDspData(tModule* mod)
{
	if (!mod) {goto _ret;}{

	volatile T_Eq_3bands_Coefs* coef;	//(note: here, we use the "base" coef type of all EQ types)

	sta_assert(mod->m_dspCoefs);

	//init YMEM (front coefs: dsp side)
	coef = (volatile T_Eq_3bands_Coefs*) mod->m_pDspCoefFront;
	DSP_WRITE(&coef->numBands, mod->m_nfilters);
	DSP_WRITE(&coef->numCh,    mod->m_nin);		//2ch by default

	//init YMEM (back coefs: arm side)
	coef = (volatile T_Eq_3bands_Coefs*) mod->m_pDspCoefBack;
	DSP_WRITE(&coef->numBands, mod->m_nfilters);
	DSP_WRITE(&coef->numCh,    mod->m_nin);		//2ch by default

	mod->UpdateDspCoefs(mod);

	}
_ret: return;
}


//Num bands can't be changed runtime because needs to change the filterFunc
//Instead the user should choose a new equalizer type.
/*
void EQUA_SetNumBands( tModule* mod, u32 numBands )
{
	mod->m_nfilters = numBands;

	EQUA_InitBands(mod);

	//update dsp coefs
	if (mod->m_dspCoefs)
		EQUA_UpdateDspCoefs(mod);
}
*/

//Replaced with generic STA_SetFilterGains()
/*
//sets new gains, AND UPDATES the DSP coefs
//Gains: array must contains all the Gains, including for the bands not to be updated.
void EQUA_SetGains(tModule* mod, u32 bandmask, const s16* Gains)
{
	tBiquad* 	    		 const m_bands = mod->m_pFilters;
	volatile T_Biquad_Coefs* const m_coefs = ((T_Eq_Nbands_Coefs*)mod->m_dspCoefs)->coefs;

	bool updateDsp = (mod->m_mode != STA_EQ_OFF) && DSPCOEFS(mod);
	int i;

	FORi(mod->m_nfilters) {
		if (bandmask & (1<<i)) {
			BIQ_SetGain(&m_bands[i], Gains[i]);
			BIQ_UpdateCoefs(&m_bands[i]);
			mod->m_dirtyDspCoefs |= 1 << i;
			if (updateDsp)
				BIQ_UpdateDspCoefs(&m_bands[i], &m_coefs[i]);
		}
	}

	if (updateDsp)
		mod->m_dirtyDspCoefs = 0;
}
*/

//DP EQ ONLY
static void EQUA_UpdateDspCoefs(tModule* mod)
{
	if (!mod) {goto _ret;}{

//	tEqualizer* 					const      eq = mod->m_sub;
	tBiquad* 	      				const m_bands = mod->m_pFilters;
	volatile T_Eq_3bands_Coefs*		const dspCoef = mod->m_pDspCoefBack;
//	volatile T_Eq_3bands_DP_Data*	const dspData = (T_Eq_3bands_DP_Data*)mod->m_dspData;
	u32 i;


	//update dsp coefs
	sta_assert(dspCoef);

	//EQ OFF
	if (mod->m_mode == (u32)STA_EQ_OFF)
	{
		FORi(mod->m_nfilters) {BIQ_SetDspCoefs2bypass(&dspCoef->coefs[i]);}
		mod->m_dirtyDspCoefs = 0xFFFFFFFF;
	}
	//EQ ON
	else
	{
		FORi(mod->m_nfilters)
		{
			//update biquad coefs if dirty
			if (m_bands[i].m_dirtyBiquad) {
				BIQ_UpdateCoefs(&m_bands[i]);
				mod->m_dirtyDspCoefs |= 1u << i;
			}

			//update dsp coefs if dirty
			//note: because of the dual coef buffer, we need to update all the coefs each time.
//			if (mod->m_dirtyDspCoefs & (1<<i))
			{
				BIQ_UpdateDspCoefs(&m_bands[i], &dspCoef->coefs[i]);
			}
		}
		mod->m_dirtyDspCoefs = 0;
	}

	MOD_SwapCoefBuffers(mod);

	}
_ret: return;
}

//===========================================================================
// EQUALIZER SP
//===========================================================================

static void EQUA_UpdateDspCoefs_sp(tModule* mod)
{
	if (!mod) {goto _ret;}{

	tEqualizer* 					const      eq = mod->m_sub;
	tBiquad* 	      				const m_bands = mod->m_pFilters;
	volatile T_Eq_3bands_SP_Coefs*	const dspCoef = mod->m_pDspCoefBack;
	u32 i;

	//update dsp coefs
	sta_assert(dspCoef);
	sta_assert(eq->m_simplePrec);
	(void)(u32)eq; //not used outside of sta_assert

	//EQ OFF
	if (mod->m_mode == (u32)STA_EQ_OFF)
	{
		FORi(mod->m_nfilters) {BIQ_SetDspCoefs2bypass_sp(&dspCoef->coefs[i]);}
		mod->m_dirtyDspCoefs = 0xFFFFFFFF;
	}
	//EQ ON
	else
	{
		FORi(mod->m_nfilters)
		{
			//update biquad coefs if dirty
			if (m_bands[i].m_dirtyBiquad) {
				BIQ_UpdateCoefs(&m_bands[i]);
				mod->m_dirtyDspCoefs |= 1u << i;
			}

			//update dsp coefs if dirty
			//note: because of the dual coef buffer, we need to update all the coefs each time.
//			if (mod->m_dirtyDspCoefs & (1<<i))
			BIQ_UpdateDspCoefs_sp(&m_bands[i], &dspCoef->coefs[i]);
		}
		mod->m_dirtyDspCoefs = 0;
	}

	MOD_SwapCoefBuffers(mod);

	}
_ret: return;
}


//===========================================================================
// COMPANDER
//===========================================================================
static void COMP_InitDspData(tModule* mod);
static void COMP_UpdateDspCoefs(tModule* mod);

static void COMP_Init(tModule* mod)
{
	if (!mod) {goto _ret;}
	mod->InitDspData    = &COMP_InitDspData;
	mod->UpdateDspCoefs = &COMP_UpdateDspCoefs;
	mod->m_mode			= (u32)STA_COMP_OFF;

_ret: return;
}

static void COMP_InitDspData(tModule* mod)
{
	if (!mod) {goto _ret;}{

	volatile T_CompanderCoeffs*        const coef = mod->m_dspCoefs;		//ARM addr
	volatile T_CompanderGenericData*   const generic_data = mod->m_dspData; //ARM addr
	tDspPtr  xdata = (tDspPtr)mod->m_xdata; //DSP addr
	tDspPtr  ycoef = (tDspPtr)mod->m_ycoef; //DSP addr
	u32 i;

	sta_assert(mod->m_dspData && mod->m_dspCoefs);

	//init XMEM data and YMEM coef
	//XMEM layout: T_CompanderXXXData | in[nin] | out[nout]
	//YMEM layout: T_CompanderCoeffs | dlay[nin*COMP_DLAY_SIZE]

	ycoef += WSIZEOF(T_CompanderCoeffs);

	switch (mod->m_type) {
	case STA_COMP_1CH:	{
		volatile T_CompanderMonoData* const data = mod->m_dspData;	//ARM addr
		xdata += WSIZEOF(T_CompanderMonoData);			//DSP addr
		DSP_WRITE(&data->din, 			(frac*)(xdata));
		DSP_WRITE(&data->dout, 			(frac*)(xdata + 1));
		DSP_WRITE(&data->delay_buffer,	(frac*)(ycoef));
		DSP_WRITE(&data->delay_ptr,		(frac*)(ycoef));
		DSP_WRITE(&data->delay_buf_end,	(frac*)(ycoef + COMP_DLAY_SIZE));
		break; }

	case STA_COMP_2CH: {
		volatile T_CompanderStereoData* const data = mod->m_dspData;//ARM addr
		xdata += WSIZEOF(T_CompanderStereoData);			 //DSP addr
		DSP_WRITE(&data->din[0],		(frac*)(xdata));
		DSP_WRITE(&data->din[1],		(frac*)(xdata + 1));
		DSP_WRITE(&data->dout[0],		(frac*)(xdata + 2));
		DSP_WRITE(&data->dout[1],		(frac*)(xdata + 3));
		DSP_WRITE(&data->delay_buffer,	(frac*)(ycoef));
		DSP_WRITE(&data->delay_ptr,		(frac*)(ycoef));
		DSP_WRITE(&data->delay_buf_end,	(frac*)(ycoef + 2*COMP_DLAY_SIZE));
		break; }

	case STA_COMP_4CH: {
		volatile T_CompanderQuadroData* const data = mod->m_dspData;//ARM addr
		xdata += WSIZEOF(T_CompanderQuadroData);			 //DSP addr
		FORi(4) {
			DSP_WRITE(&data->din[i],	(frac*)(xdata + i));
			DSP_WRITE(&data->dout[i],	(frac*)(xdata + i + 4));
		}
		DSP_WRITE(&data->delay_buffer,	(frac*)(ycoef));
		DSP_WRITE(&data->delay_ptr,		(frac*)(ycoef));
		DSP_WRITE(&data->delay_buf_end,	(frac*)(ycoef + 4*COMP_DLAY_SIZE));
		break; }

	case STA_COMP_6CH: {
		volatile T_Compander6ChData* const data = mod->m_dspData;	//ARM addr
		xdata += WSIZEOF(T_Compander6ChData);				//DSP addr
		FORi(6) {
			DSP_WRITE(&data->din[i],	(frac*)(xdata + i));
			DSP_WRITE(&data->dout[i],	(frac*)(xdata + i + 6));
		}
		DSP_WRITE(&data->delay_buffer,	(frac*)(ycoef));
		DSP_WRITE(&data->delay_ptr,		(frac*)(ycoef));
		DSP_WRITE(&data->delay_buf_end,	(frac*)(ycoef + 6*COMP_DLAY_SIZE));
		break; }

	default: sta_assert(0); break;
	}


	//Common XMEM and YMEM inits (same as in dsp audio_compander_init_rom())

	//XMEM generic_data
	DSP_WRITE(&generic_data->mean_sig, 				0);
    DSP_WRITE(&generic_data->g_1, 					FRAC( 0.500));
    DSP_WRITE(&generic_data->control_value, 		0);
    DSP_WRITE(&generic_data->control_level, 		0);
    #ifdef COMPANDER_VER_1
    DSP_WRITE(&generic_data->input_level, 			0);
    DSP_WRITE(&generic_data->t_current, 			FRAC( 0.05));	// 0.9 ms
    #else
    DSP_WRITE(&generic_data->t_memory, 				FRAC( 0.95));	// 0.9 ms
    #endif

	//YMEM standard compander coefficients
	DSP_WRITE(&coef->base_coeffs.on_off, 			(s32)mod->m_mode); // 0x000000 => OFF bypass mode
	DSP_WRITE(&coef->base_coeffs.mean_max, 			0);				// 0x000000 => MEAN method
	DSP_WRITE(&coef->base_coeffs.t_av, 				FRAC( 0.004));	// 12 ms
    #ifdef COMPANDER_VER_1
	DSP_WRITE(&coef->base_coeffs.attenuation, 		FRAC( 0.0625));	// 12 dB
    #else
	DSP_WRITE(&coef->base_coeffs.gain, 				FRAC(-0.0625));	// -12 dB
    #endif

	DSP_WRITE(&coef->compression_coeffs.threshold, 	FRAC(-0.21875));// -42 dB
	DSP_WRITE(&coef->compression_coeffs.slope, 		FRAC(-0.5));    // -0.5 compression slope
	DSP_WRITE(&coef->compression_coeffs.hysteresis, FRAC( 0.0026)); // 0.5 dB
    #ifdef COMPANDER_VER_1
	DSP_WRITE(&coef->compression_coeffs.t_attack, 	FRAC( 0.1));	// 0.5 ms
	DSP_WRITE(&coef->compression_coeffs.t_release, 	FRAC( 0.001));	// 47.9 ms
    #else
	DSP_WRITE(&coef->compression_coeffs.t_attack, 	FRAC( 0.9));	// 0.5 ms
	DSP_WRITE(&coef->compression_coeffs.t_release,	FRAC( 0.999));	// 47.9 ms
    #endif

	DSP_WRITE(&coef->expansion_coeffs.threshold,	FRAC(-0.375));	// -72 dB
	DSP_WRITE(&coef->expansion_coeffs.slope,		FRAC(-0.45));	// -0.45 compression slope
	DSP_WRITE(&coef->expansion_coeffs.hysteresis,	FRAC( 0.001));	// 0.2 dB
    #ifdef COMPANDER_VER_1
	DSP_WRITE(&coef->expansion_coeffs.t_attack,		FRAC( 0.15));	// 0.3 ms
	DSP_WRITE(&coef->expansion_coeffs.t_release,	FRAC( 0.05));	// 0.9 ms
    #else
	DSP_WRITE(&coef->expansion_coeffs.t_attack,		FRAC( 0.85));	// 0.3 ms
	DSP_WRITE(&coef->expansion_coeffs.t_release,	FRAC( 0.95));	// 0.9 ms
    #endif

	mod->m_dirtyDspCoefs = 0;

	//note: no init should be done by DSP !!

	}
_ret: return;
}
/*
void COMP_SetParams(tModule* mod, s16* params)
{
}
*/
static void COMP_UpdateDspCoefs(tModule* mod)
{
	if (!mod) {goto _ret;}

	//set on/off
	sta_assert(DSPCOEFS(mod));

	DSP_WRITE(&((volatile T_ComprBaseCoeffs*)mod->m_dspCoefs)->on_off, (mod->m_mode == (u32)STA_COMP_OFF) ? 0 : 1);

_ret: return;
}

//===========================================================================
// LIMITER
//===========================================================================
static void LMTR_InitDspData(tModule* mod);
static void LMTR_UpdateDspCoefs(tModule* mod);

static void LMTR_Init(tModule* mod)
{
	if (!mod) {goto _ret;}{

	tLimiter* const lim = mod->m_sub;

	mod->InitDspData    = &LMTR_InitDspData;
	mod->UpdateDspCoefs = &LMTR_UpdateDspCoefs;
	mod->m_mode			= (u32)STA_LMTR_OFF;

	lim->m_peakAtkTime  = 1; //TODO: align with coef->peak_at = FRAC( 0.10);

	}
_ret: return;
}


static void LMTR_InitDspData(tModule* mod)
{
	if (!mod) {goto _ret;}{

	volatile T_Limiter2Coeffs* const coef = mod->m_dspCoefs;		//ARM addr
	volatile T_Limiter2Data*   const generic_data = mod->m_dspData; //ARM addr
	tDspPtr  xdata = (tDspPtr)mod->m_xdata; //DSP addr
	tDspPtr  ycoef = (tDspPtr)mod->m_ycoef; //DSP addr
	u32 i;

	sta_assert(mod->m_dspData && mod->m_dspCoefs);

	//init XMEM data and YMEM coef
	//XMEM layout: T_LimiterXXXXData | in[nin] | out[nout]
	//YMEM layout: T_LimiterCoeffs | dlay[nin*LMTR_DLAY_SIZE]

	ycoef += WSIZEOF(T_Limiter2Coeffs);

	switch (mod->m_type) {
	case STA_LMTR_1CH:	{
		volatile T_Limiter2MonoData* const data = mod->m_dspData;	//ARM addr
		xdata += WSIZEOF(T_Limiter2MonoData);						//DSP addr
		DSP_WRITE(&data->din, 			(frac*)(xdata));
		DSP_WRITE(&data->dout, 			(frac*)(xdata + 1));
		DSP_WRITE(&data->delay_buffer, 	(frac*)(ycoef));
		DSP_WRITE(&data->delay_ptr, 	(frac*)(ycoef));
		DSP_WRITE(&data->delay_buf_end,	(frac*)(ycoef + LMTR_DLAY_SIZE));
		break; }
	case STA_LMTR_2CH: {
		volatile T_Limiter2StereoData* const data = mod->m_dspData; //ARM addr
		xdata += WSIZEOF(T_Limiter2StereoData);			 			//DSP addr
		DSP_WRITE(&data->din[0],		(frac*)(xdata));
		DSP_WRITE(&data->din[1],		(frac*)(xdata + 1));
		DSP_WRITE(&data->dout[0],		(frac*)(xdata + 2));
		DSP_WRITE(&data->dout[1],		(frac*)(xdata + 3));
		DSP_WRITE(&data->delay_buffer,	(frac*)(ycoef));
		DSP_WRITE(&data->delay_ptr, 	(frac*)(ycoef));
		DSP_WRITE(&data->delay_buf_end,	(frac*)(ycoef + 2*LMTR_DLAY_SIZE));
		break; }
	case STA_LMTR_4CH: {
		volatile T_Limiter2QuadroData* const data = mod->m_dspData; //ARM addr
		xdata += WSIZEOF(T_Limiter2QuadroData);						//DSP addr
		FORi(4) {
			DSP_WRITE(&data->din[i], 	(frac*)(xdata + i));
			DSP_WRITE(&data->dout[i],	(frac*)(xdata + i + 4));
		}
		DSP_WRITE(&data->delay_buffer, 	(frac*)(ycoef));
		DSP_WRITE(&data->delay_ptr, 	(frac*)(ycoef));
		DSP_WRITE(&data->delay_buf_end,	(frac*)(ycoef + 4*LMTR_DLAY_SIZE));
		break; }
	case STA_LMTR_6CH: {
		volatile T_Limiter2SixChData* const data = mod->m_dspData;	//ARM addr
		xdata += WSIZEOF(T_Limiter2SixChData);						//DSP addr
		FORi(6) {
			DSP_WRITE(&data->din[i],	(frac*)(xdata + i));
			DSP_WRITE(&data->dout[i],	(frac*)(xdata + i + 6));
		}
		DSP_WRITE(&data->delay_buffer,	(frac*)(ycoef));
		DSP_WRITE(&data->delay_ptr, 	(frac*)(ycoef));
		DSP_WRITE(&data->delay_buf_end,	(frac*)(ycoef + 6*LMTR_DLAY_SIZE));
		break; }

	default: sta_assert(0); break;
	}


	//Common XMEM and YMEM inits (same as in dsp audio_limiter_init_rom())

	//XMEM generic_data
	DSP_WRITE(&generic_data->peak_sig,		0);
    DSP_WRITE(&generic_data->g_1,			FRAC( 1.000));
    DSP_WRITE(&generic_data->control_value,	FRAC( 1.000));
    DSP_WRITE(&generic_data->control_level,	FRAC( 1.000));
    DSP_WRITE(&generic_data->t_current,		FRAC(0.05));		// 0x066666, initial value

	//YMEM standard limiter coefficients
	DSP_WRITE(&coef->on_off,				(s32)mod->m_mode);	// 0x000000 => OFF bypass mode
	DSP_WRITE(&coef->threshold,				FRAC( 0.25));	    // -12 dB,
	DSP_WRITE(&coef->inv_hysteresis,		FRAC( 0.982878873));// -0.15 dB,
	DSP_WRITE(&coef->t_attack,				FRAC( 0.05));		// 0x066666
	DSP_WRITE(&coef->t_release,				FRAC( 0.001));		// 0x0020c4
	DSP_WRITE(&coef->attenuation,			FRAC( 1.000));		// 0.0 dB, attenuation disabled
	//peak_coeffs
	DSP_WRITE(&coef->peak_at,				FRAC( 0.10));		// peak at 0x0ccccd
	DSP_WRITE(&coef->peak_rt,				FRAC( 0.999));		// peak rt 0x7fdf3b

	//old version
//	generic_data->min_control_level =  0;
//	DSP_WRITE(&coef->threshold,				FRAC(-0.0625));	// -12 dB, [dB/(32*20*log10(2))] (Limiter 1st version)
//	DSP_WRITE(&coef->hysteresis,			FRAC( 0.0026));	// 0.5 dB, [dB/(32*20*log10(2))] (Limiter 1st version)


	mod->m_dirtyDspCoefs = 0;

	//note: no init should be done by DSP !!

	}
_ret: return;
}


static void LMTR_UpdateDspCoefs(tModule* mod)
{
	if (!mod) {goto _ret;}

	sta_assert(DSPCOEFS(mod));

	//set on/off
	DSP_WRITE(&((volatile T_Limiter2Coeffs*)mod->m_dspCoefs)->on_off, (mod->m_mode == (u32)STA_LMTR_OFF) ? 0 : 1);

	//other coef are set directly at API level (STA_LimiterSetParams())

_ret: return;
}

//===========================================================================
// CLIP LIMITER
//===========================================================================
static void CLIP_InitDspData(tModule* mod);
static void CLIP_UpdateDspCoefs(tModule* mod);

static void CLIP_Init(tModule* mod)
{
	if (!mod) {goto _ret;}

	mod->InitDspData    = &CLIP_InitDspData;
	mod->UpdateDspCoefs = &CLIP_UpdateDspCoefs;
	mod->m_mode			= (u32)STA_CLIPLMTR_OFF;
	mod->m_wsizeofData  = WSIZEOF(T_ClipLimiterData) + mod->m_nin + mod->m_nout; // Nch inputs +1 for clip signal, Nch outputs
	mod->m_wsizeofCoefs = WSIZEOF(T_ClipLimiterNchParams);

_ret: return;
}

tModule* CLIP_SetNumChannels(tModule* mod, u32 nch)
{
	if (!mod) {goto _ret;}{

	//backup the module's info before the re-allocation
	tModule oldmod = *mod;

	//using a temp user info to update the sizes...
	STA_UserModuleInfo info;// = {0};
	memset(&info, 0, sizeof(STA_UserModuleInfo));
	info.m_type			= mod->m_type;
	info.m_dspType		= mod->m_dspType;
	info.m_nin			= nch + 1;   //+1 for the clip signal
	info.m_nout			= nch;
	info.m_sizeofParams = sizeof(tClipLimiter);
	info.m_wsizeofData  = WSIZEOF(T_ClipLimiterData) + 2*nch + 1;
	info.m_wsizeofCoefs = WSIZEOF(T_ClipLimiterNchParams);

	sta_assert(!mod->m_dspCoefs);				//must be called before BuildFlow()

	//free the module mem (in esram) and reallocate.
	DSP_DelModule(mod->m_dsp, mod);

	mod = DSP_AddModule2(oldmod.m_dsp, oldmod.m_type, &info, oldmod.m_id, oldmod.m_name, oldmod.m_tags);

	}
_ret: return mod;
}

static void CLIP_InitDspData(tModule* mod)
{
	if (!mod) {goto _ret;}{

	volatile T_ClipLimiterNchParams*    const coef = mod->m_dspCoefs;	//ARM addr
	volatile T_ClipLimiterData*     	const data = mod->m_dspData;
	tDspPtr  xdata = (tDspPtr)mod->m_xdata; //DSP addr
//	tDspPtr  ycoef = (tDspPtr)mod->m_ycoef; //DSP addr

	sta_assert(data && coef);


	switch (mod->m_type)
	{
	case STA_CLIPLMTR_NCH:
		xdata += WSIZEOF(T_ClipLimiterData);			 //DSP addr
		DSP_WRITE(&data->din, 	(frac*)(xdata));
		DSP_WRITE(&data->dout, 	(frac*)(xdata + mod->m_nin));
		break;

	default: sta_assert(0); break;
	}

	//ycoef += WSIZEOF(T_ClipLimiterNchParams);

	//YMEM standard limiter coefficients
	DSP_WRITE(&coef->num_ch,			mod->m_nout);	// number of channels
	DSP_WRITE(&coef->on_off,			mod->m_mode);	// 0x000000 => OFF bypass mode
	DSP_WRITE(&coef->bit_mask,			1);					// bit 1
	DSP_WRITE(&coef->polarity,			1);					// bit 1
	DSP_WRITE(&coef->prev_clip,			0);					// 0 for no clip
	DSP_WRITE(&coef->gain_goal,			FRAC( 1.000));		// linear gain = 1.0   0.0 dB, attenuation disabled
	DSP_WRITE(&coef->tc_factor,			0/*FRAC( 0.000)*/);		//
	DSP_WRITE(&coef->gain_goal_att,		0/*FRAC( 0.000)*/);
	DSP_WRITE(&coef->tc_factor_att,		FRAC( 0.990451772));// 5 ms
	DSP_WRITE(&coef->gain_goal_rel,		FRAC( 1.000));
	DSP_WRITE(&coef->tc_factor_rel,		FRAC( 0.999904064));// 500 ms
	DSP_WRITE(&coef->noise_rem_gain,	FRAC( 0.800));
	DSP_WRITE(&coef->final_count_down,	0);
	DSP_WRITE(&coef->gain,				FRAC( 1.000));		// initial linear gain for each channel is 1.0
	DSP_WRITE(&coef->inc,				0/*FRAC( 0.000)*/);		// initial inc for each channel is zero

	mod->m_dirtyDspCoefs = 0;

	//note: no init should be done by DSP !!

	}
_ret: return;
}


static void CLIP_UpdateDspCoefs(tModule* mod)
{
	if (!mod) {goto _ret;}

	sta_assert(DSPCOEFS(mod));

	//set on/off
	DSP_WRITE(&((volatile T_ClipLimiterNchParams*)mod->m_dspCoefs)->on_off,
			(s32)(mod->m_mode > (u32)STA_CLIPLMTR_FULL_ATT_MODE) ? 0 : (s32)mod->m_mode);

	//other coef are set directly at API level (STA_LimiterSetParams())

_ret: return;
}


//===========================================================================
// DELAY LINE
//===========================================================================
static void DLAY_InitDspData(tModule* mod);


static void DLAY_Init(tModule* mod)
{
	if (!mod) {goto _ret;}{

	tDelay* const dlay = mod->m_sub;
	int i;

	mod->AdjustDspMemSize = &DLAY_AdjustDspMemSize;
	mod->InitDspData      = &DLAY_InitDspData;
//	mod->UpdateDspCoefs   = DLAY_UpdateDspCoefs;

	FORi(STA_MAX_DELAY_CH) {
		dlay->m_delayLengths[i] = 1; 	//can't be 0
		dlay->m_delays[i] = 0;			//note: delays = 0 to length-1
	}

	}
_ret: return;
}

//called during BuildFLow()
void DLAY_AdjustDspMemSize(tModule* mod)
{
	if (!mod) {goto _ret;}{

	tDelay* const dlay = mod->m_sub;
	u32 nch = mod->m_nin;
	u32 i, totdelaylenths = 0;

	//sum all the m_delayLengths:
	FORi(nch) {totdelaylenths += dlay->m_delayLengths[i];}

	switch (mod->m_type) {
	case STA_DLAY_Y_2CH:
	case STA_DLAY_Y_4CH:
	case STA_DLAY_Y_6CH:
		//The delay lines are in the YMEM:
		mod->m_dsp->m_xmemPoolEstimatedSize -= mod->m_wsizeofData;
		mod->m_wsizeofData  = WSIZEOF(T_DelayData)*nch + 2*nch;
		mod->m_dsp->m_xmemPoolEstimatedSize += mod->m_wsizeofData;
		mod->m_wsizeofCoefs = WSIZEOF(T_DelayYParam)*nch + totdelaylenths;
		break;
	case STA_DLAY_X_2CH:
	case STA_DLAY_X_4CH:
	case STA_DLAY_X_6CH:
		//The delay lines are in the XMEM:
		mod->m_dsp->m_xmemPoolEstimatedSize -= mod->m_wsizeofData;
		mod->m_wsizeofData  = WSIZEOF(T_DelayData)*nch + 2*nch + totdelaylenths;
		mod->m_dsp->m_xmemPoolEstimatedSize += mod->m_wsizeofData;
		mod->m_wsizeofCoefs = WSIZEOF(T_DelayYParam)*nch;
		break;
	default: sta_assert(0); break;
	}

	}
_ret: return;
}

static void DLAY_InitDspData(tModule* mod)
{
	if (!mod) {goto _ret;}{

	tDelay*  				const dlay = mod->m_sub;
	volatile T_DelayData* 	const data = mod->m_dspData;	//ARM addr
	volatile T_DelayYParam* const coef = mod->m_dspCoefs;	//ARM addr
	tDspPtr  xdata = (tDspPtr)mod->m_xdata;					//DSP addr
	tDspPtr  ycoef = (tDspPtr)mod->m_ycoef;					//DSP addr

	u32 nin = mod->m_nin;
	u32 i, totdelaylenths = 0;

	sta_assert(data && coef);

	//sum all the m_delayLengths:
	FORi(nin) {totdelaylenths += dlay->m_delayLengths[i];}

	//init XMEM and YMEM data
	//note: we fully initialize DSP data and coefs, thus no need to use the DSP init fonctions.
	switch (mod->m_type)
	{
	case STA_DLAY_Y_2CH:
	case STA_DLAY_Y_4CH:
	case STA_DLAY_Y_6CH:
	{
		//The delay lines are in the YMEM:
		//XMEM layout: T_DelayData[nin]   | inout[nin*2]
		//YMEM layout: T_DelayYParam[nin] | delay[m_delayLengths[0]] | ... | delay[m_delayLengths[nin-1]]

		//check that the required X and Y data sizes have been adjusted
		sta_assert(mod->m_wsizeofData  == (WSIZEOF(T_DelayData)*nin + 2*nin));
		sta_assert(mod->m_wsizeofCoefs == (WSIZEOF(T_DelayYParam)*nin + totdelaylenths));

		xdata += WSIZEOF(T_DelayData)*nin;
		ycoef += WSIZEOF(T_DelayYParam)*nin;

		FORi(nin) {
			//XMEM
			DSP_WRITE(&data[i].p_in, 		(frac*)(xdata++));
			DSP_WRITE(&data[i].p_out,		(frac*)(xdata++));
			//YMEM
			DSP_WRITE(&coef[i].delayLineLength, dlay->m_delayLengths[i]);
			DSP_WRITE(&coef[i].p_begin, 	(frac*)ycoef);
			DSP_WRITE(&coef[i].p_end,		(frac*)(ycoef + dlay->m_delayLengths[i] - 1));
			DSP_WRITE(&coef[i].p_write ,	(frac*)ycoef);
			DSP_WRITE(&coef[i].readDiff,	(s32)dlay->m_delayLengths[i] - (s32)dlay->m_delays[i] - 1); //<- the only variable after buildflow
			ycoef += dlay->m_delayLengths[i];
		}
		break;
	}
	case STA_DLAY_X_2CH:
	case STA_DLAY_X_4CH:
	case STA_DLAY_X_6CH:
	{
		//The delay lines are in the XMEM:
		//XMEM layout: T_DelayData[nin]   | inout(0) | delay[m_delayLengths[0]] | ... | inout(nin-1) | delay[m_delayLengths[nin-1]]
		//YMEM layout: T_DelayXParam[nin]

		//check that the required X and Y data sizes have been adjusted
		sta_assert(mod->m_wsizeofData  == (WSIZEOF(T_DelayData)*nin + 2*nin + totdelaylenths));
		sta_assert(mod->m_wsizeofCoefs == (WSIZEOF(T_DelayYParam)*nin));

		xdata += WSIZEOF(T_DelayData)*nin;

		FORi(nin) {
			//XMEM
			DSP_WRITE(&data[i].p_in,		(frac*)xdata);
			DSP_WRITE(&data[i].p_out,		(frac*)(xdata + 1));
			//YMEM
			DSP_WRITE(&coef[i].delayLineLength,	dlay->m_delayLengths[i]);
			DSP_WRITE(&coef[i].p_begin,		(frac*)(xdata + 2));
			DSP_WRITE(&coef[i].p_end,		(frac*)(xdata + 2 + dlay->m_delayLengths[i] - 1));
			DSP_WRITE(&coef[i].p_write,		(frac*)(xdata + 2));
			DSP_WRITE(&coef[i].readDiff,	(s32)dlay->m_delayLengths[i] - (s32)dlay->m_delays[i] - 1); //<- the only variable after buildflow
			xdata += 2 + dlay->m_delayLengths[i];
		}
		break;
	}
	default: sta_assert(0); break;
	}

	(void) totdelaylenths; //avoid a lnt msg: symbol not accessed

	}
_ret: return;
}

//don't use, delay is updated directly in the API call
/*
void DLAY_UpdateDspCoefs(tModule* mod)
{
	int i;
	sta_assert(DSPCOEFS(mod));

	//set new delay
	FORi(mod->m_nin) {
		DSP_WRITE(&((T_DelayYParam*)mod->m_dspCoefs)[i].readDiff, dlay->m_delayLengths[i] - dlay->m_delays[i] - 1); //<- the only variable after buildflow
	}
}
*/
#endif /* !EARLY_AUDIO */

//===========================================================================
// SINEWAVE GENERATOR
//===========================================================================
static void SINE_InitDspData(tModule* mod);

static void SINE_Init(tModule* mod)
{
	if (!mod) {goto _ret;}{

	tSinewaveGen* const sine = mod->m_sub;

//	mod->AdjustDspMemSize = NULL;
	mod->InitDspData      = &SINE_InitDspData;
	mod->UpdateDspCoefs   = &SINE_UpdateDspCoefs;
	mod->m_mode			  = (u32)STA_SINE_OFF;

	sine->m_freq = 44000;								//init with A4 440Hz
	sine->m_gainDb[0] = sine->m_gainDb[1] = 0; 		    //0 dB;
	sine->m_gainLn[0] = sine->m_gainLn[1] = FRAC(1.0);	//linear gains = 1

	}
_ret: return;
}

static void SINE_InitDspData(tModule* mod)
{
	if (!mod) {goto _ret;}{

	volatile T_SinewaveData* 	const data = mod->m_dspData;	//ARM addr
//	volatile T_SinewaveCoeffs* 	const coef = mod->m_dspCoefs;	//ARM addr
	tDspPtr  xdata = (tDspPtr)mod->m_xdata; //DSP addr
//	tDspPtr  ycoef = (tDspPtr)mod->m_ycoef; //DSP addr

	sta_assert(mod->m_nout == 2);	//only outputs 2ch
	sta_assert(data);

	//XMEM layout: T_SinewaveData | dout[2]
	xdata += WSIZEOF(T_SinewaveData);
	DSP_WRITE(&data->dout[0], 	(frac*) xdata);
	DSP_WRITE(&data->dout[1],	(frac*)(xdata + 1));
	DSP_WRITE(&data->phase,		0);

	//YMEM layout: T_SinewaveCoeffs
	SINE_UpdateDspCoefs(mod);

	}
_ret: return;
}

void SINE_UpdateDspCoefs(tModule* mod)
{
	if (!mod) {goto _ret;}{

	tSinewaveGen*  				const sine = mod->m_sub;		//arm
	volatile T_SinewaveCoeffs* 	const coef = mod->m_dspCoefs;	//dsp
	f32 tmp;

	sta_assert(mod->m_dspCoefs);

	DSP_WRITE(&coef->out_gain[0],	sine->m_gainLn[0]);
	DSP_WRITE(&coef->out_gain[1],	sine->m_gainLn[1]);

	DSP_WRITE(&coef->phase_inc, 	FREQ2PHASEINC(sine->m_freq));

	tmp = (f32)STA_SAMPLING_FREQ / 1000 * sine->m_msec;
	DSP_WRITE(&coef->on_off,		tmp);

	}
_ret: return;
}

