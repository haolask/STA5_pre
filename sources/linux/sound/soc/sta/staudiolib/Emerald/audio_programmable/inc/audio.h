/***********************************************************************************/
/*!
*  \file      audio.h
*
*  \brief     <i><b> Main header for the STAudio programmable Audio lib for EMERALD DSP </b></i>
*
*  \details   Main header for the STAudio programmable Audio lib for EMERALD DSP
*
*  \author    APG-MID Application Team
*
*  \author    (original version) Christophe Quarre
*
*  \version   1.0
*
*  \date      2013/04/15
*
*  \bug       Unknown
*
*  \warning
*
*  This file is part of <STAudioLib> and is dual licensed,
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
* <STAudioLib> is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* <STAudioLib> is distributed in the hope that it will be
* useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* Please refer to <http://www.gnu.org/licenses/>.
*/
/***********************************************************************************/

#ifndef _AUDIO_H_
#define _AUDIO_H_

#define DSP_ALIB_VERSION    39

//---------------------------------------------------------------------------
//EMERARLD Specific
//---------------------------------------------------------------------------
//When compiling the programmable Audio lib on Emerald
#ifdef EMERALDCC
#include "emerald.h"	        // This must be always included
#define FRAC(fl)				(fl)
#else
//---------------------------------------------------------------------------
//When compiling STAudioLib on ARM
//From emerald.h
typedef long 				    fraction;
#define _XMEM
#define _YMEM

//lint -emacro(960, FRAC)   misra2004 10.4: "cast of complex expression from float to int" (required to convert to dsp fixed point)
//WARNING: FRAC(fl) assumes that 'fl' is >= -1

//#define FRAC(fl)				((fraction)((fl) * 0x7FFFFF + 0.5f)) 	//fixed signed 0.24 bits
//#define FRAC(f)				(MIN((fraction)((f)*0x800000), 0x7FFFFF))
#define FRAC(f)					(MIN((fraction)((f)*(0x800000<<1)+1)/2, 0x7FFFFF))

#pragma pack(push, 1)           //set struct pack mode to exact fit - no padding
#endif  //EMERALDCC

//---------------------------------------------------------------------------
// COMPILE SWITCHES
//---------------------------------------------------------------------------
// DO NOT MODIFY FOLLOWING LINES !!!

//----- COMPANDER -----
//select only one compander version
//#define COMPANDER_VER_1
  #define COMPANDER_VER_2

//----- MIXER -----
//select only one mixer version
//#define MIXER_VER_1       /*static*/
//#define MIXER_VER_2       /*linear/exp not optimized*/
  #define MIXER_VER_3       /*linear/exp optimized*/


//---------------------------------------------------------------------------
// INCLUDES
//---------------------------------------------------------------------------
#include "audio_lib.h"		    //imported audio lib from ADR3/HIT2
#include "biquad_dp.h"
#include "biquad_sp_optimized.h"
#include "audio_cd_deemphasis.h"
#include "audio_gain.h"
#include "audio_loudness.h"
#include "audio_limiter2.h"
#include "audio_clip_limiter.h"
#include "audio_delay_line.h"
#include "audio_sinewave.h"
#include "audio_peak_detector.h"
#include "dco_detect_ref.h"

#ifdef MIXER_VER_1
#include "audio_mixer.h"
#elif defined MIXER_VER_2
#include "audio_mixer2.h"
#elif defined MIXER_VER_3
#include "audio_mixer3.h"
#else
//#warning "One Mixer version must be defined!
#endif

#ifdef COMPANDER_VER_1
#include "audio_compander.h"
#elif defined COMPANDER_VER_2
#include "audio_compander2.h"
#else
//#warning "One Compander version must be defined!
#endif


//---------------------------------------------------------------------------
// COMMON stuff
//---------------------------------------------------------------------------
#ifndef NULL
#define NULL  0
#endif


//---------------------------------------------------------------------------
//  IRQ
//---------------------------------------------------------------------------
//IRQ registers
#ifdef  ACCORDO2_DSP2
#define IRQ_DSP2ARM_SET      (*((unsigned int _XMEM_EXT*) 0x4000))
#define IRQ_DSP2ARM_CLR      (*((unsigned int _XMEM_EXT*) 0x4001))
#define IRQ_ARM2DSP_EN       (*((unsigned int _XMEM_EXT*) 0x4002))
#define IRQ_ARM2DSP_STS      (*((unsigned int _XMEM_EXT*) 0x4003))
#else
#define IRQ_DSP2ARM_SET      (*((unsigned int _XMEM_EXT*) 0x2000))
#define IRQ_DSP2ARM_CLR      (*((unsigned int _XMEM_EXT*) 0x2001))
#define IRQ_ARM2DSP_EN       (*((unsigned int _XMEM_EXT*) 0x2002))
#define IRQ_ARM2DSP_STS      (*((unsigned int _XMEM_EXT*) 0x2003))
#endif

//IRQ lines
#define IRQ_ARM1             1
#define IRQ_ARM2             2
#define IRQ_ARM3             3
#define IRQ_FSYNC            4
#define IRQ_CK1              5
#define IRQ_CK2              6
#define IRQ_ARM4             7

//IRQ Enables
#define IRQ_ARM1_EN          (1 << 1)
#define IRQ_ARM2_EN          (1 << 2)
#define IRQ_ARM3_EN          (1 << 3)
#define IRQ_FSYNC_EN         (1 << 4)
#define IRQ_CK1_EN           (1 << 5)
#define IRQ_CK2_EN           (1 << 6)
#define IRQ_ARM4_EN          (1 << 7)


//---------------------------------------------------------------------------
//  Programmable Audio pipeline management
//---------------------------------------------------------------------------
extern int g_update_slot;


typedef void (*FUNCPTR)(void);

typedef void (*AUDIO_FUNCPTR)(fraction _XMEM *data, fraction _YMEM *params);


typedef struct {
    //int id;                      //just for reference
    AUDIO_FUNCPTR       func;      //filter func: init, run, update...
    fraction _XMEM      *data;     //ptr to corresponding data struct
    fraction _YMEM      *params;   //ptr to corresponding params struct
} T_Filter;

typedef struct {
    AUDIO_FUNCPTR       audioFunc;
    AUDIO_FUNCPTR       initFunc;
    AUDIO_FUNCPTR       updateFunc;
} T_FilterFuncs;


void audio_lib_init(void);

void audio_process(T_Filter _YMEM *filterTable);

// First value = num of transfers (must be even number)
void audio_transfer( fraction* _YMEM * transferTable );

void audio_clear_data(fraction _XMEM *addr, int len);
/*
//Pb with compilation
#macro audio_clear_data(addr, len)
{
    while(len) {
        *(addr)++ = 0;
        (len)--;
    }
}
#endm
*/

void exit_loop(void);
void exit_loop_incSlot(void);
void exit_loop_resetSlot(void);

//---------------------------------------------------------------------------
//  MUX
//---------------------------------------------------------------------------

//TODO (?)
//void mux_init(fraction **mux, int mux_size, fraction *ins);

//---------------------------------------------------------------------------
//  CD Deemphasis
//---------------------------------------------------------------------------

//implemented in audio_lib
void audio_cd_deemphasis_init(T_CDDeemphasisData _XMEM *data, T_CDDeemphasisParam _YMEM *params);
void audio_cd_deemphasis(T_CDDeemphasisData _XMEM *data, T_CDDeemphasisParam _YMEM *params);

//---------------------------------------------------------------------------
//  STATIC GAIN
//---------------------------------------------------------------------------
/*
//defined in audio_gain.h
typedef struct{
    fraction _XMEM *ins;
    fraction _XMEM *out;
} T_GainData;
*/
typedef struct {
    fraction        gain;
    int             scale;
} T_GainSimpleParams;

typedef struct {
    int             numCh;
    fraction        initGain;
    int             initScale;
    fraction _YMEM  *gains;
    int      _YMEM  *scales;
} T_GainSimpleNchParams;


//NOTE: params must be pre-initialised with numCh, gains and scales before calling this function
//void audio_gain_simple_Nch_init(T_GainSimpleParams *initGainScale, T_GainSimpleNchParams _YMEM *params);
void audio_gain_simple_Nch_init(T_GainData _XMEM *data, T_GainSimpleNchParams _YMEM *params);

void audio_gain_simple_Nch(T_GainData _XMEM *data, T_GainSimpleNchParams _YMEM *params);

//More efficient than _Nch version
void audio_gain_simple_2Nch(T_GainData _XMEM *data, T_GainSimpleNchParams _YMEM *params);

//---------------------------------------------------------------------------
//  SMOOTH GAIN (exponential and linear)
//---------------------------------------------------------------------------
/*
//defined in audio_gain.h
typedef struct{
    fraction _XMEM *ins;
    fraction _XMEM *out;
} T_GainData;
*/

typedef struct {
    fraction        gains[2];       //gain += inc; inc *= incfactor;
    fraction        gainGoals[2];   //not used
    fraction        inc[2];
    fraction        incFactor[2];
} T_GainSmooth2chParams;
/*
typedef struct {
    int             numCh;
    fraction        gainUp;
    fraction        gainDown;
    fraction _YMEM  *gainGoals;
    fraction _YMEM  *gains;
} T_GainSmoothNchParams;
*/

//(the params order is fixed and optimized for the smoothing implementation)
//previously
/*
typedef struct {
    fraction        inc;
    fraction        incFactor;
    fraction        gainGoal;
    fraction        gain;
    int             lshift;
} T_GainSmooth1chParams;
*/
typedef struct {
    fraction        gain;
    fraction        gainGoal;
    int             lshift;
    fraction        inc;
    fraction        incFactor;
} T_GainSmooth1chParams;

typedef struct {
    int                           sync; // used to synchronize ARM/DSP memory modification to avoid noise
    int                           numCh;
    T_GainSmooth1chParams _YMEM*  pch;
} T_GainSmoothNchParams;


//EXPONENTIAL GAIN
//smooth gain, self-updating (no init function, must be initialized by Host CPU)
void audio_gain_smooth_Nch(T_GainData _XMEM *data, T_GainSmoothNchParams _YMEM *params);

//LINEAR GAIN
//#define T_GainLinear1chParams  T_GainSmooth1chParams
//#define T_GainLinearNchParams  T_GainSmoothNchParams
typedef T_GainSmooth1chParams  T_GainLinear1chParams;
typedef T_GainSmoothNchParams  T_GainLinearNchParams;

void audio_gain_linear_Nch(T_GainData _XMEM *data, T_GainLinearNchParams _YMEM *params);

//---------------------------------------------------------------------------
//  BIT-SHIFTER
//---------------------------------------------------------------------------

typedef struct {
    int             halvedNumCh;
    int      _YMEM  *leftShift;
} T_BitShiftNchParams;

typedef T_GainData T_BitShiftData;

void audio_bitshift_2Nch(T_BitShiftData _XMEM *data, T_BitShiftNchParams _YMEM *params);


//---------------------------------------------------------------------------
//  LOUDNESS
//---------------------------------------------------------------------------

typedef  T_Biquad_DP_Stage_2  T_Loudness_Data;

typedef struct {
    T_Biquad_DP_Stage_2  ch[2];
} T_LoudnessDual_Data;

//already defined in audio_loudness.h
/*
typedef struct {
    T_Biquad_Coefs LP_coefs;
    T_Biquad_Coefs HP_coefs;
} T_Loudness_Coefs;
*/

void loudness_static_dp_init( T_Loudness_Data _XMEM *data, T_Loudness_Coefs _YMEM *coefs );
void loudness_static_dp( T_Loudness_Data _XMEM *data, T_Loudness_Coefs _YMEM *coefs );

void loudness_static_dual_dp_init( T_LoudnessDual_Data _XMEM *data, T_Loudness_Coefs _YMEM *coefs );
void loudness_static_dual_dp( T_LoudnessDual_Data _XMEM *data, T_Loudness_Coefs _YMEM *coefs );


//---------------------------------------------------------------------------
//  TONES
//---------------------------------------------------------------------------
typedef T_Biquad_DP_Stage_3		T_Tone_Data;

typedef struct {
    T_Biquad_DP_Stage_3 ch[2];
} T_ToneDual_Data;

typedef struct {
    T_Biquad_Coefs coefs[3];
} T_Tone_Coefs;

void tone_static_dp_init( T_Tone_Data _XMEM *data,  T_Tone_Coefs _YMEM *coefs );
void tone_static_dp( T_Tone_Data _XMEM *data,  T_Tone_Coefs _YMEM *coefs );

void tone_static_dual_dp_init( T_ToneDual_Data _XMEM *data,  T_Tone_Coefs _YMEM *coefs );
void tone_static_dual_dp( T_ToneDual_Data _XMEM *data,  T_Tone_Coefs _YMEM *coefs );

//---------------------------------------------------------------------------
//  EQUALIZER DP
//---------------------------------------------------------------------------
#define EQ_MAX_NUM_BANDS    16

//Note: The following structs are for 1 ch. For Nch, use a contigous array of Data. (Coefs are shared for all channels)

//EQ N-band DP DATA (1 ch)
typedef struct {T_Biquad_DP_Input in;  T_Biquad_DP_Data data[1];}  T_Eq_1band_DP_Data;
typedef struct {T_Biquad_DP_Input in;  T_Biquad_DP_Data data[2];}  T_Eq_2bands_DP_Data;
typedef struct {T_Biquad_DP_Input in;  T_Biquad_DP_Data data[3];}  T_Eq_3bands_DP_Data;
typedef struct {T_Biquad_DP_Input in;  T_Biquad_DP_Data data[4];}  T_Eq_4bands_DP_Data;
typedef struct {T_Biquad_DP_Input in;  T_Biquad_DP_Data data[5];}  T_Eq_5bands_DP_Data;
typedef struct {T_Biquad_DP_Input in;  T_Biquad_DP_Data data[6];}  T_Eq_6bands_DP_Data;
typedef struct {T_Biquad_DP_Input in;  T_Biquad_DP_Data data[7];}  T_Eq_7bands_DP_Data;
typedef struct {T_Biquad_DP_Input in;  T_Biquad_DP_Data data[8];}  T_Eq_8bands_DP_Data;
typedef struct {T_Biquad_DP_Input in;  T_Biquad_DP_Data data[9];}  T_Eq_9bands_DP_Data;
typedef struct {T_Biquad_DP_Input in;  T_Biquad_DP_Data data[10];} T_Eq_10bands_DP_Data;
typedef struct {T_Biquad_DP_Input in;  T_Biquad_DP_Data data[11];} T_Eq_11bands_DP_Data;
typedef struct {T_Biquad_DP_Input in;  T_Biquad_DP_Data data[12];} T_Eq_12bands_DP_Data;
typedef struct {T_Biquad_DP_Input in;  T_Biquad_DP_Data data[13];} T_Eq_13bands_DP_Data;
typedef struct {T_Biquad_DP_Input in;  T_Biquad_DP_Data data[14];} T_Eq_14bands_DP_Data;
typedef struct {T_Biquad_DP_Input in;  T_Biquad_DP_Data data[15];} T_Eq_15bands_DP_Data;
typedef struct {T_Biquad_DP_Input in;  T_Biquad_DP_Data data[16];} T_Eq_16bands_DP_Data;

//EQ N-band DP COEFS
typedef struct {int numCh, numBands;   T_Biquad_Coefs coefs[1];}   T_Eq_1band_Coefs;
typedef struct {int numCh, numBands;   T_Biquad_Coefs coefs[2];}   T_Eq_2bands_Coefs;
typedef struct {int numCh, numBands;   T_Biquad_Coefs coefs[3];}   T_Eq_3bands_Coefs;
typedef struct {int numCh, numBands;   T_Biquad_Coefs coefs[4];}   T_Eq_4bands_Coefs;
typedef struct {int numCh, numBands;   T_Biquad_Coefs coefs[5];}   T_Eq_5bands_Coefs;
typedef struct {int numCh, numBands;   T_Biquad_Coefs coefs[6];}   T_Eq_6bands_Coefs;
typedef struct {int numCh, numBands;   T_Biquad_Coefs coefs[7];}   T_Eq_7bands_Coefs;
typedef struct {int numCh, numBands;   T_Biquad_Coefs coefs[8];}   T_Eq_8bands_Coefs;
typedef struct {int numCh, numBands;   T_Biquad_Coefs coefs[9];}   T_Eq_9bands_Coefs;
typedef struct {int numCh, numBands;   T_Biquad_Coefs coefs[10];}  T_Eq_10bands_Coefs;
typedef struct {int numCh, numBands;   T_Biquad_Coefs coefs[11];}  T_Eq_11bands_Coefs;
typedef struct {int numCh, numBands;   T_Biquad_Coefs coefs[12];}  T_Eq_12bands_Coefs;
typedef struct {int numCh, numBands;   T_Biquad_Coefs coefs[13];}  T_Eq_13bands_Coefs;
typedef struct {int numCh, numBands;   T_Biquad_Coefs coefs[14];}  T_Eq_14bands_Coefs;
typedef struct {int numCh, numBands;   T_Biquad_Coefs coefs[15];}  T_Eq_15bands_Coefs;
typedef struct {int numCh, numBands;   T_Biquad_Coefs coefs[16];}  T_Eq_16bands_Coefs;

void eq_static_1band_Nch_dp(   T_Eq_1band_DP_Data   _XMEM *data,  T_Eq_1band_Coefs   _YMEM *coefs );
void eq_static_2bands_Nch_dp(  T_Eq_2bands_DP_Data  _XMEM *data,  T_Eq_2bands_Coefs  _YMEM *coefs );
void eq_static_3bands_Nch_dp(  T_Eq_3bands_DP_Data  _XMEM *data,  T_Eq_3bands_Coefs  _YMEM *coefs );
void eq_static_4bands_Nch_dp(  T_Eq_4bands_DP_Data  _XMEM *data,  T_Eq_4bands_Coefs  _YMEM *coefs );
void eq_static_5bands_Nch_dp(  T_Eq_5bands_DP_Data  _XMEM *data,  T_Eq_5bands_Coefs  _YMEM *coefs );
void eq_static_6bands_Nch_dp(  T_Eq_6bands_DP_Data  _XMEM *data,  T_Eq_6bands_Coefs  _YMEM *coefs );
void eq_static_7bands_Nch_dp(  T_Eq_7bands_DP_Data  _XMEM *data,  T_Eq_7bands_Coefs  _YMEM *coefs );
void eq_static_8bands_Nch_dp(  T_Eq_8bands_DP_Data  _XMEM *data,  T_Eq_8bands_Coefs  _YMEM *coefs );
void eq_static_9bands_Nch_dp(  T_Eq_9bands_DP_Data  _XMEM *data,  T_Eq_9bands_Coefs  _YMEM *coefs );
void eq_static_10bands_Nch_dp( T_Eq_10bands_DP_Data _XMEM *data,  T_Eq_10bands_Coefs _YMEM *coefs );
void eq_static_11bands_Nch_dp( T_Eq_11bands_DP_Data _XMEM *data,  T_Eq_11bands_Coefs _YMEM *coefs );
void eq_static_12bands_Nch_dp( T_Eq_12bands_DP_Data _XMEM *data,  T_Eq_12bands_Coefs _YMEM *coefs );
void eq_static_13bands_Nch_dp( T_Eq_13bands_DP_Data _XMEM *data,  T_Eq_13bands_Coefs _YMEM *coefs );
void eq_static_14bands_Nch_dp( T_Eq_14bands_DP_Data _XMEM *data,  T_Eq_14bands_Coefs _YMEM *coefs );
void eq_static_15bands_Nch_dp( T_Eq_15bands_DP_Data _XMEM *data,  T_Eq_15bands_Coefs _YMEM *coefs );
void eq_static_16bands_Nch_dp( T_Eq_16bands_DP_Data _XMEM *data,  T_Eq_16bands_Coefs _YMEM *coefs );

//---------------------------------------------------------------------------
//  EQUALIZER SP
//---------------------------------------------------------------------------

//Note: The following structs are for 1 ch. For Nch, use a contigous array of Data. (Coefs are shared for all channels)

//EQ N-band SP DATA (1 ch)
typedef struct { fraction xn;  T_Biquad_Optim_SP_Chained_Data data[1];}   T_Eq_1band_SP_Data;
typedef struct { fraction xn;  T_Biquad_Optim_SP_Chained_Data data[2];}   T_Eq_2bands_SP_Data;
typedef struct { fraction xn;  T_Biquad_Optim_SP_Chained_Data data[3];}   T_Eq_3bands_SP_Data;
typedef struct { fraction xn;  T_Biquad_Optim_SP_Chained_Data data[4];}   T_Eq_4bands_SP_Data;
typedef struct { fraction xn;  T_Biquad_Optim_SP_Chained_Data data[5];}   T_Eq_5bands_SP_Data;
typedef struct { fraction xn;  T_Biquad_Optim_SP_Chained_Data data[6];}   T_Eq_6bands_SP_Data;
typedef struct { fraction xn;  T_Biquad_Optim_SP_Chained_Data data[7];}   T_Eq_7bands_SP_Data;
typedef struct { fraction xn;  T_Biquad_Optim_SP_Chained_Data data[8];}   T_Eq_8bands_SP_Data;
typedef struct { fraction xn;  T_Biquad_Optim_SP_Chained_Data data[9];}   T_Eq_9bands_SP_Data;
typedef struct { fraction xn;  T_Biquad_Optim_SP_Chained_Data data[10];}  T_Eq_10bands_SP_Data;
typedef struct { fraction xn;  T_Biquad_Optim_SP_Chained_Data data[11];}  T_Eq_11bands_SP_Data;
typedef struct { fraction xn;  T_Biquad_Optim_SP_Chained_Data data[12];}  T_Eq_12bands_SP_Data;
typedef struct { fraction xn;  T_Biquad_Optim_SP_Chained_Data data[13];}  T_Eq_13bands_SP_Data;
typedef struct { fraction xn;  T_Biquad_Optim_SP_Chained_Data data[14];}  T_Eq_14bands_SP_Data;
typedef struct { fraction xn;  T_Biquad_Optim_SP_Chained_Data data[15];}  T_Eq_15bands_SP_Data;
typedef struct { fraction xn;  T_Biquad_Optim_SP_Chained_Data data[16];}  T_Eq_16bands_SP_Data;

//EQ N-band SP COEFS
typedef struct { int numCh, numBands;  T_Biquad_Optim_SP_Coefs  coefs[1];}      T_Eq_1band_SP_Coefs;
typedef struct { int numCh, numBands;  T_Biquad_Optim_SP_Coefs  coefs[2];}      T_Eq_2bands_SP_Coefs;
typedef struct { int numCh, numBands;  T_Biquad_Optim_SP_Coefs  coefs[3];}      T_Eq_3bands_SP_Coefs;
typedef struct { int numCh, numBands;  T_Biquad_Optim_SP_Coefs  coefs[4];}      T_Eq_4bands_SP_Coefs;
typedef struct { int numCh, numBands;  T_Biquad_Optim_SP_Coefs  coefs[5];}      T_Eq_5bands_SP_Coefs;
typedef struct { int numCh, numBands;  T_Biquad_Optim_SP_Coefs  coefs[6];}      T_Eq_6bands_SP_Coefs;
typedef struct { int numCh, numBands;  T_Biquad_Optim_SP_Coefs  coefs[7];}      T_Eq_7bands_SP_Coefs;
typedef struct { int numCh, numBands;  T_Biquad_Optim_SP_Coefs  coefs[8];}      T_Eq_8bands_SP_Coefs;
typedef struct { int numCh, numBands;  T_Biquad_Optim_SP_Coefs  coefs[9];}      T_Eq_9bands_SP_Coefs;
typedef struct { int numCh, numBands;  T_Biquad_Optim_SP_Coefs  coefs[10];}     T_Eq_10bands_SP_Coefs;
typedef struct { int numCh, numBands;  T_Biquad_Optim_SP_Coefs  coefs[11];}     T_Eq_11bands_SP_Coefs;
typedef struct { int numCh, numBands;  T_Biquad_Optim_SP_Coefs  coefs[12];}     T_Eq_12bands_SP_Coefs;
typedef struct { int numCh, numBands;  T_Biquad_Optim_SP_Coefs  coefs[13];}     T_Eq_13bands_SP_Coefs;
typedef struct { int numCh, numBands;  T_Biquad_Optim_SP_Coefs  coefs[14];}     T_Eq_14bands_SP_Coefs;
typedef struct { int numCh, numBands;  T_Biquad_Optim_SP_Coefs  coefs[15];}     T_Eq_15bands_SP_Coefs;
typedef struct { int numCh, numBands;  T_Biquad_Optim_SP_Coefs  coefs[16];}     T_Eq_16bands_SP_Coefs;


void eq_static_1band_Nch_sp(  T_Eq_1band_SP_Data *data,     T_Eq_1band_SP_Coefs  _YMEM *coefs );
void eq_static_2bands_Nch_sp( T_Eq_2bands_SP_Data *data,    T_Eq_2bands_SP_Coefs _YMEM *coefs );
void eq_static_3bands_Nch_sp( T_Eq_3bands_SP_Data *data,    T_Eq_3bands_SP_Coefs _YMEM *coefs );
void eq_static_4bands_Nch_sp( T_Eq_4bands_SP_Data *data,    T_Eq_4bands_SP_Coefs _YMEM *coefs );
void eq_static_5bands_Nch_sp( T_Eq_5bands_SP_Data *data,    T_Eq_5bands_SP_Coefs _YMEM *coefs );
void eq_static_6bands_Nch_sp( T_Eq_6bands_SP_Data *data,    T_Eq_6bands_SP_Coefs _YMEM *coefs );
void eq_static_7bands_Nch_sp( T_Eq_7bands_SP_Data *data,    T_Eq_7bands_SP_Coefs _YMEM *coefs );
void eq_static_8bands_Nch_sp( T_Eq_8bands_SP_Data *data,    T_Eq_8bands_SP_Coefs _YMEM *coefs );
void eq_static_9bands_Nch_sp( T_Eq_9bands_SP_Data *data,    T_Eq_9bands_SP_Coefs _YMEM *coefs );
void eq_static_10bands_Nch_sp( T_Eq_10bands_SP_Data *data,  T_Eq_10bands_SP_Coefs _YMEM *coefs );
void eq_static_11bands_Nch_sp( T_Eq_11bands_SP_Data *data,  T_Eq_11bands_SP_Coefs _YMEM *coefs );
void eq_static_12bands_Nch_sp( T_Eq_12bands_SP_Data *data,  T_Eq_12bands_SP_Coefs _YMEM *coefs );
void eq_static_13bands_Nch_sp( T_Eq_13bands_SP_Data *data,  T_Eq_13bands_SP_Coefs _YMEM *coefs );
void eq_static_14bands_Nch_sp( T_Eq_14bands_SP_Data *data,  T_Eq_14bands_SP_Coefs _YMEM *coefs );
void eq_static_15bands_Nch_sp( T_Eq_15bands_SP_Data *data,  T_Eq_15bands_SP_Coefs _YMEM *coefs );
void eq_static_16bands_Nch_sp( T_Eq_16bands_SP_Data *data,  T_Eq_16bands_SP_Coefs _YMEM *coefs );

//---------------------------------------------------------------------------
//  SPECTRUM VIEWER
//---------------------------------------------------------------------------

//Note:
//Each band of the spectrum viewer may contain several cascaded biquads for sharper filtering.
//The number of biquads per band is fixed (for the inner hw loop)
//The following structs are for 1 band. For Nband, use contigous arrays for Data and Coefs

//data
typedef struct {T_Biquad_Optim_SP_Data biqData[1];}                      T_SpectrumViewer_1biq_SP_Data;

//coef
typedef struct { int numBands;  T_Biquad_Optim_SP_Coefs  biqCoefs[1];}   T_SpectrumViewer_1biq_SP_Coefs;


//---------------------------------------------------------------------------
//  COMPANDER
//---------------------------------------------------------------------------

typedef struct a_CompanderGenericData    T_CompanderGenericData;
typedef struct a_CompanderMonoData       T_CompanderMonoData;
typedef struct a_CompanderStereoData     T_CompanderStereoData;
typedef struct a_CompanderQuadroData     T_CompanderQuadroData;
typedef struct a_Compander6ChData        T_Compander6ChData;

typedef struct a_ComprBaseCoeffs         T_ComprBaseCoeffs;
typedef struct a_ComprSectionCoeffs      T_ComprSectionCoeffs;
typedef struct a_CompressorCoeffs        T_CompressorCoeffs;
typedef struct a_CompanderCoeffs         T_CompanderCoeffs;

void compander_mono_init(T_CompanderMonoData _XMEM * data, T_CompanderCoeffs _YMEM * coeffs);
void compander_stereo_init(T_CompanderStereoData _XMEM * data, T_CompanderCoeffs _YMEM * coeffs);
void compander_quadro_init(T_CompanderQuadroData _XMEM * data, T_CompanderCoeffs _YMEM * coeffs);
void compander_6ch_init(T_Compander6ChData _XMEM * data, T_CompanderCoeffs _YMEM * coeffs);

//---------------------------------------------------------------------------
//  LIMITER
//---------------------------------------------------------------------------

//already defined in audio_limiter.h
/*
typedef struct a_LimiterData             T_LimiterData;
typedef struct a_LimiterMonoData         T_LimiterMonoData;
typedef struct a_LimiterStereoData       T_LimiterStereoData;
typedef struct a_LimiterQuadroData       T_LimiterQuadroData;
typedef struct a_Limiter6ChData          T_Limiter6ChData;

typedef struct a_LimiterCoeffs           T_LimiterCoeffs;
*/
/*
void limiter_mono_init(T_LimiterMonoData _XMEM * data, T_LimiterCoeffs _YMEM * coeffs);
void limiter_stereo_init(T_LimiterStereoData _XMEM * data, T_LimiterCoeffs _YMEM * coeffs);
void limiter_quadro_init(T_LimiterQuadroData _XMEM * data, T_LimiterCoeffs _YMEM * coeffs);
void limiter_6ch_init(T_Limiter6ChData _XMEM * data, T_LimiterCoeffs _YMEM * coeffs);

//previous decl
void audio_limiter_update(T_LimiterCoeffs _YMEM * coeffs, T_LimiterData _XMEM * data);
*/
//---------------------------------------------------------------------------
//  DELAY LINE
//---------------------------------------------------------------------------
//types are declared in "audio_delay_line.h"

//long delay buf in Ymem
void audio_delay_line_y_2ch(T_DelayData    _XMEM *data, T_DelayYParam    _YMEM *params);
void audio_delay_line_y_4ch(T_Delay4chData _XMEM *data, T_DelayY4chParam _YMEM *params);
void audio_delay_line_y_6ch(T_Delay6chData _XMEM *data, T_DelayY6chParam _YMEM *params);

//long delay buf in Xmem
void audio_delay_line_x_2ch(T_DelayData    _XMEM *data, T_DelayXParam    _YMEM *params);
void audio_delay_line_x_4ch(T_Delay4chData _XMEM *data, T_DelayX4chParam _YMEM *params);
void audio_delay_line_x_6ch(T_Delay6chData _XMEM *data, T_DelayX6chParam _YMEM *params);

//TODO: short delay buf X and Y....


//---------------------------------------------------------------------------
//  SINEWAGE GENERATOR
//---------------------------------------------------------------------------
typedef struct a_SinewaveData           T_SinewaveData;
typedef struct a_SinewaveCoeffs         T_SinewaveCoeffs;

//---------------------------------------------------------------------------
//  MUX
//---------------------------------------------------------------------------
#define MUX_MAX_INS     18
#define MUX_MAX_OUT     10
//note: only MUX_MAX_OUT has an impact on the number of cy,
//let's fix MUX_MAX_IN = 18 for all MUX types.

//X data
typedef struct { fraction ins[MUX_MAX_INS], out[2]; }          T_Mux2outData;
typedef struct { fraction ins[MUX_MAX_INS], out[4]; }          T_Mux4outData;
typedef struct { fraction ins[MUX_MAX_INS], out[6]; }          T_Mux6outData;
typedef struct { fraction ins[MUX_MAX_INS], out[8]; }          T_Mux8outData;
typedef struct { fraction ins[MUX_MAX_INS], out[10];}          T_Mux10outData;
typedef struct { fraction ins[MUX_MAX_INS], out[MUX_MAX_OUT];} T_MuxData;

//Y params
typedef struct { unsigned int sel[2];}            T_Mux2outParam;
typedef struct { unsigned int sel[4];}            T_Mux4outParam;
typedef struct { unsigned int sel[6];}            T_Mux6outParam;
typedef struct { unsigned int sel[8];}            T_Mux8outParam;
typedef struct { unsigned int sel[10];}           T_Mux10outParam;
typedef struct { unsigned int sel[MUX_MAX_OUT];}  T_MuxParam;

void mux_2out( T_Mux2outData  _XMEM *data, T_Mux2outParam  _YMEM *params);
void mux_4out( T_Mux4outData  _XMEM *data, T_Mux4outParam  _YMEM *params);
void mux_6out( T_Mux6outData  _XMEM *data, T_Mux6outParam  _YMEM *params);
void mux_8out( T_Mux8outData  _XMEM *data, T_Mux8outParam  _YMEM *params);
void mux_10out(T_Mux10outData _XMEM *data, T_Mux10outParam _YMEM *params);

//---------------------------------------------------------------------------
//  PCM CHIME / Clic-clac
//---------------------------------------------------------------------------

typedef struct {
    fraction out[2]; //left and right
    unsigned int  *data;
    unsigned int  *leftRead;
    unsigned int  *rightRead;
    unsigned int  *end;
} T_PCMChime_12bit_X_Data;

typedef struct {
    fraction out[2]; //left and right
    unsigned int _YMEM *data;
    unsigned int _YMEM *leftRead;
    unsigned int _YMEM *rightRead;
    unsigned int _YMEM *end;
} T_PCMChime_12bit_Y_Data;

//types seen from ARM are the same
//#define T_PCMChime_12bit_Data  T_PCMChime_12bit_Y_Data
typedef T_PCMChime_12bit_Y_Data  T_PCMChime_12bit_Data;

/*
typedef struct {
    int playCount;                // < 0 means infinit loop
    unsigned int muteCount;
    unsigned int postMuteDuration;
    unsigned int _YMEM *data;
    unsigned int _YMEM *read;
    unsigned int _YMEM *end;
} T_PCMChime_12bit_Params;
*/

typedef struct {
    int playCount;                // < 0 means infinit loop
    unsigned int leftMuteCount;
    unsigned int leftPreMuteDuration;
    unsigned int leftPostMuteDuration;
    unsigned int rightMuteCount;
    unsigned int rightPreMuteDuration;
    unsigned int rightPostMuteDuration;
    unsigned int endOfCycle;
} T_PCMChime_12bit_Params;


void pcmchime_12bit_y( T_PCMChime_12bit_Y_Data  *d,  T_PCMChime_12bit_Params _YMEM *p );
void pcmchime_12bit_x( T_PCMChime_12bit_X_Data  *d,  T_PCMChime_12bit_Params _YMEM *p );

//---------------------------------------------------------------------------
//  POLY CHIME
//---------------------------------------------------------------------------
#define POLYCHIME_RAMP_STATIC  0
#define POLYCHIME_RAMP_EXP     1
#define POLYCHIME_RAMP_LIN     2
//#define POLYCHIME_RAMP_STOP  3

typedef struct {
    int            type;            //0:static,  1:linear,  2:exp
    fraction       target;          //linear target gain
    union {
    fraction       inc;             //lin ramp coef
    fraction       incFactor;       //exp ramp coef
    unsigned int   sampleDuration;  //static ramp duration
    } k;
   fraction        phase_inc;       //(=frequency setting) for sinewave generation
} T_Ramp;

typedef struct {
    fraction       out;             //chime output (mono)
    unsigned int   sampleCount;     //for internal duration count
    fraction       inc;             //for internal ramp coef update
    fraction       phase;           //current phase for the sinewave approximation
} T_PolyChime_Data;

typedef struct {
    fraction       gain;             //envelop gain (always <= 1)
    int            repeatSync;       //toggle at each repeat loop
    int            repeatCount;      //repeat loop counter
    void _YMEM     *pMasterParams;   //pointer to the master chime's params (for true polychime synchro)
    T_Ramp         stopRamp;         //fast stop ramp. Also holds the current frequency setting (phase_inc)
    T_Ramp _YMEM   *ramps;
    T_Ramp _YMEM   *curRamp;
    T_Ramp _YMEM   *lastRepeatRamp;
    T_Ramp _YMEM   *lastRamp;
} T_PolyChime_Params;

void polychime(T_PolyChime_Data *x,  T_PolyChime_Params _YMEM *y);


//---------------------------------------------------------------------------
//  SPECTRUM METER SP
//---------------------------------------------------------------------------

//------- DATA --------
//internal data 1band x 1biq
typedef struct {
    fraction                             xn;
    T_Biquad_Optim_SP_Chained_Data       biqData[1];
    int                                  peakdB;               // in Q8
    fraction                             peak;                 // (keep last for optim purpose)
} T_SpeMeter_1biq_Data;

//data 2bands x 1biq
typedef struct {
    T_SpeMeter_1biq_Data                 band[2];
} T_SpeMeter_2bands_1biq_Data;

//data Nband
typedef struct {
    T_SpeMeter_1biq_Data _YMEM           *band;   //in fact band[N]
    //must be followed in memory by the bands'data
} T_SpectrumMeter_Data;

//------- COEFS --------

//internal biquad coefs (1 biq only)
typedef struct {
    T_Biquad_Optim_SP_Coefs             biqCoefs[1];
}   T_SpeMeter_1biq_Coefs;

//coefs 2bands x 1biq
typedef struct {
    int                                 numBands;
    int                                 curBand;
    fraction                            decayFactor;
    fraction                            log2todBFactor;
    T_SpeMeter_1biq_Coefs               band[2];
}   T_SpeMeter_2bands_1biq_Coefs;

//coefs Nband
typedef struct {
    int                                 numBands;
    int                                 curBand;
    fraction                            decayFactor;
    fraction                            log2todBFactor;
    T_SpeMeter_1biq_Coefs _YMEM         *band;          //warning: in truth, to be seen as band[N]
    //must be followed in memory by the bands's coefs
}   T_SpectrumMeter_Coefs;


void spectrum_meter( T_SpectrumMeter_Data *data,  T_SpectrumMeter_Coefs _YMEM *coefs );

//---------------------------------------------
//when compiling ARM side audio lib
#ifndef EMERALDCC
#pragma pack(pop) //reset struct packing mode as before
#endif
//---------------------------------------------

#endif

