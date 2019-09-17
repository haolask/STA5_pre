/***********************************************************************************/
/*!
*
*  \file      audioTypes.h
*
*  \brief     <i><b> STAudioLib internal audio types </b></i>
*
*  \details
*
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

#ifndef AUDIOTYPES_H_
#define AUDIOTYPES_H_

//allow anonymous union when compiling with ARM Gcc
#if !defined LINUX_OS && !defined WIN32 && !defined GNU_ARM_GCC
#pragma anon_unions		//lint !e975  misra2004 3.4 (allows anonymous union when compiling with ARM Gcc)
#endif


typedef T_ARM_XMEM_Interface  tAXI;
typedef T_ARM_YMEM_Interface  tAYI;


typedef struct sDriver {
	STA_ErrorCode	m_error;
	STA_ErrorCode	m_errorLast;
	u32				m_cutVersion;
	u32*			m_dev;				//for linux stuct device *dev
	u32 			m_lastModuleId;
	void 			(*m_fsyncISR)(void);
//	tList			m_chimes;			//converted ramps, loadable into any DSP
	//status/flags
	unsigned int	m_isPlaying			: 1; //Playing the current audio flow (ie one or all DSPs are running)
	unsigned int	m_trace_sta_calls	: 1;
	unsigned int	m_trace_sta_calls_b	: 1;
} tDriver;


typedef struct sDMABUS {
	u32*			m_pTx;
	u32				m_numt;
} tDMABUS;

//Only used for the g_modulesInfo[]
typedef struct sModuleInfo {
	T_AudioFuncType dspType;			//index used to fetch DSP function ptrs from ayi.pAlib_func_table or ayi.pUser_func_table
	u8				nin;				//number of inputs
	u8				nout;				//number of outputs
	union {
		u8			nfilters;			//number of filters, bands...
		u8			ninpc;				//number of input per channel for MIXER
	};
	u32				sizeofParams;		//size of ARM sub-params (in Bytes)
	u32				wsizeofData;		//size of DSP data  (in Words !), can be updated in AdjustDspMemSize()
	u32				wsizeofCoefs;		//size of DSP coefs (in Words !), can be updated in AdjustDspMemSize()
} tModuleInfo;

//base info for all the module types
/*
typedef struct sModule {
	STA_ModuleType	m_type;
	tModuleInfo		m_info;				//copy of info from g_modulesInfo, which can be adjusted (e.g. parametric Gain_2nch)
	u32				m_id;
//	u32				m_refCount;
	u32				m_refCode;			//used to check if the module really exists
	struct sDSP*	m_dsp;
	u32				m_mode;
	struct sBiquad*	m_pFilters;			//alias to m_sub.m_filters[]
	void*			m_sub;				//sub-params specific per type
	void*			m_dspCoefs;
	void*			m_dspData;
	u32				m_dirtyDspCoefs;	//dirty flags, (eg. dirty bitmaps for nbands/filters)
	STA_UserModuleInfo* m_userInfo;

	void (*InitModule)		(struct sModule* mod); //required for USR modules
	void (*AdjustDspMemSize)(struct sModule* mod); //[opt.]called by BuildFlow() before to allocate DSP mems
	void (*InitDspData) 	(struct sModule* mod); //called by BuildFlow() after m_dspCoefs and m_dspData are given an address
	void (*UpdateDspCoefs)	(struct sModule* mod); //called during dynamic update
	void*(*GetDspInAddr)	(const struct sModule* mod, u32 in);   //called by BuildFlow()
	void*(*GetDspOutAddr)	(const struct sModule* mod, u32 out);  //called by BuildFlow()
	void (*SetMode)			(struct sModule* mod, u32 mode); //[opt.] called during dynamic update

} tModule;
*/
typedef struct sModule {
	//===========
	//public info
	//===========
	// /!\ ORDER MUST MATCH typedef STA_UserModuleInfo
	STA_ModuleType	m_type;				//STA module type
	u32				m_dspType;			//DSP module type (internal T_AudioFuncType or dsp USER type)
	u32				m_nin;				//number of input channels
	u32				m_nout;				//number of output channels
	u32				m_sizeofParams;		//size of HOST params (in Bytes)
	u32				m_wsizeofData;		//size of DSP data  (XMEM) (in Words !), can be updated in AdjustDspMemSize()
	u32				m_wsizeofCoefs;		//size of DSP coefs (YMEM) (in Words !), can be updated in AdjustDspMemSize()
	void*			m_sub;				//ptr to HOST params allocated by STA_AddModule()
	volatile void*	m_dspData;			//ptr to DSP data  (XMEM) allocated by STA_BuildFlow()
	volatile void*	m_dspCoefs;			//ptr to DSP coefs (YMEM) allocated by STA_BuildFlow()
	u32				m_xdata;			//data word offset in DSP XMEM base
	u32				m_ycoef;			//coef word offset in DSP YMEM base
	u32				m_mainCycleCost;	//DSP cycles for main function
	u32				m_updateCycleCost;	//DSP cycles for update function
	void (*InitModule)		(struct sModule* mod); //required for USR modules
	void (*AdjustDspMemSize)(struct sModule* mod); //[opt.]called by BuildFlow() to adjust wsizeofData and wsizeofCoefs before to allocate DSP mems
	void (*InitDspData) 	(struct sModule* mod); //called by BuildFlow() after that dspData and dspCoefs have received an address
	void (*UpdateDspCoefs)	(struct sModule* mod); //called during dynamic upd ate
	void*(*GetDspInAddr)	(const struct sModule* mod, u32 in);   //called by BuildFlow()
	void*(*GetDspOutAddr)	(const struct sModule* mod, u32 out);  //called by BuildFlow()
	void (*SetMode)			(struct sModule* mod, u32 mode); //[opt.] called during dynamic update

	char			m_name[STA_MAX_MODULE_NAME_SIZE];
	u32				m_tags;				//tags to register the module in the target's Audio framework
	//=====================
	//private internal info
	//=====================
	u32				m_id;				//must be between 1 and 199 (because STAParser::id is 8 bit and >=200 are reserved)
//	u32				m_refCount;
	u32				m_refCode;			//used to check if the module really exists
	struct sDSP*	m_dsp;
	u32				m_mode;
	struct sBiquad*	m_pFilters;			//alias to m_sub.m_filters[]
	u32				m_updateSlot;
	u32				m_dirtyDspCoefs;	//dirty flags, (eg. dirty bitmaps for nbands/filters)

	//coefs shadowing (front/back coefs buffer)
	volatile void*	m_pDspCoefFront;	//DSP side (coefs used by the DSP)
	volatile void*	m_pDspCoefBack;		//ARM side (coefs being updated by the ARM)
	volatile u32*	m_pFuncTableYcoef;	//address in the func table of the ycoef ptr to swap

	union {
		u32			m_nfilters;			//number of filters, bands...
		u32			m_ninpc;			//number of input per channel for MIXER
	};

	//flags
	unsigned int	m_coefsShadowing	: 1;

} tModule;


//typedef STAConnector tConnection;

typedef struct sConnection {
	tModule* 		from;
	tModule*		to;
	u16				chout;		//'from' channel out
	u16				chin;		//'to' channel in
	u32				armAddr;	//arm address (in the DSP) of the transfer (set by the buildflow)
} tConnection;


typedef struct sDSP {
	STA_Dsp			m_core;
	tList			m_modules;
	tList			m_connections;
	tModule			m_xin;
	tModule			m_xout;
	volatile tAXI*	m_axi;
	volatile tAYI*	m_ayi;
	union {
	vu32* 			m_baseAddr;			//(value passed via STA_Init() in Linux)
	vu32*			m_pram;
	};
	vu32*			m_xram;
	vu32*			m_yram;
	u32				m_xmemPoolUsedSize;		 //exact size (in words) consummed in the dsp xmem pool (computed during the BuildFlow)
	u32				m_ymemPoolUsedSize;		 //exact size (in words) consummed in the dsp ymem pool (computed during the BuildFlow)
	u32				m_xmemPoolEstimatedSize; //estimated size (in words) consummed in the dsp xmem pool
	u32				m_ymemPoolEstimatedSize; //estimated size (in words) consummed in the dsp ymem pool
	u32				m_numOfUpdateSlots;	//6 by default
	u32				m_mainCycleCost;
	u32				m_updateCycleCost[STA_MAX_UPDATE_SLOTS];
	void 			(*m_dspISR)(STA_Dsp dspcore);
	//flags
//	unsigned int	m_isInitialised : 1;
	unsigned int	m_dirtyFlow		: 1;
	unsigned int	m_isFlowBuilt  	: 1;
	unsigned int	m_isRunning 	: 1; //DSP has started (should be same as m_axi->chk_dsp_running)
//	unsigned int	m_isPlaying		: 1; //DSP is playing the current flow
} tDSP;


typedef struct sMux {
	u8				m_sels[STA_MAX_MUX_OUT + (STA_MAX_MUX_OUT%4)];	//must be modulo 4 bytes
} tMux;

/*
typedef struct sCDDeemph {
} tCDDeemph;
*/

//Bit Shifter params for Nch (2 by default)
typedef struct sBitShift {
	volatile s32*	m_leftShifts;	//array of leftShifts in DSP mem (ptr set during BuildFlow())
} tBitShift;

//Peak Detector params for Nch (2 by default)
typedef struct sPeakDetect {
	volatile s32*	m_peaks;		//array of Peak values in DSP mem (ptr set during BuildFlow())
} tPeakDetect;

//Static Gain params for Nch (2 by default)
typedef struct sGain {
	s32				m_initGain;		//init linear gain
	s32				m_initScale;	//init scale
	volatile s32*	m_gains;		//array of linear gains in DSP mem (ptr set during BuildFlow())
	volatile s32*	m_scales;		//array of scales in DSP mem       (ptr set during BuildFlow())
} tGain;


//Smooth Gain params for 1ch
typedef struct sSmoothGain {
	s32				m_goalDb;		//target user input gain (dB * 10)
	s32				m_goalLn;		//target linear gain (Fraction)
	s32				m_lshift;		//lshifts to rescale m_goalLn so to stay below 1.0
	u32				m_tup;			//msec
	u32				m_tdown;		//msec
	f32				m_kup;			//up   coef for ramp
	f32				m_kdown;		//down coef for ramp
	u32				m_maxGain;		//allowed max gain (in dB)
	s8				m_polarity;		//+/- 1
} tSmoothGain;


//Biquad params for 1 ch
//Hold the params and coef in float or fixed, before conversion to DSP's fraction (1.23 bits)
typedef struct sBiquad {
	STA_FilterType	m_type;
	void 			(*m_pUpdateCoefs)(struct sBiquad* bq);
	//input params
	f32 			m_G;		//(Gain * BGS) dB
	f32 			m_g;		//Linear gain = 10^(G/20.0)
	f32 			m_q;		//Quality factor [0.5:6]
	f32 			m_fc;		//Cut off freq [Hz]
	f32 			m_fs;		//Sampling freq [Hz] (ex. 45600)
	f32				m_fn;		//Normalised freq = fc/fs
	f32				m_K;		//tmp value = 1/tan(fn * PI)
	f32				m_cosw0;	//= cos(2*PI*fn)
	f32				m_sinw0;	//= sin(2*PI*fn)
	f32				m_A;		//= sqrt(m_g)  used for peak and shelving 2nd order
	//output coefs
	f32 			m_a[3];
	f32 			m_b[3];
	u16   			m_bshift;	//for the biquad DP and SP
	u16   			m_outshift;	//only for the biquad DP
	//flags
	unsigned int	m_dirtyBiquad:1;   //set when one input param is modified
	unsigned int	m_userCoefs_q23:1; //coefs are set per user and stored in pre-scaled Q23 in m_a,m_b. no conversion function.
	unsigned int	m_simplePrec:1;
} tBiquad;


//Loudness params (for single or dual)
typedef struct sLoudness {
	STA_LoudFilterType m_bassType;
	STA_LoudFilterType m_trebType;
	tBiquad			   m_filters[2];		//bass, treble
	STALinLimiter	   m_autoBassQLevel;	//for auto mode (bass only)
	STALinLimiter	   m_autoGLevels[2];	//for auto mode (bass and treb)
//	s16 			   m_autoGainCurrent[2];
//	s16 			   m_autoGainDesired[2];
	struct sMixer*	   m_pMixer;			//mixer volume for auto mode
} tLoudness;


//Tone params (for single or dual)
typedef struct sTone {
	STA_ToneFilterType m_bassType;
	STA_ToneFilterType m_trebType;
	tBiquad			   m_filters[3];	//bass, middle, treble
	STALinLimiter	   m_autoBassQLevel;//for auto mode
} tTone;


//Equalizer (1ch)
typedef struct sEqualizer {
	tBiquad			m_bands[STA_MAX_EQ_NUM_BANDS];
	unsigned int	m_simplePrec:1;
} tEqualizer;


//Spectrum Meter (N bands x M biq per bands)
typedef struct sSpectrumMeter {
	u16				m_numBands;
	u16				m_numBiqPerBand;
	s32				m_dBscale;	  //(note: can be negative in order to get always positive dBpeaks, thus avoiding to extend the last 4 sign bits.
	u32				m_decayMsec;  //(msec) user input
	s32				m_decayFactor;//from decayMsec
	tBiquad			*m_bands;     //pointing to tBiquad[numBand][numBiq] array in mem contigously following this header.
	fraction		m_prev_xn;
} tSpectrumMeter;


typedef struct sAttenuator {
	struct sMixer*	m_pMixer;
	s16				m_attenuation;
	s16				m_limit;
	STALinLimiter	m_linLimiter;
} tAttenuator;

//lint -esym(768, sBalance::*, sFader::*, sMixer::*) (NOT a misra rule, but pclint rule) struct members not referenced

typedef struct sBalance {
	tAttenuator		m_ch[3];	//left, middle, right
} tBalance;


typedef struct sFader {
	tAttenuator		m_ch[3];	//front, middle, rear
} tFader;


//Mixer1|2 states (1ch)
typedef struct sMixer {
	s32 			m_initUpDownGain[3];			//linear gain  (not used in MIXER3)
	s32				m_outGain;						//db gain
	s32				m_outShift;
	s32				m_outMuteDepth;
//	s16				m_inGains[STA_MAX_MIXER_INS];
	s32				m_outTime;						// output ramps in 0.1 ms (MIXER2)
	s32				m_inTime[STA_MAX_MIXER_INS];	// input ramps in 0.1 ms (MIXER2)
	s16				m_volume;
	s16				m_volumeAttenuated;
	volatile void*	m_pDspCoefs;
	tModule*		m_pBase;
	tModule*		m_pLoudness;
	tAttenuator*	m_pBalance;
	tAttenuator*	m_pFader;
} tMixer;


//Mixer3 states (1ch)
typedef struct sMixer3 {
	tSmoothGain		m_inGains[STA_MAX_MIXER_INS];	//linear gains
	tSmoothGain		m_outGain;						//exp gain
} tMixer3;


typedef struct sLimiter {
	u32				m_peakAtkTime;
//	u16				m_peakRelTime;
} tLimiter;


//lint -esym(768, sClipLimiter::*, sCompander::*)  (NOT a misra rule, but pclint rule) struct members not referenced (TMP)

typedef struct sClipLimiter {
	s32				m_dummy;
} tClipLimiter;

typedef struct sCompander {
	s32				m_dummy;
} tCompander;


typedef struct sDelay {
	u32				m_delayLengths[STA_MAX_DELAY_CH];
	u32				m_delays[STA_MAX_DELAY_CH];		//in number of samples
} tDelay;


//2 out ch
typedef struct sSinewaveGen {
	u32				m_freq;
	u32				m_msec;
	s32				m_gainDb[2];	//user input gain (dB * 10)
	s32				m_gainLn[2];	//linear gain (Fraction)
} tSinewaveGen;


//1ch
typedef struct sDCO {
	u32				m_mode;			//(note: can't use mod.m_mode because need per channel)
	u32				m_fracDelay;	//x100
	s32				m_dco;			//x100
	u32				m_winThres;		//x100
	u32				m_scoreWindow;
	u32				m_alertThres;	//metering score alert thres
} tDCO;

//PCM Chime
typedef struct sPCMChime {
	u32				m_bytesizeMax;
	u32				m_bytesize;
	u32*			m_data;			//arm address to the dsp mem holding the data
} tPCMChime;

//Polyphonic Chime Generator
typedef struct sChimeGen {
	u32				m_maxNumRamps;
	bool			m_disablePostRepeatRamps;
	u32				m_lastRepeatRampDspOffset;
	u32				m_lastRampDspOffset;
//	u32				m_numRamps;
//	STA_RampParams*	m_currentChime;		//arm address to the dsp mem holding the data
} tChimeGen;

#endif /* AUDIOTYPES_H_ */
