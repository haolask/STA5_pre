/***********************************************************************************/
/*!
*  \file      mixer.c
*
*  \brief     <i><b> STAudioLib Mixer Module </b></i>
*
*  \details   STA Mixer
*
*
*  \author    Quarre Christophe
*
*  \author    (original version) Quarre Christophe
*
*  \version
*
*  \date      2016/05/10
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

//hack for emIDE
#if !defined(MIXER_VER_1) && !defined (MIXER_VER_2) && !defined (MIXER_VER_3)
#define MIXER_VER_3
#endif

static void MIXE_InitDspData(tModule* mod);
static void MIXE_ProcessVolume(tMixer *mix);
static volatile T_Mixer3_inGain* MIXE_GetInGainAddr(const tModule* mod, u32 ch, u32 in);
static volatile T_Mixer3_outGain* MIXE_GetOutGainAddr(const tModule* mod, u32 ch);


//===========================================================================
// MIXER 1 or 2
//===========================================================================
#if defined(MIXER_VER_1) || defined(MIXER_VER_2)


static void MIXE_ProcessVolume(tMixer *mix)
{
	s32 vol;
#ifdef MIXER_VER_1
	const int vpos = mix->m_pBase->m_ninpc + 1; //volume index
#else
	s32 scale;
#endif

	sta_assert(mix->m_pDspCoefs);

	//update volumeAttenuated
#ifdef MIXER_VER_1
	mix->m_volumeAttenuated = mix->m_volume;
#else
	mix->m_volumeAttenuated = mix->m_volume + mix->m_outGain;
#endif

	if (mix->m_pBalance) {
		if (mix->m_pBalance->m_attenuation > mix->m_pBalance->m_limit) //(negative values)
			mix->m_volumeAttenuated += mix->m_pBalance->m_attenuation;
		else
			mix->m_volumeAttenuated = -1200;
	}

	if (mix->m_pFader) {
		if (mix->m_pFader->m_attenuation > mix->m_pFader->m_limit) //(negative values)
			mix->m_volumeAttenuated += mix->m_pFader->m_attenuation;
		else
			mix->m_volumeAttenuated = -1200;
	}

	//set volume
#ifdef MIXER_VER_1
	vol = _STA_db2lin(mix->m_volumeAttenuated, 10, NULL, DB_CLAMP); //clamp to 0dB max

	//was .gainGoal[in] =
	switch (mix->m_pBase->m_type) {
	case STA_MIXE_2INS_NCH:	((T_Mixer2insParam*)mix->m_pDspCoefs)->gain[vpos] = vol; break;
	case STA_MIXE_3INS_NCH:	((T_Mixer3insParam*)mix->m_pDspCoefs)->gain[vpos] = vol; break;
	case STA_MIXE_4INS_NCH:	((T_Mixer4insParam*)mix->m_pDspCoefs)->gain[vpos] = vol; break;
	case STA_MIXE_5INS_NCH:	((T_Mixer5insParam*)mix->m_pDspCoefs)->gain[vpos] = vol; break;
	case STA_MIXE_6INS_NCH:	((T_Mixer6insParam*)mix->m_pDspCoefs)->gain[vpos] = vol; break;
	case STA_MIXE_7INS_NCH:	((T_Mixer7insParam*)mix->m_pDspCoefs)->gain[vpos] = vol; break;
	case STA_MIXE_8INS_NCH:	((T_Mixer8insParam*)mix->m_pDspCoefs)->gain[vpos] = vol; break;
	case STA_MIXE_9INS_NCH:	((T_Mixer9insParam*)mix->m_pDspCoefs)->gain[vpos] = vol; break;
	default: sta_assert(0); break;
	}
#else
#define MIXE_PROCESSVOLUME(type)	{						\
		volatile type* coefs = mix->m_pDspCoefs;			\
		s64 tmp;											\
		s32 local_inc;					                    \
        /* reset all increments to stop DSP convergence */  \
        coefs->volume_inc = 0;		       			        \
        /* wait end of DSP convergence */                   \
		STA_SLEEP(1); /* 1ms > 21 us */                         \
        /* now can access to current gain */                \
        /* set new gain goal */                             \
        /* compute local increments */                      \
        if (scale > coefs->shift) {                         \
            /* adapt current scale to new scale */          \
            coefs->volume >>=                               \
                scale-coefs->shift;                         \
            coefs->shift = scale;				            \
        }                                                   \
        if (scale < coefs->shift) {                         \
            /* adapt new scale to current scale */          \
            vol >>= coefs->shift-scale;                     \
            scale = coefs->shift;                           \
        }                                                   \
		mix->m_outShift = scale;	/*backup*/              \
		coefs->volumeGoal = vol;                            \
		if (mix->m_outTime > 0) {							\
			tmp = vol - coefs->volume;					    \
			tmp *= 10000;					                \
			local_inc = mix->m_outTime;						\
			local_inc *= STA_SAMPLING_FREQ;					\
			local_inc = tmp / local_inc;					\
		} else {											\
			local_inc = vol - coefs->volume;				\
		}													\
		/* apply all increments together to synchronise */	\
		/* the start of gain convergence in DSP */			\
		coefs->volume_inc = local_inc;						\
		coefs->shift = scale;								\
		}

	vol = _STA_db2lin(mix->m_volumeAttenuated, 10, &scale, DB_NOFLAG); //do not clamp to 0dB max

	switch (mix->m_pBase->m_type) {
	case STA_MIXE_2INS_NCH:	MIXE_PROCESSVOLUME(T_Mixer2insParam); break;
	case STA_MIXE_3INS_NCH:	MIXE_PROCESSVOLUME(T_Mixer3insParam); break;
	case STA_MIXE_4INS_NCH:	MIXE_PROCESSVOLUME(T_Mixer4insParam); break;
	case STA_MIXE_5INS_NCH:	MIXE_PROCESSVOLUME(T_Mixer5insParam); break;
	case STA_MIXE_6INS_NCH:	MIXE_PROCESSVOLUME(T_Mixer6insParam); break;
	case STA_MIXE_7INS_NCH:	MIXE_PROCESSVOLUME(T_Mixer7insParam); break;
	case STA_MIXE_8INS_NCH:	MIXE_PROCESSVOLUME(T_Mixer8insParam); break;
	case STA_MIXE_9INS_NCH:	MIXE_PROCESSVOLUME(T_Mixer9insParam); break;
	default: sta_assert(0); break;
	}
#endif

	if (mix->m_pLoudness && mix->m_pLoudness->m_mode == (u32)STA_LOUD_AUTO)
		LOUD_UpdateDspCoefs(mix->m_pLoudness);
}

void MIXE_SetVolume(tModule* mod, u32 chBits, const s16 volume)
{
	tMixer* const mix = mod->m_sub;
	u32 i;

	sta_assert((-1200 <= volume) && (volume <= 0));
	sta_assert(DSPCOEFS(mod));

	FORMASKEDi(mod->m_nout, chBits) {
		mix[i].m_volume = volume;		//backup volume
		MIXE_ProcessVolume(&mix[i]);
	}
}


#endif //MIXER_VER_1 || MIXER_VER_2

//===========================================================================
// MIXER 3
//===========================================================================

#ifdef MIXER_VER_3

static volatile T_Mixer3_inGain* MIXE_GetInGainAddr(const tModule* mod, u32 ch, u32 in)
{
	volatile T_Mixer3_inGain* ret = 0;

	if (!mod) {goto _ret;}

	switch (mod->m_type) {
	case STA_MIXE_2INS_NCH:	ret = &(((volatile T_Mixer3_2in1chParams*)mod->m_dspCoefs)[ch].inGains[in]); break;
	case STA_MIXE_3INS_NCH:	ret = &(((volatile T_Mixer3_3in1chParams*)mod->m_dspCoefs)[ch].inGains[in]); break;
	case STA_MIXE_4INS_NCH:	ret = &(((volatile T_Mixer3_4in1chParams*)mod->m_dspCoefs)[ch].inGains[in]); break;
	case STA_MIXE_5INS_NCH:	ret = &(((volatile T_Mixer3_5in1chParams*)mod->m_dspCoefs)[ch].inGains[in]); break;
	case STA_MIXE_6INS_NCH:	ret = &(((volatile T_Mixer3_6in1chParams*)mod->m_dspCoefs)[ch].inGains[in]); break;
	case STA_MIXE_7INS_NCH:	ret = &(((volatile T_Mixer3_7in1chParams*)mod->m_dspCoefs)[ch].inGains[in]); break;
	case STA_MIXE_8INS_NCH:	ret = &(((volatile T_Mixer3_8in1chParams*)mod->m_dspCoefs)[ch].inGains[in]); break;
	case STA_MIXE_9INS_NCH:	ret = &(((volatile T_Mixer3_9in1chParams*)mod->m_dspCoefs)[ch].inGains[in]); break;
	default: sta_assert(0); break;
	}

_ret: return ret;
}

static volatile T_Mixer3_outGain* MIXE_GetOutGainAddr(const tModule* mod, u32 ch)
{
	volatile T_Mixer3_outGain* ret = 0;

	if (!mod) {goto _ret;}

	switch (mod->m_type) {
	case STA_MIXE_2INS_NCH:	ret = &(((volatile T_Mixer3_2in1chParams*)mod->m_dspCoefs)[ch].outGain); break;
	case STA_MIXE_3INS_NCH:	ret = &(((volatile T_Mixer3_3in1chParams*)mod->m_dspCoefs)[ch].outGain); break;
	case STA_MIXE_4INS_NCH:	ret = &(((volatile T_Mixer3_4in1chParams*)mod->m_dspCoefs)[ch].outGain); break;
	case STA_MIXE_5INS_NCH:	ret = &(((volatile T_Mixer3_5in1chParams*)mod->m_dspCoefs)[ch].outGain); break;
	case STA_MIXE_6INS_NCH:	ret = &(((volatile T_Mixer3_6in1chParams*)mod->m_dspCoefs)[ch].outGain); break;
	case STA_MIXE_7INS_NCH:	ret = &(((volatile T_Mixer3_7in1chParams*)mod->m_dspCoefs)[ch].outGain); break;
	case STA_MIXE_8INS_NCH:	ret = &(((volatile T_Mixer3_8in1chParams*)mod->m_dspCoefs)[ch].outGain); break;
	case STA_MIXE_9INS_NCH:	ret = &(((volatile T_Mixer3_9in1chParams*)mod->m_dspCoefs)[ch].outGain); break;
	default: sta_assert(0); break;
	}

_ret: return ret;
}


void MIXE_Init(tModule* mod)
{
	if (!mod) {goto _ret;}{

	tMixer3* const mix = mod->m_sub; //array of mixers
	u32 ch, i;

	mod->InitDspData    = &MIXE_InitDspData;
//	mod->UpdateDspCoefs = MIXE_UpdateDspCoefs;


	for (ch = 0; ch < MIXER_MAX_CH; ch++)
	{
		//IN GAINS (linear)
		for (i = 0; i < mod->m_ninpc; i++) {
			mix[ch].m_inGains[i].m_goalDb = 0;						//db gain
			mix[ch].m_inGains[i].m_goalLn = FRAC(1.0);				//linear gain
			mix[ch].m_inGains[i].m_lshift = 0;
			GAINS_SetRampLinear(&mix[ch].m_inGains[i], 100, 100);	//up and down = 100 ms
		}

		//OUT GAIN (exponential)
		mix[ch].m_outGain.m_goalDb = 0;			//db gain
		mix[ch].m_outGain.m_goalLn = FRAC(1.0/16.0);	//linear gain
		mix[ch].m_outGain.m_lshift = 4;
		GAINS_SetRampExponential(&mix[ch].m_outGain, 100, 100);	//up and down = 100 ms
	}

	}
_ret: return;
}


static void MIXE_InitDspData(tModule* mod)
{
	if (!mod) {goto _ret;}{

	tMixer3*						const mix   = mod->m_sub;
	volatile T_Mixer3_NchData*		const data  = mod->m_dspData;		//ARM addr
	volatile T_Mixer3_2inNchParams*	const coefs = mod->m_dspCoefs;		//ARM addr
	volatile T_Mixer3_inGain*		dsp_inGain;
	volatile T_Mixer3_outGain*		dsp_outGain;

	u32 xdata = mod->m_xdata;	//DSP offset
	u32 ninpc = mod->m_ninpc;
	u32 nout  = mod->m_nout;
	u32 ch, i;

	sta_assert(mod->m_dspData && mod->m_dspCoefs);


	DSP_WRITE(&data->sync, 0);  // set mixer in normal mode, updating its parameters
	DSP_WRITE(&data->numCh, (s32)nout);

	xdata += WSIZEOF(T_Mixer3_NchData);			//inset offset

	dsp_inGain = &(coefs->chParams[0].inGains[0]);	//init with first ingain of first channel

	for (ch = 0; ch < nout; ch++)
	{
		//INSET pointer
		//xmem layout: T_Mixer3_NchData | insetL[ninpc] | insetR[ninpc]
		//the first  N/2 channels use insetL
		//the second N/2 channels use insetR
		// eg. if N=2, ins[0]  ->insetL and ins[1]  ->insetR
		//     if N=3, ins[0:1]->insetL and ins[2]  ->insetR
		//     if N=4, ins[0:1]->insetL and ins[2:3]->insetR
		//     ...
		DSP_WRITE(&data->chData[ch].inset, (frac*)(xdata + ninpc*(2*ch/nout)));

		//IN GAINS params
		for (i = 0; i < ninpc; i++) {
			DSP_WRITE(&dsp_inGain->gain,	mix[ch].m_inGains[i].m_goalLn);
			DSP_WRITE(&dsp_inGain->target,	mix[ch].m_inGains[i].m_goalLn);
			DSP_WRITE(&dsp_inGain->inc,		0);

			dsp_inGain++;	//next inGain
		}

		//OUT GAIN params
		dsp_outGain = (volatile T_Mixer3_outGain*)(volatile void*) dsp_inGain;  //(contiguous dsp mem)

		DSP_WRITE(&dsp_outGain->gain,		mix[ch].m_outGain.m_goalLn);
		DSP_WRITE(&dsp_outGain->target,		mix[ch].m_outGain.m_goalLn);
		DSP_WRITE(&dsp_outGain->lshift,		mix[ch].m_outGain.m_lshift);
		DSP_WRITE(&dsp_outGain->inc,		0);
		DSP_WRITE(&dsp_outGain->incFactor,	0);

		//next channel (contiguous dsp mem)
		dsp_outGain++;															//next channel
		dsp_inGain = (volatile T_Mixer3_inGain*)(volatile void*) dsp_outGain;	//next channel's inGain
	}

	}
_ret: return;
}


//set relative input gain (linear ramp, only negative gain)
void MIXE_SetInGainLinear(tModule* mod, u32 ch, u32 in, s32 Gain)
{
	if (!mod) {goto _ret;}{

	tMixer3*		const mix  = mod->m_sub;
	tSmoothGain*	const gain = &mix[ch].m_inGains[in];

	static s32 s_gain_db = 0;
	static s32 s_gain_ln = 0x7fffff;


	if (gain->m_goalDb == Gain) {
		goto _ret;} //nothing to do

	//Convert dB Gain to linear frac gain (< 0 dB)
	if (s_gain_db != Gain) {
		s_gain_db = Gain;
		s_gain_ln = _STA_db2lin(Gain, 10, NULL, DB_CLAMP); //clamp to 1.0
	}
	gain->m_goalDb = s_gain_db;
	gain->m_goalLn = s_gain_ln;
	gain->m_lshift = 0;


	//Update DSP now (or later in BuildFlow)
	if (mod->m_dspCoefs)
	{
		volatile T_Mixer3_inGain* dspGain = MIXE_GetInGainAddr(mod, ch, in);

		s32 curGain, inc0, inc;
		f32 incf, incFactor;

		//stop the current ramp
		DSP_WRITE(&dspGain->inc, 0);

		//prep the ramp params
		curGain   = DSP_READ(&dspGain->gain);		//read the current dsp gain (linear frac)
		inc0	  = gain->m_goalLn - curGain;
		incFactor = (inc0 > 0) ? gain->m_kup : gain->m_kdown;
		incf      = (f32)inc0 / incFactor;			//note: incFactor is never null as we set if ms= 0 => incFactor =1
		inc       = (s32)incf;
		inc       = CLAMP(inc, (s32)0xFF800000, (s32)0x7FFFFF);

		if (inc == 0 && inc0 !=0) {
			inc = (inc0 > 0) ? +1 : -1;}			//set a minimal increment to always have a ramp.

		//finaly, update the dsp coefs
		DSP_WRITE(&dspGain->gain, 	curGain);
		DSP_WRITE(&dspGain->target,	gain->m_goalLn);
		DSP_WRITE(&dspGain->inc, 	inc);			//run the ramp
	}

	}
_ret: return;
}

/*
//get relative input gain
s32 MIXE_GetInGain(const tModule* mod, u32 ch, u32 in)
{
	s32 ret = 0;
	if (!mod) {goto _ret;}
	ret = ((tMixer3*)mod->m_sub)[ch].m_inGains[in].m_goalDb;

_ret: return ret;
}
*/

//update outGain (exponential, neg or positve)
void MIXE_SetOutGainExponential(tModule* mod, u32 ch, s32 Gain)
{
	if (!mod) {goto _ret;}{

	tMixer3*	 const mix   = mod->m_sub;
	tSmoothGain* const gain  = &mix[ch].m_outGain;

	static s32 s_gain_db = 0; // 0 dB
	static s32 s_gain_ln = 0x7fffff; // linear value 1.0
	static s32 s_shift   = 4; // fixed to 4 to be able to reach +24dB = 16.0 = 1.0 << 4


	if (gain->m_goalDb == Gain) {
		goto _ret;} //nothing to do

	//Convert dB Gain to linear frac gain (which can be positive)
	if (s_gain_db != Gain) {
		s_gain_db = Gain;
		s_gain_ln = _STA_db2lin(Gain, 10, &s_shift, DB_CLAMP);
	}
	gain->m_goalDb = s_gain_db;
	gain->m_goalLn = s_gain_ln;
	gain->m_lshift = s_shift;


	//Update DSP now (or later in BuildFlow)
	if (mod->m_dspCoefs)
	{
		volatile T_Mixer3_outGain* dspGain = MIXE_GetOutGainAddr(mod, ch);
		volatile T_Mixer3_NchData*		const data  = mod->m_dspData;		//ARM addr

		s32 curGain, newGain, curShift, maxShift;
		f32 inc, incFactor;

		// tells DSP mixer to stop updating its parameters
		DSP_WRITE(&data->sync, 1);
		// get current sample
		newGain = DSP_READ(&mod->m_dsp->m_axi->frame_counter);
		curShift = newGain;
		// if DSP is started, wait next sample
		if(DSP_READ(&(mod)->m_dsp->m_axi->isInitsDone))
		{
			// timeout of 1000 read
			inc = 1000;
			while((inc--)&&(newGain==curShift)) curShift = DSP_READ(&mod->m_dsp->m_axi->frame_counter);
		}

		//get the current gain
		curGain  = DSP_READ(&dspGain->gain); 		//linear frac
		curShift = DSP_READ(&dspGain->lshift);

		//first, let's adjust the current shift from the current gain (which may have changed while smoothing down)
		while (curShift > 0 && curGain <= (ONE>>1)) {
			curGain <<= 1;
			curShift--;
		}

		//align current and target gains to the biggest scaling shift
		maxShift = max(curShift, gain->m_lshift);

		curGain  = (curGain << curShift) >> maxShift;
		newGain  = (gain->m_goalLn << gain->m_lshift) >> maxShift;

		//prep the ramp coefs (in frac)
		inc	      = newGain - curGain;
		incFactor = (inc > 0) ? gain->m_kup : gain->m_kdown;

		//finaly, update the dsp coefs:
		DSP_WRITE(&dspGain->gain,		curGain);
		DSP_WRITE(&dspGain->lshift,		maxShift);
		DSP_WRITE(&dspGain->target,		newGain);
		DSP_WRITE(&dspGain->inc,		(s32)inc);
		DSP_WRITE(&dspGain->incFactor,	(s32)incFactor);
		// and tells DSP mixer to restart to update its parameters
		DSP_WRITE(&data->sync, 0);
	}

	}
_ret: return;
}


static void MIXE_ProcessVolume(tMixer *mix)
{
	//TODO ?
	(void)(u32) mix;
}

void MIXE_SetVolume(tModule* mod, u32 chBits, const s16 volume)
{
	//TODO ?
	(void)(u32) mod; (void) chBits; (void) volume;
}

#endif //MIXER_VER_3


//===========================================================================
// MIXER BALANCE / FADER
//===========================================================================

void BALN_Init(tModule* mod)
{
	if (!mod) {goto _ret;}{

//	tBalance* const bal = mod->m_sub;
	tAttenuator* const m_ch = mod->m_sub;

	mod->InitDspData    = NULL; //BALN_InitDspData;
	mod->UpdateDspCoefs = NULL; //BALN_UpdateDSPmode;

	//as in ADR3
	m_ch[0].m_limit = m_ch[1].m_limit = m_ch[2].m_limit = -1200;				//limits to min dB
	m_ch[0].m_attenuation = m_ch[1].m_attenuation = m_ch[2].m_attenuation = 0;  //no attenuation

	//only middle channel needs linLimiter
	_STA_SetLinLimiter(&m_ch[1].m_linLimiter, -40, -200, 0, -140);

	}
_ret: return;
}


void MIXE_SetBalance(tModule* mod, s32 balance)
{
	if (!mod) {goto _ret;}{

	tAttenuator* const m_ch = mod->m_sub;

	sta_assert((mod->m_type == STA_MIXE_BALANCE) || (mod->m_type == STA_MIXE_FADER));
	sta_assert(m_ch[0].m_pMixer && m_ch[2].m_pMixer);
	sta_assert(m_ch[0].m_pMixer->m_pDspCoefs && m_ch[2].m_pMixer->m_pDspCoefs);
	sta_assert((-1200 <= balance) && (balance <= 1200));

	//left and right
	//negative balance sets attenuation to the left/rear
	//positive balance sets attenuation to the right/front
	if (balance > 0) {
		balance = -balance; //make it negative
		m_ch[0].m_attenuation = 0;				//left  unchanged
		m_ch[2].m_attenuation = (s16)balance;	//right attenuated
	} else {
		m_ch[0].m_attenuation = (s16)balance;	//left  attenuated
		m_ch[2].m_attenuation = 0;				//right unchanged
	}
	MIXE_ProcessVolume(m_ch[0].m_pMixer);
	MIXE_ProcessVolume(m_ch[2].m_pMixer);

	//middle
	if (m_ch[1].m_pMixer && m_ch[1].m_pMixer->m_pDspCoefs) {
		m_ch[1].m_attenuation = (s16)_STA_GetInterpolatedValue(&m_ch[1].m_linLimiter, balance);
		MIXE_ProcessVolume(m_ch[1].m_pMixer);
	}

	}
_ret: return;
}

/*
void MiXE_SetBalanceLimit(tModule* mod, s32 limit)
{
	tAttenuator* const m_ch = mod->m_sub;

	sta_assert((mod->m_type == STA_MIXE_BALANCE) || (mod->m_type == STA_MIXE_FADER));
//	sta_assert(m_ch[0].m_pMixer && m_ch[2].m_pMixer);
//	sta_assert(m_ch[0].m_pMixer->m_pDspCoefs && m_ch[2].m_pMixer->m_pDspCoefs);
	sta_assert((-1200 <= limit) && (limit <= 0));

	m_ch[0].m_limit = m_ch[1].m_limit = limit;
}
*/
