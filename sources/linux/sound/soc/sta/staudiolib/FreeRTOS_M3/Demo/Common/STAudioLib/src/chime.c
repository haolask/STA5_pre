/***********************************************************************************/
/*!
*
*  \file      chime.c
*
*  \brief     <i><b> STAudioLib Chime Module </b></i>
*
*  \details   PCM chime
*
*
*  \author    Quarre Christophe
*
*  \author    (original version) Quarre Christophe
*
*  \version
*
*  \date      2016/02/19
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

#if 1

#include "internal.h"


//===========================================================================
// PCM CHIME
//===========================================================================
static void PCM_InitDspData(tModule* mod);


//load directly into the DSP mem
void PCM_LoadData_16bit(tModule* mod, const void* data, u32 bytesize)
{
	if (!mod || !data) {goto _ret;}{
	tPCMChime* const pcm = mod->m_sub;

	const    u32 *psrc = data;
	volatile u32 *pdst = pcm->m_data;
	u32 i;

	//load a multiple of 4 bytes to make a dsp word. Eventually the 3 remaining bytes are not loaded.
	bytesize &= 0xFFFFFFFC;

	sta_assert(bytesize <= pcm->m_bytesizeMax);
	sta_assert(mod->m_dspCoefs);
	sta_assert(pcm->m_data);

	//load into dsp converting from 16 to packed_12 bits: LLLL RRRR (=>RRRRLLLL) to 00LLLRRR
	for (i = 0; i < bytesize; i += 4)
	{
		u32 d = *psrc++;

		//TODO: add rounding + clamp to 0x7ff)
//		u32 L = (((d + 0x8)    <<  8) & 0xFFF000);	//note: +0x8 for the rounding
//		u32 R = (((d + 0x8000) >> 20) & 0x000FFF);	//note: +0x8000 for the rounding
//		*pdst++ = L | R;

		u32 packed = ((d << 8) & 0xFFF000) | ((d >> 20) & 0xFFF);		//ok

		DSP_WRITE(pdst++, packed);
	}

	pcm->m_bytesize = bytesize;

	}
_ret: return;
}
//----------------------------------------------------------------------------
void PCM_Init(tModule* mod)
{
	if (!mod) {goto _ret;}
	mod->AdjustDspMemSize = &PCM_AdjustDspMemSize;
	mod->InitDspData      = &PCM_InitDspData;

_ret: return;
}
//----------------------------------------------------------------------------
//called during BuildFLow()
void PCM_AdjustDspMemSize(tModule* mod)
{
	if (!mod) {goto _ret;}{
	tPCMChime* const pcm = mod->m_sub;

	switch (mod->m_type)
	{
	case STA_PCMCHIME_12BIT_Y_2CH:	//The pcm data are in the YMEM:
		mod->m_wsizeofCoefs = WSIZEOF(T_PCMChime_12bit_Params) + (pcm->m_bytesizeMax >> 2);
		break;

	case STA_PCMCHIME_12BIT_X_2CH:	//The pcm data are in the XMEM:
		mod->m_dsp->m_xmemPoolEstimatedSize -= mod->m_wsizeofData;
		mod->m_wsizeofData = WSIZEOF(T_PCMChime_12bit_Data) + (pcm->m_bytesizeMax >> 2);
		mod->m_dsp->m_xmemPoolEstimatedSize += mod->m_wsizeofData;
		break;

	default: sta_assert(0); break;
	}

	}
_ret: return;
}
//----------------------------------------------------------------------------
static void PCM_InitDspData(tModule* mod)
{
	if (!mod) {goto _ret;}{
	tPCMChime*						  const pcm  = mod->m_sub;
//	volatile T_PCMChime_12bit_Data*   const data = mod->m_dspData;	//ARM addr
//	volatile T_PCMChime_12bit_Params* const coef = mod->m_dspCoefs;	//ARM addr

	sta_assert(mod->m_dspCoefs);


	switch (mod->m_type)
	{
	case STA_PCMCHIME_12BIT_Y_2CH:	//The pcm data are in the YMEM:
		//XMEM layout: T_PCMChime_12bit_Data
		//YMEM layout: T_PCMChime_12bit_Params | pcm buffer[bytesizeMax/4]
		pcm->m_data = (u32*)((u32)mod->m_dspCoefs + sizeof(T_PCMChime_12bit_Params)); //backup the arm address
		break;

	case STA_PCMCHIME_12BIT_X_2CH:	//The pcm data are in the XMEM:
		//XMEM layout: T_PCMChime_12bit_Data | pcm buffer[bytesizeMax/4]
		//YMEM layout: T_PCMChime_12bit_Params
		pcm->m_data = (u32*)((u32)mod->m_dspData + sizeof(T_PCMChime_12bit_Data)); //backup the arm address
		break;

	default: sta_assert(0); break;
	}

	}
_ret: return;
}
//----------------------------------------------------------------------------
void PCM_Play(tModule* mod, const STA_PCMChimeParams* params)
{
	if (!mod || !params) {goto _ret;}{

	tPCMChime*						  const pcm  = mod->m_sub;
	volatile T_PCMChime_12bit_Data*   const data = mod->m_dspData;	//ARM addr
	volatile T_PCMChime_12bit_Params* const coef = mod->m_dspCoefs;	//ARM addr
	u32 ycoef = mod->m_ycoef;	//DSP offset
	u32 xdata = mod->m_xdata;	//DSP offset

	u32 leftPreMuteDuration, leftPostMuteDuration, rightPreMuteDuration, rightPostMuteDuration;
	f32 tmp;

	//convert from ms to samples
//	leftPreMuteDuration   = params->leftDelay      * ((f32)STA_SAMPLING_FREQ / 1000);
	tmp = params->leftDelay      * ((f32)STA_SAMPLING_FREQ / 1000);
	leftPreMuteDuration   = (u32)tmp;

	tmp = params->leftPostDelay  * ((f32)STA_SAMPLING_FREQ / 1000);
	leftPostMuteDuration  = (u32)tmp;

	tmp = params->rightDelay     * ((f32)STA_SAMPLING_FREQ / 1000);
	rightPreMuteDuration  = (u32)tmp;

	tmp = params->rightPostDelay * ((f32)STA_SAMPLING_FREQ / 1000);
	rightPostMuteDuration = (u32)tmp;

	//adjustements due to dsp implementation
	if (leftPostMuteDuration >= 3) {
		leftPostMuteDuration -= 3;}
	if (rightPostMuteDuration >= 3) {
		rightPostMuteDuration -= 3;}

	sta_assert(mod->m_dspCoefs);

	DSP_WRITE(&coef->playCount, 0); // mute first


	//set the 'data' and 'end' pointers
	switch (mod->m_type)
	{
	case STA_PCMCHIME_12BIT_Y_2CH:	//The pcm data are in the YMEM:
		//XMEM layout: T_PCMChime_12bit_Data
		//YMEM layout: T_PCMChime_12bit_Params | pcm buffer[bytesizeMax/4]
		ycoef += WSIZEOF(T_PCMChime_12bit_Params);
		DSP_WRITE(&data->data, (s32*) ycoef);
		DSP_WRITE(&data->end,  (s32*)(ycoef + (pcm->m_bytesize>>2) - 1));
		break;

	case STA_PCMCHIME_12BIT_X_2CH:	//The pcm data are in the XMEM:
		//XMEM layout: T_PCMChime_12bit_Data | pcm buffer[bytesizeMax/4]
		//YMEM layout: T_PCMChime_12bit_Params
		xdata += WSIZEOF(T_PCMChime_12bit_Data);
		DSP_WRITE(&data->data,  (s32*) xdata);
		DSP_WRITE(&data->end,   (s32*)(xdata + (pcm->m_bytesize>>2) - 1));
		break;

	default: sta_assert(0); break;
	}

	//set the params
	DSP_WRITE(&coef->leftPreMuteDuration,   leftPreMuteDuration);
	DSP_WRITE(&coef->leftPostMuteDuration,  leftPostMuteDuration);
	DSP_WRITE(&coef->rightPreMuteDuration,  rightPreMuteDuration);
	DSP_WRITE(&coef->rightPostMuteDuration, rightPostMuteDuration);
	DSP_WRITE(&coef->endOfCycle,			0); //reset cycle
	DSP_WRITE(&coef->playCount, 			(params->playCount > 0) ? params->playCount + 1	//count (+1 due to dsp implementation)
																	: params->playCount);	//=0:hard stop, or < 0:infinite loop

	}
_ret: return;
}
//----------------------------------------------------------------------------
void PCM_Stop(tModule* mod)
{
	if (!mod) {goto _ret;}{
	volatile T_PCMChime_12bit_Params* const coef = mod->m_dspCoefs;	//ARM addr

	sta_assert(mod->m_dspCoefs);

	DSP_WRITE(&coef->playCount, 0); //hard stop

	}
_ret: return;
}
//----------------------------------------------------------------------------
void PCM_SetRepeatCount(tModule* mod, s32 repeatCount)
{
	if (!mod) {goto _ret;}{
	volatile T_PCMChime_12bit_Params* const coef = mod->m_dspCoefs;	//ARM addr

	sta_assert(mod->m_dspCoefs);

	//assuming setting while playing
	//note: No hard stop (playCount=0). For hard stop, one must call PCM_Stop()
	//      This behavior is to mimic the behaviour of the chime.
	DSP_WRITE(&coef->playCount, (repeatCount >= 0) ? repeatCount + 1  	//last or remaining counts (+1 due to dsp implementation)
													: -1);				//infinite repeat

	}
_ret: return;
}

//===========================================================================
// POLY CHIME
//===========================================================================
static void CHIME_InitDspData(tModule* mod);
static STA_ErrorCode CHIME_ConvertRamps(const STA_RampParams* userRamps, volatile T_Ramp* dspRamps, u32 numRamps, bool stopRamp, u32* numLoadedRamps);


void CHIME_Init(tModule* mod)
{
	if (mod) {
		mod->AdjustDspMemSize = &CHIME_AdjustDspMemSize;
		mod->InitDspData      = &CHIME_InitDspData;
	}
}
//----------------------------------------------------------------------------
//called during BuildFLow()
void CHIME_AdjustDspMemSize(tModule* mod)
{
	if (mod) {

	tChimeGen* const chm = mod->m_sub;

	sta_assert(mod->m_type == STA_CHIMEGEN);

	//the chime data are stored in the YMEM:
	//YMEM layout: T_PolyChime_Params | [max_ramp_data]
	mod->m_wsizeofCoefs = WSIZEOF(T_PolyChime_Params) + chm->m_maxNumRamps * WSIZEOF(T_Ramp);
	}
}
//----------------------------------------------------------------------------
static void CHIME_InitDspData(tModule* mod)
{
	if (mod) {

//	tChimeGen*					 const chm  = mod->m_sub;
//	volatile T_PolyChime_Data*   const data = mod->m_dspData;	//ARM addr
	volatile T_PolyChime_Params* const coef = mod->m_dspCoefs;	//ARM addr
	f32 tmp;

	sta_assert(mod->m_dspCoefs);
	sta_assert(mod->m_type == STA_CHIMEGEN);

	/*
	//reset the chime generator
	DSP_WRITE(&data->pMasterParams, 0);			//not linked to a master chime
	DSP_WRITE(&data->phase,   0); 				//reset the sinewave phase
	DSP_WRITE(&coef->gain,    0); 				//reset the envelop to MUTE
	DSP_WRITE(&coef->curRamp, 0); 				//no data yet!
	*/

	//the chime data are stored in the YMEM:
	//YMEM layout: T_PolyChime_Params | [max_ramp_data]
	DSP_WRITE(&coef->ramps, (T_Ramp*) (mod->m_ycoef + WSIZEOF(T_PolyChime_Params)));

	//default stop ramp
	tmp = -0x7FFFFF / _STA_TCtoLinearFactor(100); //from 1 to mute in 100ms
	DSP_WRITE(&coef->stopRamp.type,      POLYCHIME_RAMP_LIN);
	DSP_WRITE(&coef->stopRamp.k.inc,     (s32)tmp);
	DSP_WRITE(&coef->stopRamp.target,    0);
	DSP_WRITE(&coef->stopRamp.phase_inc, 0);
/*
	STA_RampParams stopRamp;
	stopRamp.type = STA_RAMP_LIN;
	stopRamp.msec = 100;
	stopRamp.ampl = 0;
	CHIME_ConvertRamps(&stopRamp, &coef->stopRamp, 1, TRUE);
*/

	}
}
//----------------------------------------------------------------------------
static STA_ErrorCode CHIME_ConvertRamps(const STA_RampParams* userRamps, volatile T_Ramp* dspRamps, u32 numRamps, bool stopRamp, u32* numLoadedRamps)
{
	STA_ErrorCode ret = STA_NO_ERROR;
	u32 i, j = 0;
	f32 tmp;

	sta_assert(userRamps && dspRamps);
	if (!userRamps || !dspRamps) {ret = STA_INVALID_VALUE; goto _ret;}


	FORi(numRamps)
	{
		const STA_RampParams* usrRamp = userRamps + i;
		volatile	  T_Ramp* dspRamp = dspRamps  + j;

		//TODO: check the ranges
		if (usrRamp->ampl > 1000
//		||	usrRamp->msec > 10000     //max 10 sec
//		||	usrRamp->freq > 2000000	  //max 20kHz (always the case in u16)
			) {
			ret = STA_INVALID_VALUE; goto _ret;
		}

		if (usrRamp->msec == 0) {
			continue; /*skip 0 ms ramp*/
		}

		//convert target amplitude to linear gain
		DSP_WRITE(&dspRamp->target, FRAC1000(usrRamp->ampl));

		//convert freq to phase_inc
		//note: if stopRamp, don't update the phase_inc, which is updated by the DSP from the current ramp
		if (!stopRamp) {
		//	static u32  s_freq = 0;
		//	static frac s_phase_inc = 0;
			DSP_WRITE(&dspRamp->phase_inc, FREQ2PHASEINC(usrRamp->freq));
		}

		//convert ms to the ramp factor accordingly to the ramp type
		switch (usrRamp->type)
		{
		case STA_RAMP_SMP:
		{
			DSP_WRITE(&dspRamp->type,             POLYCHIME_RAMP_STATIC);
			DSP_WRITE(&dspRamp->k.sampleDuration, (u32)usrRamp->msec);
			break;
		}
		case STA_RAMP_STC:
		{
			tmp = (f32)STA_SAMPLING_FREQ / 1000 * usrRamp->msec;
			DSP_WRITE(&dspRamp->type,             POLYCHIME_RAMP_STATIC);
			DSP_WRITE(&dspRamp->k.sampleDuration, (u32)tmp);
			break;
		}
		case STA_RAMP_EXP:
		{
			DSP_WRITE(&dspRamp->type,        POLYCHIME_RAMP_EXP);
			DSP_WRITE(&dspRamp->k.incFactor, _STA_TCtoSmoothFactor(usrRamp->msec * 10, 100.0f, FALSE)); //1/eps = 1/0.01 = 100
			break;
		}
		case STA_RAMP_LIN:
		{
			//note: linear 'inc' depends on target and start: inc = (target-start) / incFactor
			s32 start = (stopRamp) ? 0x7FFFFF:  			//assume ramp from ONE
						(j == 0)   ? 0 						//assume ramp from MUTE
								   : DSP_READ(&dspRamps[j-1].target);	//ramp from previous target

			s32 inc0	  = DSP_READ(&dspRamp->target) - start;
			f32 incFactor = _STA_TCtoLinearFactor(usrRamp->msec);
			f32 incf      = (f32)inc0 / incFactor; //note: incFactor is never null as we set if ms= 0 => incFactor =1
			s32 inc       = (s32)incf;

			inc  = CLAMP(inc, (s32)0xFF800000, (s32)0x7FFFFF);
			if (inc == 0 && inc0 != 0) {
				inc = (inc0 > 0) ? +1 : -1;} //set a minimal increment to always have a ramp.

			DSP_WRITE(&dspRamp->k.inc, inc);
			DSP_WRITE(&dspRamp->type,  POLYCHIME_RAMP_LIN);
			break;
		}
		default: ret = STA_INVALID_TYPE; break;
		}

		if (ret != STA_NO_ERROR) {goto _ret;}

		j++;
	}

	if (numLoadedRamps) {
		*numLoadedRamps = j; }

_ret: return ret;
}

//----------------------------------------------------------------------------
STA_ErrorCode CHIME_Play(tModule* mod, const STA_ChimeParams* params)
{
	STA_ErrorCode ret = STA_NO_ERROR;

	sta_assert(mod && params);
	sta_assert(mod->m_dspCoefs);
	sta_assert(mod->m_type == STA_CHIMEGEN);

	if (!mod || !params) {goto _ret;} {

	tChimeGen*					 const chm  = mod->m_sub;
	volatile T_PolyChime_Data*   const data = mod->m_dspData;	//ARM addr
	volatile T_PolyChime_Params* const coef = mod->m_dspCoefs;	//ARM addr

	//YMEM layout: T_PolyChime_Params | [max_ramp_data]
	volatile T_Ramp* const dspRampsAddr = (T_Ramp*) ((u32)mod->m_dspCoefs + sizeof(T_PolyChime_Params));
			 T_Ramp* const dspRampsOffs = (T_Ramp*) ((u32)mod->m_ycoef    + WSIZEOF(T_PolyChime_Params));

	u32 numLoadedRamps, lastRepeatRampIdx;
	int i;
	frac tmp;


	//reset the chime generator
	DSP_WRITE(&data->phase,       0); //reset the sinewave phase
	DSP_WRITE(&coef->gain,        0); //reset the envelop to MUTE
	DSP_WRITE(&coef->repeatCount, 0); //reset the repeat counter
	DSP_WRITE(&coef->repeatSync,  0); //reset the repeat synch


	//convert and load the ramps
	ret = CHIME_ConvertRamps(params->ramps, dspRampsAddr, params->numRamps, FALSE, &numLoadedRamps);
	if (ret != STA_NO_ERROR) {goto _ret;}


	// clamp the lastRepeatRampIdx
	lastRepeatRampIdx = max(((u32)params->postRepeatRampIdx) - 1, (u32)0);

	// update lastRepeatRampIdx removing null ramps
	if (numLoadedRamps != params->numRamps) {
		for (i = (int)lastRepeatRampIdx; i >= 0; i--) {
			if (params->ramps[i].msec == 0) {
				lastRepeatRampIdx--;
			}
		}
	}

	chm->m_lastRepeatRampDspOffset = (u32)dspRampsOffs + WSIZEOF(T_Ramp) * lastRepeatRampIdx;
	chm->m_lastRampDspOffset       = (u32)dspRampsOffs + WSIZEOF(T_Ramp) * (numLoadedRamps - 1);

	//set the ramp pointers
	DSP_WRITE(&coef->ramps,				dspRampsOffs);
	DSP_WRITE(&coef->lastRepeatRamp,	(T_Ramp*)chm->m_lastRepeatRampDspOffset);
	DSP_WRITE(&coef->lastRamp, 			(chm->m_disablePostRepeatRamps) ? (T_Ramp*)chm->m_lastRepeatRampDspOffset : (T_Ramp*)chm->m_lastRampDspOffset);

	//do some dsp inits depending on the current ramp:
	DSP_WRITE(&coef->stopRamp.phase_inc, DSP_READ(&dspRampsAddr->phase_inc));		 //set the current freq
	DSP_WRITE(&data->sampleCount,		 DSP_READ(&dspRampsAddr->k.sampleDuration)); //set the current duration
	tmp = DSP_READ(&coef->gain);
	DSP_WRITE(&data->inc,				 DSP_READ(&dspRampsAddr->target) - tmp);	 //set the current inc (if exp ramp)

	//start now!
	CHIME_SetRepeatCount(mod, params->repeatCount);
	DSP_WRITE(&coef->curRamp,       	 dspRampsOffs);

	}
_ret: return ret;
}
//----------------------------------------------------------------------------
//TODO: Check if could simply reuse CHIME_Play(stopChime)
STA_ErrorCode CHIME_Stop(tModule* mod, STA_RampType stopRampType, u32 stopRampTime)
{
	STA_ErrorCode ret = STA_NO_ERROR;

	sta_assert(mod && mod->m_dspCoefs);
	sta_assert(mod->m_type == STA_CHIMEGEN);

	if (!mod) {goto _ret;} {

//	tChimeGen*					 const chm  = mod->m_sub;
	volatile T_PolyChime_Data*   const data = mod->m_dspData;	//ARM addr
	volatile T_PolyChime_Params* const coef = mod->m_dspCoefs;	//ARM addr

	T_Ramp* const dspStopRampsOffs = (T_Ramp*)(void*) ARM2DSP(mod->m_dsp->m_yram, &coef->stopRamp);

	STA_RampParams stopRamp;
	frac tmp;


	//stop the repeatCount
	DSP_WRITE(&coef->repeatCount, 0);

	//convert and load the stop ramp
	stopRamp.type = (u32)stopRampType;
	stopRamp.msec = stopRampTime;
	stopRamp.ampl = 0;
	ret = CHIME_ConvertRamps(&stopRamp, &coef->stopRamp, 1, TRUE, NULL);

	if (ret != STA_NO_ERROR) {goto _ret;}

	//set the ramp pointers
	DSP_WRITE(&coef->ramps,				dspStopRampsOffs);	//already set
	DSP_WRITE(&coef->lastRepeatRamp, 	dspStopRampsOffs);
	DSP_WRITE(&coef->lastRamp,       	dspStopRampsOffs);

	//do some dsp inits depending on the current ramp:
	//set the current inc (if exp ramp)
	tmp = DSP_READ(&coef->gain);
	DSP_WRITE(&data->inc,				DSP_READ(&coef->stopRamp.target) - tmp);

	//stop now!
	DSP_WRITE(&coef->curRamp,			dspStopRampsOffs);

	}
_ret: return ret;
}
//----------------------------------------------------------------------------
void CHIME_EnablePostRepeatRamps(tModule* mod, bool enable)
{
	if (mod && mod->m_dspCoefs)
	{
		tChimeGen*					 const chm  = mod->m_sub;
		volatile T_PolyChime_Params* const coef = mod->m_dspCoefs;

		chm->m_disablePostRepeatRamps = enable ? FALSE : TRUE;

		//update dsp
		DSP_WRITE(&coef->lastRamp, (chm->m_disablePostRepeatRamps) ? (void*)chm->m_lastRepeatRampDspOffset : (void*)chm->m_lastRampDspOffset);
	}
}
//----------------------------------------------------------------------------
void CHIME_SetRepeatCount(tModule* mod, s32 repeatCount)
{
	if (mod && mod->m_dspCoefs)
	{
		volatile T_PolyChime_Params* const coef = mod->m_dspCoefs;

		DSP_WRITE(&coef->repeatCount, repeatCount);
	}
}
//----------------------------------------------------------------------------
s32 CHIME_GetRepeatCount(tModule* mod)
{
	s32 ret = 0;

	if (mod && mod->m_dspCoefs)
	{
		volatile T_PolyChime_Params* const coef = mod->m_dspCoefs;

		ret = DSP_READ(&coef->repeatCount);
	}
	return ret;
}
//----------------------------------------------------------------------------
void CHIME_SetMasterChime(tModule* mod, const tModule* masterChime)
{
	u32 masterParamsDspAddr = 0;

	//get the master chime 'repeatCount' dsp address
	if (masterChime && masterChime->m_dspCoefs) {
		masterParamsDspAddr = (u32)ARM2DSP(masterChime->m_dsp->m_yram, masterChime->m_dspCoefs);
	}

	//set it to the slave chime
	if (mod && mod->m_dspCoefs) {
		DSP_WRITE(&((volatile T_PolyChime_Params*)mod->m_dspCoefs)->pMasterParams, masterParamsDspAddr);
	}

	(void)masterParamsDspAddr;
}

#endif //0




