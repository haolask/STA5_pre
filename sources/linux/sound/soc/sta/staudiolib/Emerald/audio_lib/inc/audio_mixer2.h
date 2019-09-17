/***********************************************************************************/
/*!
*  \file      audio_mixer2.h
*
*  \brief     <i><b> Linear Smooth Mixer Block </b></i>
*
*  \details   Linear Smooth Mixer Block
*
*  \author    APG-MID Application Team
*
*  \author    (original version) Kwell YIN
*
*  \version   1.0
*
*  \date      2016.04.21
*
*  \bug       Unknown
*
*  \warning
*
*  This file is part of <component name> and is dual licensed,
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
* <component_name> is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* <component_name> is distributed in the hope that it will be
* useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* Please refer to <http://www.gnu.org/licenses/>.
*/
/***********************************************************************************/

#ifndef _MIXER2_H_
#define _MIXER2_H_

#define MIXER_MAX_CH	6
#define MIXER_MAX_INS	9

typedef struct {
    fraction _XMEM  *ins[MIXER_MAX_CH]; //pointers to insets
    fraction _XMEM  *out[MIXER_MAX_CH];
    fraction         initUpDownGain[3];
    unsigned int     numCh;
} T_MixerLinearNchData;

typedef struct{
	fraction gain[2];              // gain[0:1]
	fraction gainGoal[2];          // gainGoal[0:1]
	fraction gain_inc[2];          // gain_inc[0:1]
	fraction volume;               // volume
	fraction volumeGoal;           // volume goal
	fraction volume_inc;           // volume_inc
	int shift;                     // shift for output data
} T_MixerLinear2insParam;

typedef struct{
	fraction gain[3];   		// gain[0:1]
	fraction gainGoal[3];	        // gainGoal[0:1]
	fraction gain_inc[3];		// gain_inc[0:1]
	fraction volume;		// volume
	fraction volumeGoal;		// volume goal
	fraction volume_inc;		// volume_inc
	int shift;			// shift for output data
} T_MixerLinear3insParam;

typedef struct{
	fraction gain[4];   		// gain[0:1]
	fraction gainGoal[4];	        // gainGoal[0:1]
	fraction gain_inc[4];		// gain_inc[0:1]
	fraction volume;		// volume
	fraction volumeGoal;		// volume goal
	fraction volume_inc;		// volume_inc
	int shift;			// shift for output data
} T_MixerLinear4insParam;

typedef struct{
	fraction gain[5];   		// gain[0:1]
	fraction gainGoal[5];	        // gainGoal[0:1]
	fraction gain_inc[5];		// gain_inc[0:1]
	fraction volume;		// volume
	fraction volumeGoal;		// volume goal
	fraction volume_inc;		// volume_inc
	int shift;			// shift for output data
} T_MixerLinear5insParam;

typedef struct{
	fraction gain[6];   		// gain[0:1]
	fraction gainGoal[6];	        // gainGoal[0:1]
	fraction gain_inc[6];		// gain_inc[0:1]
	fraction volume;		// volume
	fraction volumeGoal;		// volume goal
	fraction volume_inc;		// volume_inc
	int shift;			// shift for output data
} T_MixerLinear6insParam;

typedef struct{
	fraction gain[7];   		// gain[0:1]
	fraction gainGoal[7];	        // gainGoal[0:1]
	fraction gain_inc[7];		// gain_inc[0:1]
	fraction volume;		// volume
	fraction volumeGoal;		// volume goal
	fraction volume_inc;		// volume_inc
	int shift;			// shift for output data
} T_MixerLinear7insParam;

typedef struct{
	fraction gain[8];   		// gain[0:1]
	fraction gainGoal[8];	        // gainGoal[0:1]
	fraction gain_inc[8];		// gain_inc[0:1]
	fraction volume;		// volume
	fraction volumeGoal;		// volume goal
	fraction volume_inc;		// volume_inc
	int shift;			// shift for output data
} T_MixerLinear8insParam;

typedef struct{
	fraction gain[9];   		// gain[0:1]
	fraction gainGoal[9];	        // gainGoal[0:1]
	fraction gain_inc[9];		// gain_inc[0:1]
	fraction volume;		// volume
	fraction volumeGoal;		// volume goal
	fraction volume_inc;		// volume_inc
	int shift;			// shift for output data
} T_MixerLinear9insParam;

typedef struct{T_MixerLinear2insParam mixerParamSet[MIXER_MAX_CH];}  T_MixerLinear2insNchParam;
typedef struct{T_MixerLinear3insParam mixerParamSet[MIXER_MAX_CH];}  T_MixerLinear3insNchParam;
typedef struct{T_MixerLinear4insParam mixerParamSet[MIXER_MAX_CH];}  T_MixerLinear4insNchParam;
typedef struct{T_MixerLinear5insParam mixerParamSet[MIXER_MAX_CH];}  T_MixerLinear5insNchParam;
typedef struct{T_MixerLinear6insParam mixerParamSet[MIXER_MAX_CH];}  T_MixerLinear6insNchParam;
typedef struct{T_MixerLinear7insParam mixerParamSet[MIXER_MAX_CH];}  T_MixerLinear7insNchParam;
typedef struct{T_MixerLinear8insParam mixerParamSet[MIXER_MAX_CH];}  T_MixerLinear8insNchParam;
typedef struct{T_MixerLinear9insParam mixerParamSet[MIXER_MAX_CH];}  T_MixerLinear9insNchParam;

// Adaptation to previous MIXER API

typedef T_MixerLinearNchData T_MixerNchData;

typedef T_MixerLinear2insNchParam T_Mixer2insNchParam;
typedef T_MixerLinear3insNchParam T_Mixer3insNchParam;
typedef T_MixerLinear4insNchParam T_Mixer4insNchParam;
typedef T_MixerLinear5insNchParam T_Mixer5insNchParam;
typedef T_MixerLinear6insNchParam T_Mixer6insNchParam;
typedef T_MixerLinear7insNchParam T_Mixer7insNchParam;
typedef T_MixerLinear8insNchParam T_Mixer8insNchParam;
typedef T_MixerLinear9insNchParam T_Mixer9insNchParam;

typedef T_MixerLinear2insParam T_Mixer2insParam;
typedef T_MixerLinear3insParam T_Mixer3insParam;
typedef T_MixerLinear4insParam T_Mixer4insParam;
typedef T_MixerLinear5insParam T_Mixer5insParam;
typedef T_MixerLinear6insParam T_Mixer6insParam;
typedef T_MixerLinear7insParam T_Mixer7insParam;
typedef T_MixerLinear8insParam T_Mixer8insParam;
typedef T_MixerLinear9insParam T_Mixer9insParam;

// Prototypes

void audio_mixer_linear_2ins_Nch(T_MixerLinearNchData _XMEM *data, T_MixerLinear2insNchParam _YMEM *params);
void audio_mixer_linear_2ins_Nch_init( T_MixerLinearNchData _XMEM *data, T_MixerLinear2insNchParam _YMEM *params );

void audio_mixer_linear_3ins_Nch(T_MixerLinearNchData _XMEM *data, T_MixerLinear3insNchParam _YMEM *params);
void audio_mixer_linear_3ins_Nch_init( T_MixerLinearNchData _XMEM *data, T_MixerLinear3insNchParam _YMEM *params );

void audio_mixer_linear_4ins_Nch(T_MixerLinearNchData _XMEM *data, T_MixerLinear4insNchParam _YMEM *params);
void audio_mixer_linear_4ins_Nch_init( T_MixerLinearNchData _XMEM *data, T_MixerLinear4insNchParam _YMEM *params );

void audio_mixer_linear_5ins_Nch(T_MixerLinearNchData _XMEM *data, T_MixerLinear5insNchParam _YMEM *params);
void audio_mixer_linear_5ins_Nch_init( T_MixerLinearNchData _XMEM *data, T_MixerLinear5insNchParam _YMEM *params );

void audio_mixer_linear_6ins_Nch(T_MixerLinearNchData _XMEM *data, T_MixerLinear6insNchParam _YMEM *params);
void audio_mixer_linear_6ins_Nch_init( T_MixerLinearNchData _XMEM *data, T_MixerLinear6insNchParam _YMEM *params );

void audio_mixer_linear_7ins_Nch(T_MixerLinearNchData _XMEM *data, T_MixerLinear7insNchParam _YMEM *params);
void audio_mixer_linear_7ins_Nch_init( T_MixerLinearNchData _XMEM *data, T_MixerLinear7insNchParam _YMEM *params );

void audio_mixer_linear_8ins_Nch(T_MixerLinearNchData _XMEM *data, T_MixerLinear8insNchParam _YMEM *params);
void audio_mixer_linear_8ins_Nch_init( T_MixerLinearNchData _XMEM *data, T_MixerLinear8insNchParam _YMEM *params );

void audio_mixer_linear_9ins_Nch(T_MixerLinearNchData _XMEM *data, T_MixerLinear9insNchParam _YMEM *params);
void audio_mixer_linear_9ins_Nch_init( T_MixerLinearNchData _XMEM *data, T_MixerLinear9insNchParam _YMEM *params );

#endif

