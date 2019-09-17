/***********************************************************************************/
/*!
*
*  \file      biquadFixed.c
*
*  \brief     <i><b> STAudioLib biquad filter FIXED implementation </b></i>
*
*  \details   Computes the coefficients of biquad filters using fixed type
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

/*
  Note: The filter coefficient functions are adapted from a Matlab source
        given by ST Prague.

  Second-order biquad

          b0      +  b1 * z^-1  +  b2 * z^-2
  H(z) = ----------------------------------
          a0(=1)  +  a1 * z^-1  +  a2 * z^-2
*/


#include "internal.h"

#if STA_NO_FLOAT

#include "mathFixed.h"

//these floating point values are used only by the compiler
#ifndef M_PI
#define M_PI		3.14159265358979323846
#define M_SQRT2		1.41421356237309504880
#endif

//internal scaling factor for the k
/* F was chosen > 95.5 and with best precision for below defines */
#define FRAC_1_F        175125
#define FRAC_1_FF       1828
#define FRAC_SQRT2_F    247664


//The biquad output shift should be fixed (in peculiar between OFF/bypass and ON) to avoid "pop" noise.
//note: set as 1 to be aligned between DP and SP
#define OUTSHIFT	1


void BIQ_Bypass(tBiquad* bq);


static INLINE f32 max3f(f32 a, f32 b, f32 c)
{
	return (a > b) ? ((a > c) ? a : c)
		           : ((b > c) ? b : c);
}


void BIQ_Init(tBiquad* bq, STA_FilterType type, s32 G, s32 Q, s32 fc)
{
	BIQ_Bypass(bq);					//output coefs, dirty
	BIQ_SetFilterType(bq, type);
	BIQ_SetGain(bq, G);
	BIQ_SetQual(bq, Q);
	BIQ_SetFreq(bq, fc, STA_SAMPLING_FREQ);
}


void BIQ_Bypass(tBiquad* bq)
{
	f32* m_b = bq->m_b;
	f32* m_a = bq->m_a;

	m_b[0] = 0x800000 >> OUTSHIFT;
	m_a[0] = ONE;
	m_b[1] = m_b[2] = m_a[1] = m_a[2] = 0;

	bq->m_bshift   = 0;
	bq->m_outshift = OUTSHIFT;

	bq->m_userCoefs_q23 = 0;
	bq->m_dirtyBiquad   = 0;
}

void BIQ_SetQual(tBiquad* bq, s32 Q)
{
	bq->m_q = Q * FRAC_(0.1);

	bq->m_userCoefs_q23 = 0;
	bq->m_dirtyBiquad   = 1;
}

s32 BIQ_GetQual(const tBiquad* bq)
{
	if (!bq) {return 0;}
	return (s32) (bq->m_q / FRAC_(0.1));
}

void BIQ_SetFreq(tBiquad* bq, s32 fc, s32 fs)
{
//	s64 tmp64;
	f32 tmp;

	sta_assert(fc > 0 && fs > 0);

	bq->m_fc = fc;
	bq->m_fs = fs;
	bq->m_fn = F_DIV(fc, fs); //(f32) fc / fs;
	// bq->m_K  = (1/F) / TAN(bq->m_fn * M_PI);
	// x = m_fn*PI
	// m_K = (1/F)/TAN(x) = (1/F)*COS(x)/SIN(x)
	tmp = F_MUL(FRAC_(M_PI), bq->m_fn); // compute x
	bq->m_K = F_SIN((s32)tmp); // compute SIN(x)
	if (bq->m_K == 0) bq->m_K = 1; // avoid division by 0
	bq->m_K = F_DIV(F_MUL(FRAC_1_F,F_COS((s32)tmp)), bq->m_K); // compute (1/F)*COS(x)/SIN(x) = (1/F)/TAN(x)

	//linux kernel does not support int_div_64...
//	tmp64 = (s64) FRAC_(M_PI*2) * fc / fs;  // < PI for fc < Fs/2 (24000 kHz), so we can use F_COS and F_SIN
	tmp   = F_MUL(FRAC_(M_PI*2), bq->m_fn); // < PI for fc < Fs/2 (24000 kHz), so we can use F_COS and F_SIN

	bq->m_cosw0 = F_COS((s32)tmp);
	bq->m_sinw0 = F_SIN((s32)tmp);

	bq->m_userCoefs_q23 = 0;
	bq->m_dirtyBiquad   = 1;
}

s32 BIQ_GetFreq(const tBiquad* bq)
{
	if (!bq) {return 0;}
	return (s32) (bq->m_fc);
}

// G in 1/BGS db step
void BIQ_SetGain(tBiquad* bq, s32 G)
{
	bq->m_G = G;	//m_G is NOT USED (just for the get)

//	bq->m_g = F_DBToLinear((G << 7) / BGS);	//scale from 1/BGS to 1/128 db (1<<7)

	//note: _STA_db2lin() vs F_DBToLinear():
	//_STA_db2lin() converts to Q23, but the biquad coefs functions are using Q24 FIXED
	// ==> mult by 2 to convert from Q23 to Q24
	bq->m_g = _STA_db2lin(G, BGS, NULL, DB_NOFLAG) * 2; // linear gain in q23 (NOT CLAMPED to 0x7FFFFF [-1, 1[)
    bq->m_A = F_SQRT(bq->m_g);

	bq->m_userCoefs_q23 = 0;
	bq->m_dirtyBiquad   = 1;
}

s32 BIQ_GetGain(const tBiquad* bq)
{
	return bq->m_G;
}

//coefs[] = {(b0/2)>>bshift, (b1/2)>>bshift, (b2/2)>>bshift, a1/2, a2/2, bshift}, with a0 = 1.0 (implicitly)
void BIQ_SetHalvedCoefs_q23(tBiquad* bq, const q23* coefs)
{
	//note: assuming that all the coefs are already halved (except a0 = 1)
	bq->m_b[0] = (f32)coefs[0];
	bq->m_b[1] = (f32)coefs[1];
	bq->m_b[2] = (f32)coefs[2];
	bq->m_a[1] = (f32)coefs[3];
	bq->m_a[2] = (f32)coefs[4]; //note: need to roll-back the /2 from a2 for biquad DP

	bq->m_bshift   = (u16)coefs[5];
	bq->m_outshift = 1; //fixed by STA API (to match the biquad SP implementation)

	bq->m_userCoefs_q23 = 1;
	bq->m_dirtyBiquad   = 0;
	//note: caller has to set mod->m_dirtyDspCoefs = 1
}

//TODO: check this when using DP with OUTSHIFT != 1
void BIQ_GetHalvedCoefs_q23(tBiquad* bq, q23* coefs)
{
	coefs[0] = (q23)bq->m_b[0];
	coefs[1] = (q23)bq->m_b[1];
	coefs[2] = (q23)bq->m_b[2];
	coefs[3] = (q23)bq->m_a[1];
	coefs[4] = (q23)bq->m_a[2];
	coefs[5] = (q23)bq->m_bshift;
}
//------------------------------------------------------------------------------

static void BIQ_GetAndApplyBshift(tBiquad* bq)
{
	f32* m_b = bq->m_b;
	f32* m_a = bq->m_a;

	//from FIXED_24 to FRAC
	m_b[0] = FIXED2FRAC(m_b[0]);
	m_b[1] = FIXED2FRAC(m_b[1]);
	m_b[2] = FIXED2FRAC(m_b[2]);
	m_a[0] = FIXED2FRAC(m_a[0]);
	m_a[1] = FIXED2FRAC(m_a[1]);
	m_a[2] = FIXED2FRAC(m_a[2]);

	if (bq->m_simplePrec)
	{
		f32 bmax;

		//all coefs (but a0) must be divided by 2
		bq->m_bshift   = 0;
		bq->m_outshift = 1; //fixed by DSP implementation

		m_b[0] >>= 1;
		m_b[1] >>= 1;
		m_b[2] >>= 1;
		m_a[1] >>= 1;
		m_a[2] >>= 1;

		//find bshift so to keep all b coefs < 1
		bmax  = max3f( abs(m_b[0]), abs(m_b[1]), abs(m_b[2]) );
		while (bmax > ONE) {
			bmax >>= 1;
			bq->m_bshift++;
		}

		//apply bshift
		m_b[0] >>= bq->m_bshift;
		m_b[1] >>= bq->m_bshift;
		m_b[2] >>= bq->m_bshift;
	}
	else
	{
		f32 bmax;

		bq->m_bshift   = 0;
		bq->m_outshift = OUTSHIFT;

		//apply outshift
		m_b[0] >>= bq->m_outshift;
		m_b[1] >>= bq->m_outshift;
		m_b[2] >>= bq->m_outshift;
		m_a[1] >>= 1;	//a1 must be halved in DP
		m_a[2] >>= 1;

		//find bshift so to keep all b coefs < 1
		bmax = max3f( abs(m_b[0]), abs(m_b[1]), abs(m_b[2]) );
		while (bmax > ONE) {
			bmax >>= 1;
			bq->m_bshift++;
		}

		//apply bshift
		m_b[0] >>= bq->m_bshift;
		m_b[1] >>= bq->m_bshift;
		m_b[2] >>= bq->m_bshift;

	}

	sta_assert(IS_Q23(m_b[0]));
	sta_assert(IS_Q23(m_b[1]));
	sta_assert(IS_Q23(m_b[2]));
	sta_assert(m_a[0] == ONE);
	sta_assert(IS_Q23(m_a[1]));
	sta_assert(IS_Q23(m_a[2] << 1));  //because a2 rescale by /2 will be rolled-back for biquad DP

	bq->m_dirtyBiquad = 0;
	//note: caller has to set mod->m_dirtyDspCoefs = 1
}

//------------------------------------------------------------------------------
/*
%   First order Butterworth lowpass filter using bilinear transform
%
%  H(s) = 1 / (s + 1);
%
%             1/F       1/F
%           ------- + ------- * z^-1
%           1/F + k   1/F + k
%  H(z) = ---------------------------
%                     1/F - k
%              1   +  ------- * z^-1
%                     1/F + k
%    k = (1/F)/tan(PI*fc/fs)
*/
void BIQ_LowPassButtw1(tBiquad* bq)//, f32 fn)
{
	f32  k = bq->m_K;
	f32* b = bq->m_b;
	f32* a = bq->m_a;

	if (!bq->m_dirtyBiquad) return; //nothing to do

	b[0] = b[1] = F_DIV(FRAC_1_F , FRAC_1_F + k);
	a[1] = F_DIV(FRAC_1_F - k, FRAC_1_F + k);

	a[0] = FRAC_1;
	b[2] = 0;
	a[2] = 0;

	BIQ_GetAndApplyBshift(bq);
}
//------------------------------------------------------------------------------
/*
%  Second order Butterworth lowpass filter using bilinear transform
%
%  H(s) = 1 / (s^2 + 1.4142*s + 1);
%
%                1/F^2                  2/F^2                        1/F^2
%        -------------------- + -------------------- * z^-1 + -------------------- *z^-2
%        1/F^2+1.4142*k/F+k^2   1/F^2+1.4142*k/F+k^2          1/F^2+1.4142*k/F+k^2
% H(z) = -------------------------------------------------------------------------------
%              2/F^2 - 2*k^2                1/F^2-1.4142*k/F+k^2
%        1 + -------------------- * z^-1  + --------------------
%            1/F^2+1.4142*k/F+k^2           1/F^2+1.4142*k/F+k^2
*/

void BIQ_LowPassButtw2(tBiquad* bq)//, f32 fn)
{
	f32  k = bq->m_K;
	f32* b = bq->m_b;
	f32* a = bq->m_a;
	f32  A;
	f32  B;

	if (!bq->m_dirtyBiquad) return; //nothing to do

	A = F_MUL(FRAC_SQRT2_F, k);
	B = F_MUL(k, k);

	b[2] = b[0] = F_DIV(FRAC_1_FF, FRAC_1_FF + A + B );
	b[1] = 2 * b[0];
	a[1] = F_DIV(2*FRAC_1_FF-2*B, FRAC_1_FF + A + B );
	a[2] = F_DIV(FRAC_1_FF - A + B, FRAC_1_FF + A + B );

	a[0] = FRAC_1;

	BIQ_GetAndApplyBshift(bq);
}
//------------------------------------------------------------------------------
/*
% First order Butterworth highpass filter using bilinear transform
%
% H(s) = 1 / (1/s + 1);
%
%             k        -k
%          ------- + ------- * z^-1
%          1/F + k   1/F + k
% H(z) ?? ---------------------------
%                   1/F - k
%             1   + ------- * z^-1
%                   1/F + k
%
%   k = (1/F)/tan(PI*fc/fs)
*/
void BIQ_HighPassButtw1(tBiquad* bq)//, f32 fn)
{
	f32  k = bq->m_K;
	f32* b = bq->m_b;
	f32* a = bq->m_a;

	if (!bq->m_dirtyBiquad) return; //nothing to do

	b[0] = F_DIV(k , FRAC_1_F + k);
	b[1] = -b[0];
	a[1] = F_DIV(FRAC_1_F - k , FRAC_1_F + k);

	a[0] = FRAC_1;
	b[2] = 0;
	a[2] = 0;

	BIQ_GetAndApplyBshift(bq);
}
//------------------------------------------------------------------------------
/*
% Second order Butterworth highpass filter using bilinear transform
%
% H(s) = 1 / (1/s + 1.4142/s + 1);
%
% H(z) = (b0 + b1 * z^-1 + b2 * z^-2)/(1 + a1 * z^-1 + a2 + z-2)
%
%                  k^2
%   b0 =  ------------------------
%         1/F^2 + 1.4142*k/F + k^2
%
%   b1 = -2*b0
%
%   b2 = b0
%
%           2/F^2 - 2*k^2
%   a1 = ------------------------
%        1/F^2 + 1.4142*k/F + k^2
%
%        1/F^2 - 1.4142*k/F + k^2
%   a2 = -------------------------
%        1/F^2 + 1.4142*k/F + k^2
%
%
%    k = (1/F)*cos(pi*fc/fs)/sin(pi*fc/fs)
*/
void BIQ_HighPassButtw2(tBiquad* bq)//, f32 fn)
{
	f32  k = bq->m_K;
	f32* b = bq->m_b;
	f32* a = bq->m_a;
	f32  A;
	f32  B;

	if (!bq->m_dirtyBiquad) return; //nothing to do

	A = F_MUL(FRAC_SQRT2_F, k);
	B = F_MUL(k, k);

	b[2] = b[0] = F_DIV(B, FRAC_1_FF + A + B );
	b[1] = -2 * b[0];
	a[1] = F_DIV(2*FRAC_1_FF-2*B, FRAC_1_FF + A + B );
	a[2] = F_DIV(FRAC_1_FF - A + B, FRAC_1_FF + A + B );

	a[0] = FRAC_1;

	BIQ_GetAndApplyBshift(bq);
}

//------------------------------------------------------------------------------
/*
% First order bass shelving filter using bilinear transform
%
% H(s) = (s + g) / (s + 1);
%
% g >= 1.0
%          g/F + k     g/F - k
%         --------- + --------- * z^-1
%          1/F + k     1/F + k
% H(z) = ---------------------------
%                    1/F - k
%             1   + --------- * z^-1
%                    1/F + k
%
% g < 1.0
%          g/F + gk     g/F - gk
%         ---------- + ---------- * z^-1
%          1/F + gk     1/F + gk
% H(z) = ---------------------------
%                    1/F - gk
%             1   + ---------- * z^-1
%                    1/F + gk
*/
//param in: k = (1/F)/tan(fn * PI), where fn = fc / fs
//param in: g
void BIQ_BassShelv1(tBiquad* bq)
{
	f32  g = bq->m_g;
	f32  k = bq->m_K;
	f32* b = bq->m_b;
	f32* a = bq->m_a;
	f32  A;
	f32  B;

//	bq->m_type = STA_BIQUAD_BASS_SHELV_1;

	if (!bq->m_dirtyBiquad) return; //nothing to do

    A = F_MUL(FRAC_1_F, g);

	if (g >= FRAC_1) {
	   b[0] = F_DIV(A + k, FRAC_1_F + k);
	   b[1] = F_DIV(A - k, FRAC_1_F + k);
	   a[1] = F_DIV(FRAC_1_F - k, FRAC_1_F + k);
    }
	else {
	   B = F_MUL(g, k);
       b[0] = F_DIV(A + B, FRAC_1_F + B);
       b[1] = F_DIV(A - B, FRAC_1_F + B);
       a[1] = F_DIV(FRAC_1_F - B, FRAC_1_F + B);
    }

	a[0] = FRAC_1;
	b[2] = 0;
	a[2] = 0;

	BIQ_GetAndApplyBshift(bq);
}

//------------------------------------------------------------------------------
/*
%  second order bass shelving filter using bilinear transform
%
%         s^2 + sqrt(2*g) * s + g
% H(s) = --------------------------
%             s^2 + sqrt(s) + 1
%
% if g >= 1
%        g/F^2+sqrt(2*g)*k/F+k^2      2*g/F^2-2*k^2               g/F^2-sqrt(2*g)*k/F+k^2
%        ----------------------- + --------------------- * z^-1 + ----------------------- *z^-2
%         1/F^2+sqrt(2)*k/F+k^2    1/F^2+sqrt(2)*k/F+k^2           1/F^2+sqrt(2)*k/F+k^2
% H(z) = --------------------------------------------------------------------------------------
%                2/F^2-2*k^2                 1/F^2-sqrt(2)*k/F+k^2
%        1 + --------------------- * z^-1  + --------------------- *z^-2
%            1/F^2+sqrt(2)*k/F+k^2           1/F^2+sqrt(2)*k/F+k^2
%
% if g < 1
%        g/F^2+g*sqrt(2)*k/F+g*k^2       2*g/F^2-2*g*k^2                g/F^2-sqrt(2)*g*k/F+g*k^2
%        ------------------------- + ------------------------- * z^-1 + ------------------------- * z^-2
%        1/F^2+sqrt(2*g)*k/F+g*k^2   1/F^2+sqrt(2*g)*k/F+g*k^2          1/F^2+sqrt(2*g)*k/F+g*k^2
% H(z) = -----------------------------------------------------------------------------
%                 2/F^2-2*g*k^2                  1/F^2-sqrt(2*g)*k/F+g*k^2
%        1 + ------------------------- * z^-1  + ------------------------- * z^-2
%            1/F^2+sqrt(2*g)*k/F+g*k^2           1/F^2+sqrt(2*g)*k/F+g*k^2
*/

//param in: k = (1/F)/tan(fn * PI), where fn = fc / fs
//param in: g
void BIQ_BassShelv2(tBiquad* bq)
{
	f32  g = bq->m_g;
	f32  K = bq->m_K;
	f32* b = bq->m_b;
	f32* a = bq->m_a;

	f32 AA, BB, CC;

	if (!bq->m_dirtyBiquad) return; //nothing to do

//	bq->m_type = STA_BIQUAD_BASS_SHELV_2;

	AA = F_MUL(K, K);
	BB = F_MUL(K, FRAC_SQRT2_F);  //sqrt(2)*k/F;

	if (g >= FRAC_1) {
	    CC = AA + BB + FRAC_1_FF;
	    b[0] = F_DIV(AA + F_MUL(BB, F_SQRT(g)) + F_MUL(g,FRAC_1_FF), CC);
	    b[1] = F_DIV(2*F_MUL(g,FRAC_1_FF) - 2*AA, CC);
	    b[2] = F_DIV(AA - F_MUL(BB, F_SQRT(g)) + F_MUL(g,FRAC_1_FF), CC);
	    a[1] = F_DIV(2*FRAC_1_FF - 2*AA, CC);
	    a[2] = F_DIV(AA - BB + FRAC_1_FF, CC);
	}
	else {
	    CC = F_MUL(g, AA) + F_MUL(BB, F_SQRT(g)) + FRAC_1_FF;
	    b[0] = F_DIV(F_MUL(g, AA) + F_MUL(g, BB) + F_MUL(g, FRAC_1_FF) , CC);
	    b[1] = F_DIV(2*F_MUL(g, FRAC_1_FF) - 2*F_MUL(g, AA), CC);
	    b[2] = F_DIV(F_MUL(g, AA) - F_MUL(g, BB) + F_MUL(g, FRAC_1_FF) , CC);
	    a[1] = F_DIV(2*FRAC_1_FF - 2*F_MUL(g, AA), CC);
	    a[2] = F_DIV(F_MUL(g, AA) - F_MUL(BB, F_SQRT(g)) + FRAC_1_FF, CC);
	}

	a[0] = FRAC_1;

	BIQ_GetAndApplyBshift(bq);
}
//------------------------------------------------------------------------------
/*
% first order treble shelving filter
%
% H(s) = (1 + g * s) / (s + 1);
%
% g >= 1 -->
%          1/F + g*k     1/F - g*k
%         ----------- + ----------- * z^-1
%           1/F + k       1/F + k
% H(z) = -------------------------------
%                      1/F - k
%             1   +   --------- * z^-1
%                      1-F + k
%
% g < 1 -->
%          g/F + g*k     g/F - g*k
%         ----------- + ----------- * z^-1
%          g/F + k       g/F + k
% H(z) = -------------------------------
%                      g/F - k
%             1   +   ---------- * z^-1
%                      g/F + k
*/
//param in: k = (1/F)/tan(fn * PI), where fn = fc / fs
//param in: g
void BIQ_TrebShelv1(tBiquad* bq)//, f32 fn, f32 g)
{
	f32  g = bq->m_g;
	f32  k = bq->m_K;
	f32* b = bq->m_b;
	f32* a = bq->m_a;
	f32  A;
	f32  B;

	if (!bq->m_dirtyBiquad) return; //nothing to do

    A = F_MUL(g, k);

	if (g >= FRAC_1) {
	   b[0] = F_DIV(FRAC_1_F + A, FRAC_1_F + k);
	   b[1] = F_DIV(FRAC_1_F - A, FRAC_1_F + k);
	   a[1] = F_DIV(FRAC_1_F - k, FRAC_1_F + k);
	}
	else {
	   B = F_MUL(FRAC_1_F, g);
	   b[0] = F_DIV(B + A, B + k);
	   b[1] = F_DIV(B - A, B + k);
	   a[1] = F_DIV(B - k, B + k);
	}

	a[0] = FRAC_1;
	b[2] = 0;
	a[2] = 0;

	BIQ_GetAndApplyBshift(bq);
}
//------------------------------------------------------------------------------
/*
% Second order treble shelving filter using bilinear transform
%
%          g * s^2 + sqrt(2*g) * s + 1
% H(s) =  ------------------------------
%             s^2 + sqrt(2) * s + 1
%
% if g >= 1
%        (1/g)(1/F^2+sqrt(2*g)*k/F)+k^2           2/g/F^2-2*k^2                 (1/g)(1/F^2-sqrt(2*g)*k/F)+k^2
%        ------------------------------ + ---------------------------- * z^-1 + ------------------------------ * z^-2
%         (1/g)(1/F^2+sqrt(2)*k/F+k^2)    (1/g)(1/F^2+sqrt(2)*k/F+k^2)           (1/g)(1/F^2+sqrt(2)*k/F+k^2)
% H(z) = -------------------------------------------------------------------------------------------
%                2/F^2-2*k^2                 1/F^2-sqrt(2)*k/F+k^2
%        1 + --------------------- * z^-1  + --------------------- * z^-2
%            1/F^2+sqrt(2)*k/F+k^2           1/F^2+sqrt(2)*k/F+k^2
%
% if g < 1
%        g/F^2+sqrt(2)*g*k/F+g*k^2       2*g/F^2-2*g*k^2                g/F^2-sqrt(2)*g*k/F+g*k^2
%        ------------------------- + ------------------------- * z^-1 + ------------------------- * z^-2
%         g/F^2+sqrt(2*g)*k/F+k^2     g/F^2+sqrt(2*g)*k/F+k^2            g/F^2+sqrt(2*g)*k/F+k^2
% H(z) = -----------------------------------------------------------------------------------------------
%                 2*g/F^2-2*k^2                   g/F^2-sqrt(2*g)*k/F+k^2
%        1 + ------------------------- * z^-1  + ------------------------- * z^-2
%             g/F^2+sqrt(2*g)*k/F+k^2             g/F^2+sqrt(2*g)*k/F+k^2
*/
//param in: k = (1/F)/tan(fn * PI), where fn = fc / fs
//param in: g
void BIQ_TrebShelv2(tBiquad* bq)//, f32 fn, f32 g)
{
	f32  g = bq->m_g;
	f32  K = bq->m_K;
	f32* b = bq->m_b;
	f32* a = bq->m_a;

	f32 AA,BB,CC,DD;

	if (!bq->m_dirtyBiquad) return; //nothing to do

	AA = F_MUL(K, K);
	BB = F_MUL(K, FRAC_SQRT2_F);  //SQRT(2)*k/F;

	if (g >= FRAC_1) {
	    CC = AA + BB + FRAC_1_FF;
	    DD = F_DIV(CC, g);
	    b[0] = F_DIV(AA + F_DIV(FRAC_1_FF + F_MUL(BB, F_SQRT(g)), g), DD);
	    b[1] = F_DIV(F_DIV(2*FRAC_1_FF, g) - 2*AA, DD);
	    b[2] = F_DIV(AA + F_DIV(FRAC_1_FF - F_MUL(BB, F_SQRT(g)), g), DD);
	    a[1] = F_DIV(2*FRAC_1_FF - 2*AA, CC);
	    a[2] = F_DIV(AA - BB + FRAC_1_FF, CC);
	}
	else {
	    CC = AA+F_MUL(g, FRAC_1_FF) + F_MUL(BB, F_SQRT(g));
	    b[0] = F_DIV(F_MUL(AA + BB + FRAC_1_FF, g), CC);
	    b[1] = F_DIV(2*F_MUL(g, FRAC_1_FF-AA), CC);
	    b[2] = F_DIV(F_MUL(AA - BB + FRAC_1_FF, g), CC);
	    a[1] = F_DIV(2*F_MUL(g, FRAC_1_FF) - 2*AA, CC);
	    a[2] = F_DIV(AA - F_MUL(BB, F_SQRT(g)) + F_MUL(g, FRAC_1_FF), CC);
	}

	a[0] = FRAC_1;

	BIQ_GetAndApplyBshift(bq);
}
//------------------------------------------------------------------------------
/*
% Design second order band shelving filter using bilinear transform
%
% Q correction
% Script performs Q correction of a digital bandpass/stop filter
% with the constraint: lower 3dB cutoff frequency of analogue and digital filter are equal
*/
//param in: fn = fc / fs
//param in: g
//param in: q
static void BIQ_BandShelv(tBiquad* bq)//, f32 fn, f32 g, f32 q)
{
	f32  g = bq->m_g;
	f32  q = bq->m_q;
	f32  k = bq->m_fn; // = Fc/Fs;
	f32* b = bq->m_b;
	f32* a = bq->m_a;
	f32  k1, k2, k3;

	if (!bq->m_dirtyBiquad) return; //nothing to do

	//Q correction
	if (k >= FRAC_(0.04386)) {  // fc >= 2 kHz if fs = 45600  TODO: update for 48000
		const f32 aqc = FRAC_(-3.119513761);
		const f32 bqc = FRAC_(-0.5179761063);
		const f32 cqc = FRAC_( 1.021853509);

		const f32 aq15 = FRAC_( 0.006801984152);
		const f32 bq15 = FRAC_(-0.06130098505);
		const f32 cq15 = FRAC_( 0.6045898018);

		f32 qcomp    = min(FRAC_(4), q);
		f32 qcomp2 	 = F_MUL(qcomp,qcomp);
		f32 qoff15   = F_MUL(aq15, qcomp2) + F_MUL(bq15,qcomp) + cq15 - FRAC_(0.5055);

		f32 tmp      = FRAC_1 - F_MUL(FRAC_(3.652f), abs(FRAC_(0.32894) - k));
		f32 qoffcorr = max(0, tmp);

		f32 qoffset  = F_MUL(qoffcorr, qoff15);

		f32 kk = F_MUL(k,k);

		tmp = F_MUL(aqc, kk) + F_MUL(bqc, k) + cqc + qoffset;

		q = F_MUL(tmp, q);
	}

	//a, b
	if (g < FRAC_1)
		q = F_MUL(q, g);

	k1 = -bq->m_cosw0;
	k2 = F_DIV(2*q - bq->m_sinw0, 2*q + bq->m_sinw0);
	k3 = F_MUL(g, FRAC_1 - k2);

	b[0] = (FRAC_1 + k2 + k3) >> 1;
	b[1] = F_MUL(k1, FRAC_1 + k2);
	b[2] = (FRAC_1 + k2 - k3) >> 1;
	a[0] = FRAC_1;
	a[1] = b[1];
	a[2] = k2;

	BIQ_GetAndApplyBshift(bq);

}
//------------------------------------------------------------------------------
/* All-pass filter for phase shift (flat freq response: |H(s)| = 1)

APF:        H(s) = (s^2 - s/Q + 1) / (s^2 + s/Q + 1)

            b0 =   (1 - alpha)/(1 + alpha)    with w0 = 2*PI*fc/fs   and    alpha = sin(w0)/(2Q)
            b1 =    -2*cos(w0)/(1 + alpha)
            b2 =   (1 + alpha)/(1 + alpha) = 1
            a0 =   b2 = 1
            a1 =   b1
            a2 =   b0

 if f << fc  phase_shift =    0 deg
 if f == fc  phase_shift = -180 deg
 if f >> fc  phase_shift = -360 deg
 */
void BIQ_AllPass2_cb(tBiquad* bq)
{
	f32* m_b = bq->m_b;
	f32* m_a = bq->m_a;

	f32 alpha = F_DIV(bq->m_sinw0, 2 * bq->m_q);
	f32 a0    = FRAC_1 + alpha;

	if (!bq->m_dirtyBiquad) return; //nothing to do

	m_b[0] = m_a[2] = F_DIV(FRAC_1 - alpha, a0);
    m_b[1] = m_a[1] = F_DIV(-2*bq->m_cosw0, a0);
    m_b[2] = m_a[0] = FRAC_1;

	BIQ_GetAndApplyBshift(bq);
}

//------------------------------------------------------------------------------
/* Band-pass filter 2 (CookBook)

BPF2:       H(s) = (s/Q) / (s^2 + s/Q + 1)

            b0 =  alpha    with w0 = 2*PI*fc/fs   and    alpha = sin(w0)/(2Q)
            b1 =  0
            b2 = -alpha
            a0 =  1 + alpha
            a1 = -2*cos(w0)
            a2 =  1 - alpha
*/
static void BIQ_BandPass2_cb(tBiquad* bq)
{
    f32* b = bq->m_b;
    f32* a = bq->m_a;
    f32  alpha;
    f32  a0;

	if (!bq->m_dirtyBiquad) return; //nothing to do

    alpha = F_DIV(bq->m_sinw0, bq->m_q)/2;
    a0 = FRAC_1 + alpha;

    b[0] = F_DIV(alpha, a0);
    b[1] = 0;
    b[2] = -b[0];
    a[1] = -2*F_DIV(bq->m_cosw0, a0);
    a[2] = F_DIV(FRAC_1 - alpha, a0);

    a[0] = FRAC_1;

    BIQ_GetAndApplyBshift(bq);
}
//------------------------------------------------------------------------------
/* LowPass filter 2 (CookBook)

BPF2:       H(s) = 1 / (s^2 + s/Q + 1)

            b0 = (1 - cos(w0))/2    with w0 = 2*PI*fc/fs   and    alpha = sin(w0)/(2Q)
            b1 =  1 - cos(w0)
            b2 =  b0
            a0 =  1 + alpha
            a1 = -2*cos(w0)
            a2 =  1 - alpha
*/
static void BIQ_LowPass2_cb(tBiquad* bq)
{
    f32* b = bq->m_b;
    f32* a = bq->m_a;
    f32  alpha;
    f32  a0;

    if (!bq->m_dirtyBiquad) return; //nothing to do

    alpha = F_DIV(bq->m_sinw0, bq->m_q)/2;
    a0 = FRAC_1 + alpha;

    b[1] = F_DIV(FRAC_1 - bq->m_cosw0, a0);
    b[0] = b[1]/2;
    b[2] = b[0];
    a[1] = -2*F_DIV(bq->m_cosw0, a0);
    a[2] = F_DIV(FRAC_1 - alpha, a0);

    a[0] = FRAC_1;

    BIQ_GetAndApplyBshift(bq);
}
//------------------------------------------------------------------------------
/* HighPass filter 2 (CookBook)

BPF2:       H(s) = s^2 / (s^2 + s/Q + 1)

            b0 =  (1 + cos(w0))/2    with w0 = 2*PI*fc/fs   and    alpha = sin(w0)/(2Q)
            b1 = -(1 + cos(w0))
            b2 =  b0
            a0 =  1 + alpha
            a1 = -2*cos(w0)
            a2 =  1 - alpha
*/
static void BIQ_HighPass2_cb(tBiquad* bq)
{
    f32* b = bq->m_b;
    f32* a = bq->m_a;
    f32  alpha;
    f32  a0;

    if (!bq->m_dirtyBiquad) return; //nothing to do

    alpha = F_DIV(bq->m_sinw0, bq->m_q)/2;
    a0 = FRAC_1 + alpha;

    b[0] = F_DIV(FRAC_1 + bq->m_cosw0, a0)/2;
    b[1] = F_DIV(-FRAC_1 - bq->m_cosw0, a0);
    b[2] = b[0];
    a[1] = -2*F_DIV(bq->m_cosw0, a0);
    a[2] = F_DIV(FRAC_1 - alpha, a0);

    a[0] = FRAC_1;

    BIQ_GetAndApplyBshift(bq);
}
//------------------------------------------------------------------------------
/* Notch filter 2 (CookBook)

Notch:      H(s) = (s^2 + 1) / (s^2 + s/Q + 1)

            b0 =  1
            b1 = -2*cos(w0)
            b2 =  1
            a0 =  1 + alpha
            a1 = -2*cos(w0)
            a2 =  1 - alpha
*/
static void BIQ_Notch_cb(tBiquad* bq)
{
    f32* b = bq->m_b;
    f32* a = bq->m_a;
    f32  alpha;
    f32  a0;

    if (!bq->m_dirtyBiquad) return; //nothing to do

    alpha = F_DIV(bq->m_sinw0, bq->m_q)/2;
    a0 = FRAC_1 + alpha;

    b[0] = F_DIV(FRAC_1, a0);
    b[1] = -1*F_DIV(2*bq->m_cosw0, a0);
    b[2] = b[0];
    a[1] = b[1];
    a[2] = F_DIV(FRAC_1 - alpha, a0);

    a[0] = FRAC_1;

    BIQ_GetAndApplyBshift(bq);
}
//------------------------------------------------------------------------------
/* Peaking filter 2 (CookBook)

Peaking:    H(s) = (s^2 + s*(A/Q)+ 1) / (s^2 + s/Q + 1)

            b0 =  1 + alpha*A
            b1 = -2*cos(w0)
            b2 =  1 - alpha*A
            a0 =  1 + alpha/A
            a1 = -2*cos(w0)
            a2 =  1 - alpha/A
*/
static void BIQ_Peaking_cb(tBiquad* bq)
{
    f32* b = bq->m_b;
    f32* a = bq->m_a;
    f32  A = bq->m_A;
    f32  alpha;
    f32  a0;

    if (!bq->m_dirtyBiquad) return; //nothing to do

    alpha = F_DIV(bq->m_sinw0, bq->m_q)/2;
    a0 = FRAC_1 + F_DIV(alpha, A);

    b[0] = F_DIV(FRAC_1 + F_MUL(alpha, A), a0);
    b[1] = -1*F_DIV(2*bq->m_cosw0, a0);
    b[2] = F_DIV(FRAC_1 - F_MUL(alpha, A), a0);
    a[1] = b[1];
    a[2] = F_DIV(FRAC_1 - F_DIV(alpha, A), a0);

    a[0] = FRAC_1;

    BIQ_GetAndApplyBshift(bq);
}
//------------------------------------------------------------------------------
/* Low shelf 2 (CookBook)

    H(s) = A*(s^2 + (sqrt(A)/Q)*s+ A) / (A*s^2 + (sqrt(A)/Q)*s + 1)

*/
static void BIQ_LowShelf_cb(tBiquad* bq)
{
    f32* b = bq->m_b;
    f32* a = bq->m_a;
    f32  A = bq->m_A;
    f32  cosw0 = bq->m_cosw0;
    f32  B, C, D;
    f32  alpha;
    f32  a0;

    if (!bq->m_dirtyBiquad) return; //nothing to do

    alpha = F_DIV(bq->m_sinw0, bq->m_q)/2;
    B = A + FRAC_1;
    C = A - FRAC_1;
    D = F_MUL(2*F_SQRT(A), alpha);
    a0 = B + D + F_MUL(C, cosw0);

    b[0] = F_DIV(F_MUL(A, B + D - F_MUL(C, cosw0)), a0);
    b[1] = 2*F_DIV(F_MUL(A, C - F_MUL(B, cosw0)), a0);
    b[2] = F_DIV(F_MUL(A, B - D - F_MUL(C, cosw0)), a0);
    a[1] = -2*F_DIV(C + F_MUL(B, cosw0), a0);
    a[2] = F_DIV(B - D + F_MUL(C, cosw0), a0);

    a[0] = FRAC_1;

    BIQ_GetAndApplyBshift(bq);
}
//------------------------------------------------------------------------------
/* HighShelf filter 2 (CookBook)

    H(s) = A*(A*s^2 + (sqrt(A)/Q)*s+ 1) / (s^2 + (sqrt(A)/Q)*s + A)

*/
static void BIQ_HighShelf_cb(tBiquad* bq)
{
    f32* b = bq->m_b;
    f32* a = bq->m_a;
    f32  A = bq->m_A;
    f32  cosw0 = bq->m_cosw0;
    f32  B, C, D;
    f32  alpha;
    f32  a0;

    if (!bq->m_dirtyBiquad) return; //nothing to do

    alpha = F_DIV(bq->m_sinw0, bq->m_q)/2;
    B = A + FRAC_1;
    C = A - FRAC_1;
    D = F_MUL(2*F_SQRT(A), alpha);
    a0 = B + D - F_MUL(C, cosw0);

    b[0] = F_DIV(F_MUL(A, B + D + F_MUL(C, cosw0)), a0);
    b[1] = -2*F_DIV(F_MUL(A, C + F_MUL(B, cosw0)), a0);
    b[2] = F_DIV(F_MUL(A, B - D + F_MUL(C, cosw0)), a0);
    a[1] = 2*F_DIV(C - F_MUL(B, cosw0), a0);
    a[2] = F_DIV(B - D - F_MUL(C, cosw0), a0);

    a[0] = FRAC_1;

    BIQ_GetAndApplyBshift(bq);
}

//------------------------------------------------------------------------------
void BIQ_SetFilterType(tBiquad* bq, STA_FilterType type)
{
	bq->m_type = type;

	switch (type) {
	case STA_BIQUAD_BYPASS:			bq->m_pUpdateCoefs = BIQ_Bypass; break;
	case STA_BIQUAD_LOWP_BUTTW_1:	bq->m_pUpdateCoefs = BIQ_LowPassButtw1; break;
	case STA_BIQUAD_LOWP_BUTTW_2:	bq->m_pUpdateCoefs = BIQ_LowPassButtw2; break;
	case STA_BIQUAD_HIGP_BUTTW_1:	bq->m_pUpdateCoefs = BIQ_HighPassButtw1; break;
	case STA_BIQUAD_HIGP_BUTTW_2:	bq->m_pUpdateCoefs = BIQ_HighPassButtw2; break;
	case STA_BIQUAD_BASS_SHELV_1:	bq->m_pUpdateCoefs = BIQ_BassShelv1; break;
	case STA_BIQUAD_BASS_SHELV_2:	bq->m_pUpdateCoefs = BIQ_BassShelv2; break;
	case STA_BIQUAD_TREB_SHELV_1:	bq->m_pUpdateCoefs = BIQ_TrebShelv1; break;
	case STA_BIQUAD_TREB_SHELV_2:	bq->m_pUpdateCoefs = BIQ_TrebShelv2; break;
	case STA_BIQUAD_BAND_BOOST_2:	bq->m_pUpdateCoefs = BIQ_BandShelv; break;
	case STA_BIQUAD_AP2_CB:			bq->m_pUpdateCoefs = BIQ_AllPass2_cb; break;
	case STA_BIQUAD_LP2_CB:			bq->m_pUpdateCoefs = BIQ_LowPass2_cb; break;
	case STA_BIQUAD_HP2_CB:			bq->m_pUpdateCoefs = BIQ_HighPass2_cb; break;
	case STA_BIQUAD_BP2_CB:			bq->m_pUpdateCoefs = BIQ_BandPass2_cb; break;
	case STA_BIQUAD_NOTCH_CB:		bq->m_pUpdateCoefs = BIQ_Notch_cb; break;
	case STA_BIQUAD_PEAKING_CB:		bq->m_pUpdateCoefs = BIQ_Peaking_cb; break;
	case STA_BIQUAD_LSHELF_CB:		bq->m_pUpdateCoefs = BIQ_LowShelf_cb; break;
	case STA_BIQUAD_HSHELF_CB:		bq->m_pUpdateCoefs = BIQ_HighShelf_cb; break;
	default: sta_assert(0); return;
	}

	bq->m_dirtyBiquad = 1;
}

//------------------------------------------------------------------------------
// Support functions to update coefs into DSP
//------------------------------------------------------------------------------

//FOR BIQUAD_DP ONLY
void BIQ_SetDspCoefs2bypass(volatile T_Biquad_Coefs* coefs)
{
	DSP_WRITE(&coefs->b0, 		0x800000 >> OUTSHIFT);
	DSP_WRITE(&coefs->b1, 		0);
	DSP_WRITE(&coefs->b2,		0);
	DSP_WRITE(&coefs->a1_half, 	0);
	DSP_WRITE(&coefs->a2, 		0);
	DSP_WRITE(&coefs->bshift,	0);
	DSP_WRITE(&coefs->oshift, 	OUTSHIFT);
}
//------------------------------------------------------------------------------
//FOR BIQUAD_SP ONLY
void BIQ_SetDspCoefs2bypass_sp(volatile T_Biquad_Optim_SP_Coefs* coefs)
{
	DSP_WRITE(&coefs->b0, 		0x800000 >> 1);
	DSP_WRITE(&coefs->b1, 		0);
	DSP_WRITE(&coefs->b2, 		0);
	DSP_WRITE(&coefs->bshift,	0);
	DSP_WRITE(&coefs->a1, 		0);
	DSP_WRITE(&coefs->a2, 		0);
}
//------------------------------------------------------------------------------
//FOR BIQUAD_DP ONLY
void BIQ_UpdateDspCoefs(const tBiquad* bq, volatile T_Biquad_Coefs* coefs)
{
	DSP_WRITE(&coefs->b0, 		bq->m_b[0]);	//prescaled by >>(bshit+oshift)
	DSP_WRITE(&coefs->b1, 		bq->m_b[1]);	//prescaled by >>(bshit+oshift)
	DSP_WRITE(&coefs->b2, 		bq->m_b[2]);	//prescaled by >>(bshit+oshift)
	DSP_WRITE(&coefs->a1_half, 	bq->m_a[1]);	//prescaled by /2
	DSP_WRITE(&coefs->a2, 		bq->m_a[2] << 1);
	DSP_WRITE(&coefs->bshift,	bq->m_bshift);
	DSP_WRITE(&coefs->oshift, 	bq->m_outshift);

	//note: caller has to set mod->m_dirtyDspCoefs = 0
}
//------------------------------------------------------------------------------
//FOR BIQUAD_SP ONLY
void BIQ_UpdateDspCoefs_sp(const tBiquad* bq, volatile T_Biquad_Optim_SP_Coefs* coefs)
{
	sta_assert(bq->m_simplePrec);

	DSP_WRITE(&coefs->b0,		bq->m_b[0]);  //Q23 prescaled by /2 >> bshift
	DSP_WRITE(&coefs->b1,		bq->m_b[1]);  //Q23 prescaled by /2 >> bshift
	DSP_WRITE(&coefs->b2,		bq->m_b[2]);  //Q23 prescaled by /2 >> bshift
	DSP_WRITE(&coefs->bshift, 	bq->m_bshift);
	DSP_WRITE(&coefs->a1,		bq->m_a[1]);  //Q23 prescaled by /2
	DSP_WRITE(&coefs->a2,		bq->m_a[2]);  //Q23 prescaled by /2

	//note: the caller has to set mod->m_dirtyDspCoefs = 0
}

#endif // STA_NO_FLOAT
