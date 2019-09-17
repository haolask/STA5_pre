/***********************************************************************************/
/*!
*
*  \file      gain.c
*
*  \brief     <i><b> STAudioLib Gain Module </b></i>
*
*  \details   Static Gain, Smooth Gain (linear, exponential)
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


//===========================================================================
// STATIC GAIN
//===========================================================================
static void GAIN_InitDspData(tModule* mod);

void GAIN_Init(tModule* mod)
{
	if (!mod) {goto _ret;}{

	tGain* const gain = mod->m_sub;

//	mod->AdjustDspMemSize = &GAIN_AdjustDspMemSize;	//called by BuildFlow() before allocating DSP mems
	mod->InitDspData      = &GAIN_InitDspData;		//called by BuildFlow() when populating DSP mems
//	mod->UpdateDspCoefs   = &GAIN_UpdateDspCoefs;	//called by user control

	mod->m_mode			  = DB_AUTOSHIFT; //or DB_FIXEDSHIFT;

	gain->m_initGain  = FRAC(1.0); //linear gain
	gain->m_initScale = 0;

	GAIN_SetNch(mod, 2); //by default

	}
_ret: return;
}
//---------------------------------------------------------------------------
static void GAIN_InitDspData(tModule* mod)
{
	if (!mod) {goto _ret;}{

	tGain*							const gain = mod->m_sub;
	volatile T_GainData*			const data = mod->m_dspData;	//ARM addr
	volatile T_GainSimpleNchParams*	const coef = mod->m_dspCoefs;	//ARM addr
	u32  xdata = mod->m_xdata;									//DSP addr
	u32  ycoef = mod->m_ycoef;									//DSP addr

	sta_assert(data && coef);

	//init XMEM data
	//XMEM layout: T_GainData | ins[nch] | out[nch]
	xdata += WSIZEOF(T_GainData);
	DSP_WRITE(&data->ins, (frac*)(xdata));							//DSP addr
	DSP_WRITE(&data->out, (frac*)(xdata + mod->m_nin));

	//init YMEM coefs
	//YMEM layout: T_GainSimpleNchParams | gains[nch] | scales[nch]
	ycoef += WSIZEOF(T_GainSimpleNchParams);
	DSP_WRITE(&coef->numCh,		(s32)mod->m_nin);
	DSP_WRITE(&coef->initGain,	gain->m_initGain);
	DSP_WRITE(&coef->initScale,	gain->m_initScale);
	DSP_WRITE(&coef->gains,		(frac*)ycoef);						//DSP addr
	DSP_WRITE(&coef->scales,	(int*)(ycoef + mod->m_nin));		//DSP addr

	//store gains and scales arm addr
	gain->m_gains  = (vs32*)(volatile void*)coef + WSIZEOF(T_GainSimpleNchParams);
	gain->m_scales = gain->m_gains + mod->m_nin;					//ARM addr

	//note: dsp will init the gains/scales from initGain/initScale

	}
_ret: return;
}
//---------------------------------------------------------------------------
//must be called before BuildFlow()
void GAIN_SetNch(tModule* mod, u32 nch)
{
	if (!mod) {goto _ret;}
	sta_assert(!mod->m_dspCoefs);				//must be called before BuildFlow()

	nch = (nch + 1) & 0xFFFFFFFE; //must be 2n ch

	mod->m_nin  = nch;
	mod->m_nout = nch;

	mod->m_wsizeofCoefs = WSIZEOF(T_GainSimpleNchParams) + nch*2; //gains[n] + scales[n]
	mod->m_dsp->m_xmemPoolEstimatedSize -= mod->m_wsizeofData;
	mod->m_wsizeofData  = WSIZEOF(T_GainData) + nch*2;	          //in[n]+out[n]
	mod->m_dsp->m_xmemPoolEstimatedSize += mod->m_wsizeofData;

_ret: return;
}
//---------------------------------------------------------------------------
//if called before BuildFlow(), set initGains/initScales
//if called after, then updates directly gain/scale on DSP side
void GAIN_SetPositiveGain(tModule* mod, u32 ch, s32 G)
{
	if (!mod) {goto _ret;}{

	tGain* const gain = mod->m_sub;
	s32 g, shift;

//	sta_assert(-1200 <= G && G <= 240);

	//read the current shift
	if (DSPCOEFS(mod)) {
		shift = DSP_READ(&gain->m_scales[ch]);
	} else {
		shift = gain->m_initScale;
	}


	g = _STA_db2lin(G, 10, &shift, mod->m_mode|DB_CLAMP);

	//update dsp coefs
	if (DSPCOEFS(mod)) {
		sta_assert(gain->m_gains && gain->m_scales);
		sta_assert(ch < mod->m_nin);
		DSP_WRITE(&gain->m_gains[ch],  g);
		DSP_WRITE(&gain->m_scales[ch], shift);
	}
	else {
		gain->m_initGain  = g;
		gain->m_initScale = shift;
	}

	}
_ret: return;
}
//---------------------------------------------------------------------------
//if called before BuildFlow(), get initGains/initScales
//if called after, then get directly gain/scale from DSP side
s32 GAIN_GetPositiveGain(tModule* mod, u32 ch)
{
	s32 G = 0, g, shift;

	if (!mod) {goto _ret;}{

	tGain* const gain = mod->m_sub;

	//get dsp coefs
	if (DSPCOEFS(mod)) {
		sta_assert(gain->m_gains && gain->m_scales);
		sta_assert(ch < mod->m_nin);
		g     = DSP_READ(&gain->m_gains[ch]);
		shift = DSP_READ(&gain->m_scales[ch]);
	}
	else {
		g     = gain->m_initGain;
		shift = gain->m_initScale;
	}

	G = _STA_lin2db(g << shift)/10;

	}
_ret: return G;
}
//---------------------------------------------------------------------------
void GAIN_SetPositiveGains(tModule* mod, u32 chmask, s32 G)
{
	if (!mod) {goto _ret;}{

	tGain* const gain = mod->m_sub;
	u32 i;

	static s32 s_gainDb = 0;
	static s32 s_gainLn = ONE;
	static s32 s_lshift = 0;

	sta_assert(DSPCOEFS(mod));
	sta_assert(gain->m_gains && gain->m_scales);
//	sta_assert((-1200 <= G) && (G <= 240));

	//update dsp coefs
	FORMASKEDi(mod->m_nin, chmask)
	{
		if (s_lshift != (s32)DSP_READ(&gain->m_scales[i]) || s_gainDb != G) {
			s_gainDb = G;
			s_lshift = DSP_READ(&gain->m_scales[i]);
			s_gainLn = _STA_db2lin(G, 10, &s_lshift, mod->m_mode|DB_CLAMP);
		}
		DSP_WRITE(&gain->m_gains[i],  s_gainLn);
		DSP_WRITE(&gain->m_scales[i], s_lshift);
	}

	}
_ret: return;
}
//---------------------------------------------------------------------------
void GAIN_SetLinearGainsAndShifts(tModule* mod, u32 chmask, s32 linGain, s32 leftShift)
{
	if (!mod) {goto _ret;}{

	tGain* const gain = mod->m_sub;
	u32 i;

	sta_assert(DSPCOEFS(mod));
	sta_assert(gain->m_gains && gain->m_scales);

	//update dsp coefs
	FORMASKEDi(mod->m_nin, chmask) {
		DSP_WRITE(&gain->m_gains[i],  linGain);
		DSP_WRITE(&gain->m_scales[i], leftShift);
	}

	}
_ret: return;
}

//===========================================================================
// SMOOTH GAIN (EXP and LINEAR)
//===========================================================================
static void GAINS_InitDspData(tModule* mod);

void GAINS_SetRampLinear(tSmoothGain* gain, u32 msUp, u32 msDown)
{
	static u32 s_msUp = 0, s_msDown = 0;
	static f32 s_kup  = 1, s_kdown  = 1;

	if (!gain) {goto _ret;}

	if (s_msUp != msUp) {
		s_msUp = msUp;
		s_kup  = _STA_TCtoLinearFactor(msUp);
	}
	if (s_msDown != msDown) {
		s_msDown = msDown;
		s_kdown  = _STA_TCtoLinearFactor(msDown);
	}

	gain->m_tup   = msUp;
	gain->m_tdown = msDown;
	gain->m_kup   = s_kup;
	gain->m_kdown = s_kdown;

_ret: return;
}
//---------------------------------------------------------------------------
void GAINS_SetRampExponential(tSmoothGain* gain, u32 msUp, u32 msDown)
{
	static u32 s_msUp = 0, s_msDown = 0;
	static f32 s_kup  = 0, s_kdown  = 0;

	if (!gain) {goto _ret;}

	if (s_msUp != msUp) {
		s_msUp = msUp;
		s_kup  = _STA_TCtoSmoothFactor(msUp * 10, STA_GAIN_EXP_OOEPS, FALSE);
	}
	if (s_msDown != msDown) {
		s_msDown = msDown;
		s_kdown  = _STA_TCtoSmoothFactor(msDown * 10, STA_GAIN_EXP_OOEPS, FALSE);
	}

	gain->m_tup   = msUp;
	gain->m_tdown = msDown;
	gain->m_kup   = s_kup;
	gain->m_kdown = s_kdown;

_ret: return;
}
//---------------------------------------------------------------------------
void GAINS_Init(tModule* mod)
{
	if (!mod) {goto _ret;}{

	tSmoothGain* const gains = mod->m_sub;
	u32 i;

//	mod->AdjustDspMemSize = &GAINS_AdjustDspMemSize;
	mod->InitDspData      = &GAINS_InitDspData;		//called by BuildFlow() when populating DSP mems
//	mod->UpdateDspCoefs   = &GAINS_UpdateDspCoefs;	//note: can't assign because GAINS_UpdateDspCoefs has a different definition.

	mod->m_wsizeofData   = WSIZEOF(T_GainData) + 2*mod->m_nin;
	mod->m_wsizeofCoefs  = WSIZEOF(T_GainSmoothNchParams) + WSIZEOF(T_GainSmooth1chParams)*mod->m_nin;

	mod->m_mode			 = DB_FIXEDSHIFT; //or DB_AUTOSHIFT;

	FORi(mod->m_nin)
	{
#if 1
		gains[i].m_maxGain	= +24;		//dB
		gains[i].m_lshift	= 4;		//to allow up to +24dB (~ 1<<4)
#else
		gains[i].m_maxGain	= +48;		//dB
		gains[i].m_lshift	= 8;		//to allow up to +48dB (~ 1<<8)
#endif
		gains[i].m_polarity = +1;
		gains[i].m_goalDb 	= 0;		//dB gain
		gains[i].m_goalLn 	= _STA_db2lin(gains[i].m_goalDb, 10, &gains[i].m_lshift, mod->m_mode|DB_CLAMP);

		//ramps (up and down = 100 ms)
		if (mod->m_type == STA_GAIN_SMOOTH_NCH)	{
			GAINS_SetRampExponential(&gains[i], 100, 100);
		} else {
			GAINS_SetRampLinear(&gains[i], 100, 100);
		}
	}

	}
_ret: return;
}
//---------------------------------------------------------------------------
tModule* GAINS_SetNumChannels(tModule* mod, u32 nch)
{
	if (!mod) {goto _ret;}{

	//backup the module's info before the re-allocation
	tModule oldmod = *mod;

	//using a temp user info to update the sizes...
	STA_UserModuleInfo info;// = {0};
	memset(&info, 0, sizeof(STA_UserModuleInfo));
	info.m_type			= mod->m_type;
	info.m_dspType		= mod->m_dspType;
	info.m_nin			= nch;
	info.m_nout			= nch;
	info.m_sizeofParams = sizeof(tSmoothGain) * nch;
	info.m_wsizeofData  = WSIZEOF(T_GainData) + 2*nch;
	info.m_wsizeofCoefs = WSIZEOF(T_GainSmoothNchParams) + WSIZEOF(T_GainSmooth1chParams)*nch;

	sta_assert(!mod->m_dspCoefs);				//must be called before BuildFlow()

	//free the module mem (in esram) and reallocate.
	DSP_DelModule(mod->m_dsp, mod);

	mod = DSP_AddModule2(oldmod.m_dsp, oldmod.m_type, &info, oldmod.m_id, oldmod.m_name, oldmod.m_tags);

	}
_ret: return mod;
}
//---------------------------------------------------------------------------
static void GAINS_InitDspData(tModule* mod)
{
	if (!mod) {goto _ret;}{

	tSmoothGain*					const gains = mod->m_sub;
	volatile T_GainData*			const data  = mod->m_dspData;		//ARM addr
	volatile T_GainSmoothNchParams*	const coef  = mod->m_pDspCoefBack;	//ARM addr
	volatile T_GainSmooth1chParams* const pch   = (T_GainSmooth1chParams*)((u32)coef + sizeof(T_GainSmoothNchParams));//ARM addr
	u32  xdata = mod->m_xdata;									//DSP addr
	u32  ycoef = mod->m_ycoef;									//DSP addr
	u32 i;

	sta_assert(data && coef);

	//init XMEM data
	//XMEM layout: T_GainData | ins[nch] | out[nch]
	xdata += WSIZEOF(T_GainData);
	DSP_WRITE(&data->ins, (frac*)(xdata));							//DSP addr
	DSP_WRITE(&data->out, (frac*)(xdata + mod->m_nin));

	//init YMEM coefs
	//YMEM layout: T_GainSmoothNchParams | T_GainSmooth1chParams[nch]
	ycoef += WSIZEOF(T_GainSmoothNchParams);
	DSP_WRITE(&coef->numCh, (s32)mod->m_nin);
	DSP_WRITE(&coef->pch,   (T_GainSmooth1chParams*) ycoef);

	FORi(mod->m_nin){
		s32 goalLn = gains[i].m_goalLn * gains[i].m_polarity;
		DSP_WRITE(&pch[i].inc, 		 0);
		DSP_WRITE(&pch[i].incFactor, 0);
		DSP_WRITE(&pch[i].gainGoal,	 goalLn);
		DSP_WRITE(&pch[i].gain,	 	 goalLn);
		DSP_WRITE(&pch[i].lshift, 	 gains[i].m_lshift);
	}

	mod->m_dirtyDspCoefs = 0;

	}
_ret: return;
}
//---------------------------------------------------------------------------
//exp or linear, neg or positive gain.
void GAINS_UpdateDspCoefs(tModule* mod, u32 ch)
{
	//YMEM layout: T_GainSmoothNchParams | T_GainSmooth1chParams[nch]

	if (!mod) {goto _ret;}{

	tSmoothGain*  		   			const gain  = (tSmoothGain*)mod->m_sub + ch;			//arm params
//	volatile T_GainSmoothNchParams* const param = (T_GainSmoothNchParams*)mod->m_dspCoefs;
	volatile T_GainSmooth1chParams* const coef  = (T_GainSmooth1chParams*)((u32)mod->m_dspCoefs
												+ sizeof(T_GainSmoothNchParams)
												+ sizeof(T_GainSmooth1chParams) * ch);	//dsp coefs
	volatile T_GainSmoothNchParams* const coef2  = (T_GainSmoothNchParams*)((u32)mod->m_dspCoefs);
	s32 curGain, newGain, inc, newShift;
	f32 incFactor;


	sta_assert(mod->m_dspCoefs);	//(don't need to wait for dsp init)

	// tell to DSP to stop to update gain parameters
	DSP_WRITE(&coef2->sync,		1);
	// get current sample
	newGain = DSP_READ(&mod->m_dsp->m_axi->frame_counter);
	newShift = newGain;
	// if DSP is started, wait next sample
	if(DSP_READ(&(mod)->m_dsp->m_axi->isInitsDone))
	{
		// timeout of 1000 read
		inc = 1000;
		while((inc--)&&(newGain==newShift))
		{
			newShift = DSP_READ(&mod->m_dsp->m_axi->frame_counter);
		}
	}

	//get the current gain
	curGain  = (DSP_READ(&coef->gain) << 8) >> 8; 	//linear frac with sign bits extension (pre-polarized)
	newGain  = gain->m_goalLn * gain->m_polarity;	//apply polarity
	newShift = gain->m_lshift;


	//first, let's adjust the current shift from the current gain (which may have changed while smoothing down)
	if (mod->m_mode & DB_AUTOSHIFT)
	{
		s32	curShift;

		/////////////////////////////////////////////////////////
		//TODO: check impact of polarity with DB_AUTOSHIFT.
		//      For now, DB_AUTOSHIFT is not used for smooth gains
		sta_assert(gain->m_polarity > 0);

		curShift = DSP_READ(&coef->lshift);
		while (curShift > 0 && curGain <= (ONE>>1)) {
			curGain <<= 1;
			curShift--;
		}

		//align current and target gains to the biggest scaling shift
		newShift = max(curShift, gain->m_lshift);

		curGain  = (curGain << curShift) >> newShift;
		newGain  = (gain->m_goalLn << gain->m_lshift) >> newShift;
	}

	//prep the ramp coefs (in frac)
	inc	      = newGain - curGain;
	incFactor = (abs(newGain) > abs(curGain)) ? gain->m_kup : gain->m_kdown;

	//note: for LINEAR gain: inc = inc0 / incFactor, where inc0 is already in shifted frac,
	//      while k is either in float or int.
	if (mod->m_type == STA_GAIN_LINEAR_NCH)
	{
		s32 inc0 = inc;
		f32 incf = (f32)inc / incFactor; //note: incFactor is never null as we set if ms= 0 => incFactor =1
		inc  = (s32)incf;
		inc  = CLAMP(inc, (s32)0xFF800000, (s32)0x7FFFFF);

		if (inc == 0 && inc0 != 0) {
			inc = (inc0 > 0) ? +1 : -1; //set a minimal increment to always have a ramp.
		}

		incFactor = 0; //not used by linear gain
	}
	else
	{
		inc = abs(inc);
	}

	//finally, we update the dsp coefs:
	DSP_WRITE(&coef->gain,		curGain);
	DSP_WRITE(&coef->lshift,	newShift);
	DSP_WRITE(&coef->gainGoal,	newGain);
	DSP_WRITE(&coef->inc,		inc);
	DSP_WRITE(&coef->incFactor,	(s32)incFactor);

	// and tell to DSP to restart to update gain parameters
	DSP_WRITE(&coef2->sync,		0);

	mod->m_dirtyDspCoefs &= ~((u32)1<<ch);	//clear dirty

	}
_ret: return;
}

