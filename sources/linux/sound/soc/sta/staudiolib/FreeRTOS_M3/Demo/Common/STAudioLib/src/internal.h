/***********************************************************************************/
/*!
*
*  \file      internal.h
*
*  \brief     <i><b> STAudioLib internal functions </b></i>
*
*  \details   Internal functions for conversions....
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

#ifndef INTERNAL_H_
#define INTERNAL_H_


//---Internal Headers -----------------------------------------

#include "osal.h"				//OS, platform and types

#include "sta10xx_dsp.h"		//HW DSP_ symboles and macros, DSP_Core and DSP_SetIRQHandler()...
#include "sta10xx_audiomap.h"	//HW ARM offsets (DSP0, DSP1, DSP2...)

#include "dsp_mem_def.h"		//DSP offsets (AXI, AYI)
#include "audio.h"				//DSP audio types
#include "dsp_cycles_def.h"		//DSP cycles

#include "staudio.h"			//ARM STAudio API and platform types
#include "list.h"				//ARM STAudio internal list
#include "audioTypes.h"			//ARM STAudio internal audio types
#include "internal_user.h"		//ARM STAudio internal macros for user module

#if STA_NO_FLOAT
#include "mathFixed.h"			//ARM STAudio internal maths
#endif
//else, standard math.h is included as needed in .c (e.g. biquad.c)


typedef fraction				frac;	//dsp type (seen as s32 from ARM)

#ifdef WIN32
typedef u32*					tDspPtr;
#else
typedef u32						tDspPtr;
#endif


//lint -emacro(960, FLT, DBL, FRAC1000, FLT1000, FRAC2SCALE)	inhibits misra2004 10.3 "cast from int to float or float to int" (required to convert to DSP fixed point)
//lint -emacro(960, ARM2DSP, DSP2ARM) 							inhibits misra2004 11.5 "cast away const/volatile from ptr" (required to get address offset)
//lint -esym(960, FORi, FORj, FORMASKEDi)						inhibits misra2004 19.4 allow these macro definitions used for code readibility

//lint -esym(7*, FSYNC_IRQHandler)   inhibits 714:"not referenced", 759:"decl could be moved to module", 765/misra2004 8.10:"could be made static"
//lint -esym(7*, _DSP_memcpy, _DSP_memset, _STA_memcpy32, _STA_memset32)  inhibits 714:"not referenced", 759:"decl could be moved to module", 765/misra2004 8.10:"could be made static"
//lint -esym(7*, getModule)   inhibits 759:"decl could be moved to module", 765/misra2004 8.10:"could be made static"


//---- compile-time capabilities checks -----------------------------------
//(checks that STAudioLib and DSP fw are aligned)

#if (STA_MAX_EQ_NUM_BANDS != EQ_MAX_NUM_BANDS)
#error "STA_MAX_EQ_NUM_BANDS != EQ_MAX_NUM_BANDS"
#endif

#if (STA_MAX_MIXER_INS != MIXER_MAX_INS)
#error "STA_MAX_MIXER_INS != MIXER_MAX_INS"
#endif

#if (STA_MAX_MIXER_CH != MIXER_MAX_CH)
#error "STA_MAX_MIXER_CH != MIXER_MAX_CH"
#endif

#if (STA_MAX_MUX_INS != MUX_MAX_INS)
#error "STA_MAX_MUX_INS != MUX_MAX_INS"
#endif

//keep a margin of 3 sample delay for frac delay polynomial interpolation
#if (STA_MAX_DCO_DELAY != (FDELAY_MAX_LENGTH-3))
#error "STA_MAX_DCO_DELAY != (FDELAY_MAX_LENGTH-3)"
#endif

#ifdef COMPANDER_VER_1
#error "COMPANDER_VER_1 should not be used anymore !"
#endif


//--- Internal Defines -----------------------------------------

#ifndef STA_SAMPLING_FREQ
#define STA_SAMPLING_FREQ		48000 //45600
#endif

#define	STA_MODULE_REF_CODE		0xA2A2A2A2

#define STA_GAIN_EXP_OOEPS		((f32)10)

#define BGS						(STA_BIQ_GAIN_SCALE) /*internal alias*/

//--- fixed / frac -------------------------------------------

//signed 24 max ( x / ONE can be approximated by x >> ONESHIFT)
#define ONE						(((s32)1<<23)-1)
#define ONESHIFT 				23

//defined in audio.h
//#define FRAC(f)				((s32)((f) * 0x7FFFFF + 0.5f))	//fixed signed 24 bits
#define _ABS24(x)				(~(((u32)(x))|0xFF800000)+1)			//negative fixed 24 bits into a negative 32 bits
#define FLT(x)					(((x) & 0x800000) ? (f32)(((u32)(x))|0xFF000000) / 0x800000 : (f32)(x) / 0x800000)
#define DBL(x)					(((x) & 0x800000) ? (f64)(((u32)(x))|0xFF000000) / 0x800000 : (f64)(x) / 0x800000)

//#define FIXED2FRAC(x)			(((x)>>1)-((x)>>24))
#define FIXED2FRAC(x)			(((x)==FRAC_1) ? ONE : ((x)==-1) ? 0 : (x)>>1)

//scale should be <= 100
#define FRAC2SCALE(x, scale)	( (s32)((f32)(((x) & 0x800000) ? -(s32)_ABS24(x) : (x)) * (scale) + (1<<22)) >> 23 )


//convert between scaled int range +-1000 and dsp frac
#define FRAC1000(f)				( (s32)((f) == 1000 ? 0x7FFFFF : ((f) * 0x100000)/125) )
#define FLT1000(x)				( (s32)((f32)(((x) & 0x800000) ? -(s32)_ABS24(x) : (x)) * 125 + (1<<19)) >> 20 )

//#define IS_Q23(q)				(((q) <= 0x7fffff) && ((q) >= 0xff800000))
#define IS_Q23(q)				((s32)0xff800000 <= (q) && (q) <= (s32)0x7fffff)


#if STA_NO_FLOAT
#define FREQ2PHASEINC(freq)		(FIXED2FRAC(F_DIV(freq, STA_SAMPLING_FREQ * 50)))
#else
#define FREQ2PHASEINC(freq)		(FRAC((f32)(freq) / ((f32)STA_SAMPLING_FREQ * 50)))
#endif


//---- DSP MACROS --------------------------------------------------------------

#define DSP2ARM(base,  offset)	((u32*)(base)          + (u32)(offset))
#define ARM2DSP(base,  offset)	((u32*)(((u32)(offset) - (u32)(base))>>2))


//#define DSP_EnableCLCK(core)	AUDCR->AC0_CR.DSP_EnableClk |=  (1 << ((core)&0x3))
//#define DSP_DisableCLCK(core)	AUDCR->AC0_CR.DSP_EnableClk &= ~(1 << ((core)&0x3))


//---- HW MACROS --------------------------------------------------------------

#define DMA_TX_BASE				((u32*)AUDIO_DMA_TX_BASE)
#define DMA_RUN(run)			(AUDDMA->DMA_CR.BIT.DMA_RUN = (run))
#define DMA_isRUNNING			(AUDDMA->DMA_CR.BIT.DMA_RUN)
#define DMA_GATING(enable)		(AUDDMA->DMA_CR.BIT.GATING_EN = (enable))


//--- API checks --------------------------------------------------------
//lint -esym(960, CHECK_MODULE) misra2004 19.4 allow this macro definition used for code readibility

#define IS_STAMODULE(mod)  \
	((mod) && ((const tModule*)(mod))->m_refCode == STA_MODULE_REF_CODE)

#define CHECK_MODULE(module)  \
	getModule(module); \
	do {if (!IS_STAMODULE(mod)) {SetError(STA_INVALID_MODULE); goto _ret;}} while(0)

#define CHECK_LOADED(dsp) \
	do { \
		if ((dsp)->m_axi->chk_xmem_loaded != DSP_XMEM_LOADED_CODE) { \
			SetError((STA_ErrorCode)((u32)STA_DSP0_XMEM_NOT_LOADED + (u32)(dsp)->m_core)); goto _ret;} \
		if ((dsp)->m_ayi->chk_ymem_loaded != DSP_YMEM_LOADED_CODE) { \
			SetError((STA_ErrorCode)((u32)STA_DSP0_YMEM_NOT_LOADED + (u32)(dsp)->m_core)); goto _ret;} \
	} while(0)

#define CHECK_INITIALIZED(cond)		do {if (!(cond)) {SetError(STA_NOT_INITIALIZED); goto _ret;}} while(0)
#define CHECK_MODULES(cond)			do {if (!(cond)) {SetError(STA_INVALID_MODULE); goto _ret;}} while(0)
#define CHECK_TYPE(cond)			do {if (!(cond)) {SetError(STA_INVALID_MODULE_TYPE); goto _ret;}} while(0)
#define CHECK_PARAM(cond)			do {if (!(cond)) {SetError(STA_INVALID_PARAM); goto _ret;}} while(0)
#define CHECK_VALUE(cond)			do {if (!(cond)) {SetError(STA_INVALID_VALUE); goto _ret;}} while(0)
#define CHECK_SIZE(cond)			do {if (!(cond)) {SetError(STA_INVALID_SIZE); goto _ret;}} while(0)
#define CHECK_CHANNEL(cond)			do {if (!(cond)) {SetError(STA_INVALID_CHANNEL); goto _ret;}} while(0)
#define CHECK_SAME_DSP(cond)		do {if (!(cond)) {SetError(STA_NOT_ON_SAME_DSP); goto _ret;}} while(0)
//#define CHECK_DSP_STOPPED(dsp)	do {if ((dsp)->m_isRunning) {SetError(STA_DSP_STILL_RUNNING); goto _ret;}} while(0)
#define CHECK_DMA_STOPPED()			do {if (DMA_isRUNNING) 		{SetError(STA_DMA_STILL_RUNNING); goto _ret;}} while(0)
#define CHECK_STOPPED()				do {if (g_drv.m_isPlaying)	{SetError(STA_STILL_PLAYING); goto _ret;}} while(0)
#define CHECK_BEFORE_BUILT(cond)	do {if (cond) {SetError(STA_INVALID_AFTER_BUILT); goto _ret;}} while(0)
#define CHECK_BUILT(cond)			do {if (!(cond)) {SetError(STA_INVALID_BEFORE_BUILT); goto _ret;}} while(0)
#define CHECK_DSP_INITIALIZED(dsp)	do {if (!(dsp)->m_axi->isInitsDone) {SetError(STA_INVALID_BEFORE_DSP_INIT_DONE); goto _ret;}} while(0)

//-------------------------------------------------------------
//other internal defines


#define DSPCOEFS(mod)		(DSP_READ(&(mod)->m_dsp->m_axi->isInitsDone) && (mod)->m_dspCoefs)

#define UPDATE_DSPCOEFS(mod) \
	do { \
		if (DSP_READ(&(mod)->m_dsp->m_axi->isInitsDone) && (mod)->UpdateDspCoefs && (mod)->m_dspCoefs) { \
			(mod)->UpdateDspCoefs(mod);} \
	} while (0)


#define CLAMP(a, min, max)	((a)<(min) ? (min): ((a)>(max) ? (max) : (a)))

#define FORi(n)				for (i = 0; i < (n); i++)
#define FORj(n)				for (j = 0; j < (n); j++)

//lint -emacro(960, FORMASKEDi)		inhibits misra2004 14.8 "left braces expected" (allow this macro definition used for code readibility)
#define FORMASKEDi(n, mask)	for (i = 0; i < (n); i++) if ((mask) & ((u32)1 << i))

//Sizeof in Words
#define WSIZEOF(type)		(sizeof(type) >> 2)

#define MIN(a, b)			((a) > (b) ? (b) : (a))

//lint -esym(652, abs, min, max)	inhibits warning (NOT a misra rule) "symbol declared previously"
#ifndef LINUX_OS
 #define min(a, b)			((a) > (b) ? (b) : (a))
 #define max(a, b)			((a) > (b) ? (a) : (b))

 #ifndef abs
  #define abs(a)			(((a) > 0) ? (a) : -(a))
 #endif
#endif // not LINUX_OS

//--- Globals -------------------------------------------------

//(scatter these until we want to gather into g_driver)
extern tDSP							g_dsp[3];
extern tDriver 						g_drv;

extern const tModuleInfo 			g_modulesInfo[STA_MOD_LAST];

extern const u16* const 			g_eq_bands[16];

//--- Internal functions ---------------------------------------

//non-optimized implementations based on 32bit access
void _STA_memset32(volatile void* ptr, int value, unsigned int num);
void _STA_memcpy32(volatile void* dst, const void* src, unsigned int num);


void _SetError(STA_ErrorCode err, u32 line);
#define SetError(err)  (_SetError((err), __LINE__))


tModule* 	getModule(STAModule stamod);

//calculates k, y0, ymax and ymin from x/y start stop points
void 		_STA_SetLinLimiter(STALinLimiter *lin, s32 xstart, s32 xstop, s32 ystart, s32 ystop);

//calculates y = k*x +y0 + limiter defined by lin
s32 		_STA_GetInterpolatedValue(const STALinLimiter *lin, s32 x);

//_STA_db2lin flags
#define DB_NOFLAG		0x0
#define DB_CLAMP		0x1
#define DB_AUTOSHIFT	0x2
#define DB_FIXEDSHIFT	0x0

s32			_STA_db2lin(s32 db, s32 dbscale, s32 *rshift, u32 flags);

s32 		_STA_lin2db(s32 amplitude);
// return value of _STA_lin2db is in (dB/100)

s32 		_STA_TCtoSmoothFactor(u32 tc, f32 ooeps, bool alt);
//u32 		_STA_smoothFactorToTC(s32 kx, f32 ooeps, bool alt);

f32 		_STA_TCtoLinearFactor(u32 ms);

#ifndef MIXER_VER_3
void		_STA_TC2Frac(u32 tc, s32 *updown);
#endif
//s32		_STA_MS2Frac(u32 time, bool convtype);
s32			_STA_DB2Frac(s16 dB, s16 scale, bool convtype);

void 		DMAB_Reset(tDMABUS* dma);
u32 		DMAB_SetTransfers(tDMABUS* dma, u16 srcAddr, u16 dstAddr, u32 nch, u32 dmaPos, u32 ttype);
STA_ErrorCode DMAB_CheckAddress(u16 src, u16 dst, u32 numCh);
STA_ErrorCode DMAB_CheckType(u32 type);


void 		DSP_Init(tDSP* dsp, STA_Dsp core, u32 baseAddress);
void 		DSP_Clean(tDSP* dsp);
void		DSP_Load(tDSP* dsp);
void 		DSP_Load_ext(tDSP* dsp, const char *Paddr, u32 Psize, const char *Xaddr, u32 Xsize, const char *Yaddr, u32 Ysize);
int 		DSP_isLoaded(const tDSP* dsp);
void        DSP_Start_(tDSP* dsp);
void        DSP_Stop_(tDSP* dsp);
//STA_ErrorCode DSP_Play(tDSP* dsp);
tModule*	DSP_AddModule2(tDSP* dsp, STA_ModuleType type, const STA_UserModuleInfo *info, u32 id, const char* name, u32 tags);

void 		DSP_DelModule(tDSP* dsp, tModule* mod);
tConnection*DSP_AddConnection(tDSP* dsp, tModule* from, u32 chout, tModule* to, u32 chin);
tConnection*DSP_GetConnection(tDSP* dsp, tModule* from, u32 chout, tModule* to, u32 chin);
void 		DSP_ReconnectFrom(tConnection* con, tModule* newfrom, u32 newchout);
void 		DSP_ReconnectTo(tConnection* con, tModule* newto, u32 newchin);
void 		DSP_DelConnection(tDSP* dsp, tConnection* con);
STA_ErrorCode DSP_FillTables(tDSP* dsp);
tModule* 	DSP_FindModuleByID(const tDSP* dsp, u32 id);
tModule* 	DSP_FindModuleByName(const tDSP* dsp, const char* name);
STA_ErrorCode DSP_UpdateEstimatedSizes(tDSP* dsp);

void 		MOD_SetModuleInfo(tModule *mod, const tModuleInfo* info);
void 		MOD_Init( tModule* mod, STA_ModuleType type, u32 id, tDSP* dsp, const STA_UserModuleInfo *info, const char* name, u32 tags);
void*		MOD_GetDspInAddr(const tModule* mod, u32 in);
void*		MOD_GetDspOutAddr(const tModule* mod, u32 out);
STA_ErrorCode MOD_SetMode(tModule* mod, u32 mode);
void 		MOD_ComputeModuleCycles(tModule* mod);
void 		MOD_UpdateModuleAndDspCycles(tModule* mod);

void		GAIN_Init(tModule* mod);
void 		GAIN_SetNch(tModule* mod, u32 nch);
void		GAIN_SetPositiveGain(tModule* mod, u32 ch, s32 G);
s32			GAIN_GetPositiveGain(tModule* mod, u32 ch);
void 		GAIN_SetPositiveGains(tModule* mod, u32 chmask, s32 G);
void		GAIN_SetLinearGainsAndShifts(tModule* mod, u32 chmask, s32 linGain, s32 leftShift);

void		GAINS_Init(tModule* mod);
void 		GAINS_UpdateDspCoefs(tModule* mod, u32 ch);
tModule* 	GAINS_SetNumChannels(tModule* mod, u32 nch);
void 		GAINS_SetRampLinear(tSmoothGain* gain, u32 msUp, u32 msDown);
void 		GAINS_SetRampExponential(tSmoothGain* gain, u32 msUp, u32 msDown);

void 		BITSHIFT_SetNch(tModule* mod, u32 nch);
void 		BITSHIFT_SetLeftShifts(tModule* mod, u32 chmask, s32 leftShift);

void 		PEAKDETECT_SetNch(tModule* mod, u32 nch);
void 		PEAKDETECT_Reset(tModule* mod, u32 nch);

void 		BIQ_Init(tBiquad* bq, STA_FilterType type, s32 G, s32 Q, s32 fc);
void 		BIQ_Bypass(tBiquad* bq);
void		BIQ_SetFilterType(tBiquad* bq, STA_FilterType type);
void 		BIQ_SetFreq(tBiquad* bq, s32 fc, s32 fs);
s32 		BIQ_GetFreq(const tBiquad* bq);
void 		BIQ_SetGain(tBiquad* bq, s32 G);
s32 		BIQ_GetGain(const tBiquad* bq);
void 		BIQ_SetQual(tBiquad* bq, s32 Q);
s32 		BIQ_GetQual(const tBiquad* bq);
void 		BIQ_SetHalvedCoefs_q23(tBiquad* bq, const q23* coefs);
void 		BIQ_GetHalvedCoefs_q23(tBiquad* bq, q23* coefs);
void		BIQ_UpdateDspCoefs(const tBiquad* bq, volatile T_Biquad_Coefs* coefs);
void 		BIQ_UpdateDspCoefs_sp(const tBiquad* bq, volatile T_Biquad_Optim_SP_Coefs* coefs);
void		BIQ_SetDspCoefs2bypass(volatile T_Biquad_Coefs* coefs);
void 		BIQ_SetDspCoefs2bypass_sp(volatile T_Biquad_Optim_SP_Coefs* coefs);
#define 	BIQ_UpdateCoefs(bq)		((bq)->m_pUpdateCoefs(bq))

void		TONE_SetTypes(tModule* mod, STA_ToneFilterType bassType, STA_ToneFilterType trebType );
void		TONE_SetAutoBassQLevels(tModule* mod, s32 Gstart, s32 Gstop, s32 Qstart, s32 Qstop);

void		LOUD_SetTypes(tModule* mod, STA_LoudFilterType bassType, STA_LoudFilterType trebType );
void		LOUD_SetAutoBassQLevels(tModule* mod, s32 Gstart, s32 Gstop, s32 Qstart, s32 Qstop);
void		LOUD_SetAutoGLevels(tModule* mod, STA_LoudFilter filter, s32 Vstart, s32 Vstop, s32 Gmin, s32 Gmax);
//void 		LOUD_UpdateDspCoefs(tModule* mod);

void		EQUA_AdjustDspMemSize(tModule* mod);
//void 		EQUA_SetNumBands(tModule* mod, u32 numBands );
//void 		EQUA_SetGains(tModule* mod, u32 bandmask, const s16* Gains);

void 		MIXE_Init( tModule* mod );
void 		MIXE_SetVolume(tModule* mod, u32 chBits, const s16 volume);
void 		MIXE_SetBalance(tModule* mod, s32 balance);
void 		MIXE_SetInGainLinear(tModule* mod, u32 ch, u32 in, s32 Gain);
void 		MIXE_SetOutGainExponential(tModule* mod, u32 ch, s32 Gain);
#ifndef MIXER_VER_3
void 		MIXE_SetInGains(tModule* mod, u32 chBits, u32 inBits, s32 Gain);
void 		MIXE_SetChannelInGains(tModule* mod, u32 ch, const s16* Gains);
void 		MIXE_SetChannelInTS(tModule* mod, u32 ch, const u16* ts);
void 		MIXE_SetInTS(tModule* mod, u32 chBits, u32 inBits, u32 ts);
void 		MIXE_SetOutGains(tModule* mod, u32 chBits, s32 Gain);
void 		MIXE_SetInRamps(tModule* mod, u32 chBits, u32 inBits, u32 ts);
void 		MIXE_SetOutRamps(tModule* mod, u32 chBits, const u16 ts);
#endif

void 		BALN_Init(tModule* mod);

void 		DLAY_AdjustDspMemSize(tModule* mod);

void 		SINE_UpdateDspCoefs(tModule* mod);

void 		DCO_Init(tModule* mod);
void 		DCO_UpdateDspCoefs(tModule* mod);
void 		DCO_SetMode(tModule* mod, u32 chmask);
s32 		DCO_GetParam(tModule* mod, u32 ch, STA_DCOParam param);

void		PCM_Init(tModule* mod);
void 		PCM_AdjustDspMemSize(tModule* mod);
void 		PCM_LoadData_16bit(tModule* mod, const void* data, u32 bytesize);
void 		PCM_Play(tModule* mod, const STA_PCMChimeParams* params);
void 		PCM_Stop(tModule* mod);
void 		PCM_SetRepeatCount(tModule* mod, s32 repeatCount);

void 		CHIME_Init(tModule* mod);
void 		CHIME_AdjustDspMemSize(tModule* mod);
STA_ErrorCode CHIME_Play(tModule* mod, const STA_ChimeParams* params);
STA_ErrorCode CHIME_Stop(tModule* mod, STA_RampType stopRampType, u32 stopRampTime);
void		CHIME_EnablePostRepeatRamps(tModule* mod, bool enable);
void 		CHIME_SetMasterChime(tModule* mod, const tModule* masterChime);
void 		CHIME_SetRepeatCount(tModule* mod, s32 repeatCount);
s32 		CHIME_GetRepeatCount(tModule* mod);

tModule* 	CLIP_SetNumChannels(tModule* mod, u32 nch);

void 		SPECMTR_Init(tModule* mod);
tModule* 	SPECMTR_SetNumBands(tModule* mod, u32 numBands);
void 		SPECMTR_GetPeaks(const tModule* mod, s32* peaks, u32 peakFlag);
void 		SPECMTR_UpdateDspCoefs(tModule* mod);

#endif /* INTERNAL_H_ */
