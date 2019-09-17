/***********************************************************************************/
/*!
*  \file      audio_mixer.h
*
*  \brief     <i><b> Mixer Block </b></i>
*
*  \details   Mixer Block
*
*  \author    APG-MID Application Team
*
*  \author    (original version) Jan Ingerle
*
*  \version   1.0
*
*  \date      2009.11.23
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

#ifndef _MIXER_H_
#define _MIXER_H_

// -------------------- Defs ---------------------------------

#define GAIN_NO_ACTION 0x000000
#define GAIN_UP 0x000001
#define GAIN_DOWN 0x000002

#define MIXER_MAX_CH	6
#define MIXER_MAX_INS	9

// -------------------- Type Defs ---------------------------------

//----------
//1ch mixer
//----------
typedef struct{
    fraction gainGoal[4];	    //in[0:1], softmute, volume
    fraction gain[4];   		//in[0:1], softmute, volume
    int shift;
    fraction gainUpCoef[4];
    fraction gainDownCoef[4];
    fraction outFixGain[2];
    fraction hardMute;
} T_Mixer2insParam;

typedef struct{
    fraction gainGoal[5];
    fraction gain[5];
    int shift;
    fraction gainUpCoef[5];
    fraction gainDownCoef[5];
    fraction outFixGain[2];
    fraction hardMute;
} T_Mixer3insParam;

typedef struct{
   fraction gainGoal[6];
   fraction gain[6];
   int shift;
   fraction gainUpCoef[6];
   fraction gainDownCoef[6];
   fraction outFixGain[2];
   fraction hardMute;
} T_FourMixerParam;

typedef struct{
   fraction gainGoal[7];
   fraction gain[7];
   int shift;
   fraction gainUpCoef[7];
   fraction gainDownCoef[7];
   fraction outFixGain[2];
   fraction hardMute;
} T_FiveMixerParam;

typedef struct{
   fraction gainGoal[8];
   fraction gain[8];
   int shift;
   fraction gainUpCoef[8];
   fraction gainDownCoef[8];
   fraction outFixGain[2];
   fraction hardMute;
} T_SixMixerParam;

typedef struct{
   fraction gainGoal[9];
   fraction gain[9];
   int shift;
   fraction gainUpCoef[9];
   fraction gainDownCoef[9];
   fraction outFixGain[2];
   fraction hardMute;
} T_SevenMixerParam;

typedef struct{
   fraction gainGoal[10];
   fraction gain[10];
   int shift;
   fraction gainUpCoef[10];
   fraction gainDownCoef[10];
   fraction outFixGain[2];
   fraction hardMute;
} T_EightMixerParam;

typedef struct{
   fraction gainGoal[11];
   fraction gain[11];
   int shift;
   fraction gainUpCoef[11];
   fraction gainDownCoef[11];
   fraction outFixGain[2];
   fraction hardMute;
} T_Mixer9insParam;

typedef T_FourMixerParam        T_Mixer4insParam;
typedef T_FiveMixerParam        T_Mixer5insParam;
typedef T_SixMixerParam         T_Mixer6insParam;
typedef T_SevenMixerParam       T_Mixer7insParam;
typedef T_EightMixerParam       T_Mixer8insParam;


typedef struct{
   fraction _XMEM *ins;
   fraction _XMEM *out;
} T_MixerData;

typedef struct{
   T_FiveMixerParam mixerParamSet[6];
} T_FiveMixer6chParam;

typedef struct{
   T_SixMixerParam mixerParamSet[6];
} T_SixMixer6chParam;

typedef struct{
   T_SevenMixerParam mixerParamSet[6];
} T_SevenMixer6chParam;

typedef struct{
   T_EightMixerParam mixerParamSet[6];
} T_EightMixer6chParam;

typedef struct{
   fraction _XMEM *ins[6];
   fraction _XMEM *out[6];
} T_Mixer6chData;

typedef fraction T_FourMixerIns[4];
typedef fraction T_FiveMixerIns[5];
typedef fraction T_SixMixerIns[6];
typedef fraction T_SevenMixerIns[7];
typedef fraction T_EightMixerIns[8];

//----------
//Nch mixer
//----------

//Nch mixer params
typedef struct{T_Mixer2insParam mixerParamSet[MIXER_MAX_CH];}  T_Mixer2insNchParam;
typedef struct{T_Mixer3insParam mixerParamSet[MIXER_MAX_CH];}  T_Mixer3insNchParam;
typedef struct{T_Mixer4insParam mixerParamSet[MIXER_MAX_CH];}  T_Mixer4insNchParam;
typedef struct{T_Mixer5insParam mixerParamSet[MIXER_MAX_CH];}  T_Mixer5insNchParam;
typedef struct{T_Mixer6insParam mixerParamSet[MIXER_MAX_CH];}  T_Mixer6insNchParam;
typedef struct{T_Mixer7insParam mixerParamSet[MIXER_MAX_CH];}  T_Mixer7insNchParam;
typedef struct{T_Mixer8insParam mixerParamSet[MIXER_MAX_CH];}  T_Mixer8insNchParam;
typedef struct{T_Mixer9insParam mixerParamSet[MIXER_MAX_CH];}  T_Mixer9insNchParam;

//Nch mixer data
//This struct is compatible with Audio_lib
typedef struct {
    fraction _XMEM  *ins[MIXER_MAX_CH]; //pointers to insets
    fraction _XMEM  *out[MIXER_MAX_CH];
    fraction         initUpDownGain[3];
    unsigned int     numCh;
} T_MixerNchData;


// -------------------- Memory Defs ---------------------------------


// -------------------- Function Prototypes -----------------------

void audio_mixer_2ins_Nch_init( T_MixerNchData _XMEM *data, T_Mixer2insNchParam _YMEM *params );
void audio_mixer_3ins_Nch_init( T_MixerNchData _XMEM *data, T_Mixer3insNchParam _YMEM *params );
void audio_mixer_4ins_Nch_init( T_MixerNchData _XMEM *data, T_Mixer4insNchParam _YMEM *params );
void audio_mixer_5ins_Nch_init( T_MixerNchData _XMEM *data, T_Mixer5insNchParam _YMEM *params );
void audio_mixer_6ins_Nch_init( T_MixerNchData _XMEM *data, T_Mixer6insNchParam _YMEM *params );
void audio_mixer_7ins_Nch_init( T_MixerNchData _XMEM *data, T_Mixer7insNchParam _YMEM *params );
void audio_mixer_8ins_Nch_init( T_MixerNchData _XMEM *data, T_Mixer8insNchParam _YMEM *params );
void audio_mixer_9ins_Nch_init( T_MixerNchData _XMEM *data, T_Mixer9insNchParam _YMEM *params );

void audio_mixer_2ins_Nch( T_MixerNchData _XMEM *data, T_Mixer2insNchParam _YMEM *params );
void audio_mixer_3ins_Nch( T_MixerNchData _XMEM *data, T_Mixer3insNchParam _YMEM *params );
void audio_mixer_4ins_Nch( T_MixerNchData _XMEM *data, T_Mixer4insNchParam _YMEM *params );
void audio_mixer_5ins_Nch( T_MixerNchData _XMEM *data, T_Mixer5insNchParam _YMEM *params );
void audio_mixer_6ins_Nch( T_MixerNchData _XMEM *data, T_Mixer6insNchParam _YMEM *params );
void audio_mixer_7ins_Nch( T_MixerNchData _XMEM *data, T_Mixer7insNchParam _YMEM *params );
void audio_mixer_8ins_Nch( T_MixerNchData _XMEM *data, T_Mixer8insNchParam _YMEM *params );
void audio_mixer_9ins_Nch( T_MixerNchData _XMEM *data, T_Mixer9insNchParam _YMEM *params );

void audio_mixer_2ins_gain(T_Mixer2insParam _YMEM *params);
void audio_mixer_3ins_gain(T_Mixer3insParam _YMEM *params);



#endif
