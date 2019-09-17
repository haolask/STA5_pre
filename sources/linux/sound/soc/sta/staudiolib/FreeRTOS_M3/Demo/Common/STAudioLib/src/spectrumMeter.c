/***********************************************************************************/
/*!
*
*  \file      spectrumMeter.c
*
*  \brief     <i><b> Spectrum Meter Module </b></i>
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
*  \date      2017/01/26
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

/*
#if !STA_NO_FLOAT
#include "math.h"  //for log10
#endif
*/

//===========================================================================
// SPECTRUM METER
//===========================================================================
static void SPECMTR_AdjustDspMemSize(tModule* mod);
static void SPECMTR_InitDspData(tModule* mod);
static void SPECMTR_InitBands(tSpectrumMeter* m);

//static const u16 g_specmtr_freqs_9[9]   = {        50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000}; //src: Ryo


static void SPECMTR_InitBands(tSpectrumMeter* m)
{
	if (!m) {goto _ret;}{

	u32 numBandsInitialized = MIN(m->m_numBands, STA_MAX_EQ_NUM_BANDS);
	const u16* freqs = g_eq_bands[numBandsInitialized-1];

	tBiquad *pBiq = m->m_bands;
	u32 i,j;

	//TODO?: use LPF for first band and HPF for last band

	//init bands up to STA_MAX_EQ_NUM_BANDS bands
	for (i = 0; i < numBandsInitialized; i++) {
		for (j = 0; j < m->m_numBiqPerBand; j++) {
			pBiq->m_simplePrec = 1;
			BIQ_Init(pBiq++, STA_BIQUAD_BP2_CB, 0, 100, freqs[i]);
		}
	}

	//bypass the remaining bands
	for (; i < m->m_numBands; i++) {
		for (j = 0; j < m->m_numBiqPerBand; j++) {
			pBiq->m_simplePrec = 1;
			BIQ_Bypass(pBiq++);
		}
	}

	}
_ret: return;
}

void SPECMTR_Init(tModule* mod)
{
	if (!mod) {goto _ret;}{

	tSpectrumMeter* const spe = mod->m_sub;

	mod->AdjustDspMemSize	= &SPECMTR_AdjustDspMemSize;
	mod->InitDspData		= &SPECMTR_InitDspData;
	mod->UpdateDspCoefs		= &SPECMTR_UpdateDspCoefs;
	mod->m_coefsShadowing	= (u32)FALSE;
	spe->m_numBands			= 9;
	spe->m_numBiqPerBand	= 1;
	spe->m_dBscale			= 10;
	spe->m_decayMsec		= 100; //msec (=> -20dB/sec)
	spe->m_decayFactor		= _STA_TCtoSmoothFactor(spe->m_decayMsec * 10, STA_GAIN_EXP_OOEPS, FALSE);
	spe->m_bands			= (tBiquad*)((u32)spe + sizeof(tSpectrumMeter));
	mod->m_pFilters			= spe->m_bands;
	mod->m_nfilters			= spe->m_numBands * spe->m_numBiqPerBand;

	SPECMTR_InitBands(spe);
//	SPECMTR_AdjustDspMemSize(mod);

	}
_ret: return;
}

//MUST be called before the BuildFlow
tModule* SPECMTR_SetNumBands(tModule* mod, u32 numBands)
{
	if (!mod) {goto _ret;}{

	//backup the module's params before the re-allocation
	tModule        oldmod = *mod;
	tSpectrumMeter oldsub = *(tSpectrumMeter*)mod->m_sub; //bak sub

	//use a temp UserModuleInfo to reallocate the module with the NEW HOST SIZE
	//note: dsp sizes will be recomputed during BuildFlow in SPECMTR_AdjustDspMemSize()
	STA_UserModuleInfo info;
	memset(&info, 0, sizeof(STA_UserModuleInfo));
	info.m_sizeofParams = sizeof(tSpectrumMeter) + numBands * oldsub.m_numBiqPerBand * sizeof(tBiquad);

	sta_assert(!mod->m_dspCoefs);	//must be called before BuildFlow()

	//delete the module host mem and reallocate it.
	DSP_DelModule(oldmod.m_dsp, mod);
	info.m_wsizeofData = numBands*(WSIZEOF(T_Biquad_Optim_SP_Chained_Data) + 3);
	mod = DSP_AddModule2(oldmod.m_dsp, oldmod.m_type, &info, oldmod.m_id, oldmod.m_name, oldmod.m_tags);

	//restore the previous settings
	if (mod) {
		tSpectrumMeter* const sub = mod->m_sub;
		void* m_sub      = mod->m_sub;
		void* m_pFilters = mod->m_pFilters;
		u32 sizeofParams = mod->m_sizeofParams;

		//restore mod (keeping the new pointers)
		*mod = oldmod;
		mod->m_sizeofParams  = sizeofParams;
		mod->m_wsizeofData   = info.m_wsizeofData;
		mod->m_sub			 = m_sub;
		mod->m_pFilters		 = m_pFilters;
		mod->m_nfilters		 = numBands * oldsub.m_numBiqPerBand;

		//restore sub (keeping the new pointers)
		*sub = oldsub;
		sub->m_numBands		 = (u16)numBands;  //overwrite the numBands
		sub->m_bands		 = m_pFilters;

		SPECMTR_InitBands(sub);
		SPECMTR_AdjustDspMemSize(mod);
	}

	}
_ret: return mod;
}

//called during BuildFLow()
static void SPECMTR_AdjustDspMemSize(tModule* mod)
{
	if (!mod) {goto _ret;}{

	tSpectrumMeter* const spe = mod->m_sub;

    mod->m_dsp->m_xmemPoolEstimatedSize -= mod->m_wsizeofData;
	mod->m_wsizeofData = spe->m_numBands * (spe->m_numBiqPerBand * WSIZEOF(T_Biquad_Optim_SP_Chained_Data) + 3);
    mod->m_dsp->m_xmemPoolEstimatedSize += mod->m_wsizeofData;
	mod->m_wsizeofCoefs= spe->m_numBands *  spe->m_numBiqPerBand * WSIZEOF(T_Biquad_Optim_SP_Coefs) + WSIZEOF(T_SpectrumMeter_Coefs);

	}
_ret: return;
}
/*
void SPECMTR_SetdBscale(tModule* mod, dBscale)
{
	if (!mod || !mod->m_pDspCoefBack) {return;}{

	volatile T_SpectrumMeter_Coefs* const coef = mod->m_pDspCoefBack;	//ARM addr

	coef->log2todBFactor = dBscale * 0xfffffcfe;
}}
*/
static void SPECMTR_InitDspData(tModule* mod)
{
	if (!mod) {goto _ret;}{

	tSpectrumMeter* const spe = mod->m_sub;
	volatile T_SpectrumMeter_Coefs* const coef = mod->m_pDspCoefBack;	//ARM addr


	sta_assert(mod->m_pDspCoefBack);

	DSP_WRITE(&coef->numBands, spe->m_numBands);
	DSP_WRITE(&coef->curBand,  0);

	mod->UpdateDspCoefs(mod);

	}
_ret: return;
}

//Note: we could call EQUA_UpdateDspCoefs_sp instead of this function...
//the only difference is that here we can use the m_dirtyDspCoefs (since not using coefs shadowing)
void SPECMTR_UpdateDspCoefs(tModule* mod)
{
	if (!mod || !mod->m_pDspCoefBack) {goto _ret;}{

	tSpectrumMeter* const spe = mod->m_sub;
	tBiquad          *m_bands = spe->m_bands;

	volatile T_SpectrumMeter_Coefs* const coef    = mod->m_pDspCoefBack;	//ARM addr
	volatile T_Biquad_Optim_SP_Coefs* dspBiqCoefs = (volatile T_Biquad_Optim_SP_Coefs*)(volatile void*) &coef->band;
	u32 i;


	DSP_WRITE(&coef->decayFactor,    spe->m_decayFactor);
	DSP_WRITE(&coef->log2todBFactor, spe->m_dBscale * (s32)0xfffffcfe);	//= -20log(2) * (1<< 23 - LOG2SHIFT(=16)) * dBscale


	FORi(mod->m_nfilters)
	{
		//update biquad coefs if dirty
		if (m_bands[i].m_dirtyBiquad) {
			BIQ_UpdateCoefs(&m_bands[i]);
			mod->m_dirtyDspCoefs |= 1u << i;
		}

		//update dsp coefs if dirty
		if (mod->m_dirtyDspCoefs & ((u32)1 << i)) {
			BIQ_UpdateDspCoefs_sp(&m_bands[i], &dspBiqCoefs[i]);
		}
	}

	mod->m_dirtyDspCoefs = 0;

	//MOD_SwapCoefBuffers(mod);

	}
_ret: return;
}


/*
peaks[] must be allocated by the application

if peakFlag = STA_PEAK_LINEAR linear peaks in q23     (always [0:1[)
if peakFlag = STA_PEAK_DB         dB peaks in dBscale (always <= 0)
*/
void SPECMTR_GetPeaks(const tModule* mod, s32* peaks, u32 peakFlag)
{
	if (!mod || !mod->m_dspData || !peaks) {goto _ret;}{

	tSpectrumMeter* const spe = mod->m_sub;
	u32 i;


	//Read the peaks from DSP (linear in q23, dB depending on m_dbscale)
	if (mod->m_type == STA_SPECMTR_NBANDS_1BIQ)
	{
		const volatile T_SpeMeter_1biq_Data* pDspBand = (const volatile T_SpeMeter_1biq_Data*)mod->m_dspData;

		fraction xn = pDspBand->xn;

		//FLAT SIGNAL
		if (xn == spe->m_prev_xn)
		{
			s32 peak = (peakFlag & (u32)STA_PEAK_DB) ? -120 * spe->m_dBscale : 0;

			FORi(spe->m_numBands)
			{
				peaks[i] = peak;
#if 0
				//reset the filter history
				((volatile T_SpeMeter_1biq_Data*)pDspBand)->biqData[0].yn_1 = 0;
				((volatile T_SpeMeter_1biq_Data*)pDspBand)->biqData[0].yn_2 = 0;
				pDspBand++;	//next band
#endif
			}
		}
		//NON-FLAT SIGNAL
		else
		{
			FORi(spe->m_numBands)
			{
				peaks[i]  = (peakFlag & (u32)STA_PEAK_DB) ? (DSP_READ(&pDspBand->peakdB)<<8)>>8
														  :  DSP_READ(&pDspBand->peak);
				pDspBand++;	//next band
			}
		}

		spe->m_prev_xn = xn;

	}
	else
	{
		sta_assert(0); /* should not happen */
	}


	}
_ret: return;
}

/*
void SPECMTR_GetPeaks(const tModule* mod, s32* peaks, u32 peakFlag)
{
	if (!mod || !mod->m_dspData || !peaks) {goto _ret;}{

	tSpectrumMeter* const spe = mod->m_sub;
	u32 i;

	if (mod->m_type == STA_SPECMTR_NBANDS_1BIQ)
	{
		const volatile T_SpeMeter_1biq_Data* pDspBand = (const volatile T_SpeMeter_1biq_Data*)mod->m_dspData;

		if (peakFlag & STA_PEAK_DB)
		{
			#if 1
			FORi(spe->m_numBands) {
				s32 tmp = DSP_READ(&pDspBand->peakdB);	//dB peak in s16.8
				peaks[i] = (tmp << 8) >> 8; //dB peak in s24.8 (extending the last 8 sign bits)
				pDspBand++;	//next band
			}
			#else
			//convert form linear to dB here:
			FORi(spe->m_numBands) {
				s32 peak = DSP_READ(&pDspBand->peak);  //linear peak in q23 (always positive)
				if (peak > 0) {
					float peakdB_f = 20.0 * log10((double)peak / 0x800000);
					peaks[i] = (s32) (peakdB_f * (1 << 8));	//in s24.8
				} else {
					peaks[i] = 0xFF800000; //min value in s16.8 (as if geting peakdB from the DSP)
				}
				pDspBand++;	//next band
			}
			#endif
		}
		else
		{
			FORi(spe->m_numBands) {
				peaks[i] = (s32)DSP_READ(&pDspBand->peak); //linear peak in q23 (always positive)
				pDspBand++;	//next band
			}
		}
	}
	else
	{
		sta_assert(0);
	}


	}
_ret: return;
}
*/
