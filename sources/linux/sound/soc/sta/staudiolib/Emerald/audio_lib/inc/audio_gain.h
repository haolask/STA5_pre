/***********************************************************************************/
/*!
*  \file      audio_gain.h
*
*  \brief     <i><b> Gain Blocks </b></i>
*
*  \details   Gain Blocks
*
*  \author    APG-MID Application Team
*
*  \author    (original version) Jan Ingerle
*
*  \version   1.0
*
*  \date      2010.10.26
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

#ifndef _GAIN_H_
#define _GAIN_H_

// -------------------- Defs ---------------------------------
   
#define GAIN_NO_ACTION 0x000000
#define GAIN_UP 0x000001
#define GAIN_DOWN 0x000002

// -------------------- Type Defs ---------------------------------

typedef struct{
   fraction gainUp;
   fraction gainDown;
   fraction gainGoal;
   fraction gain;      
} T_SingleGainParams;

typedef struct{
   fraction gainUp;
   fraction gainDown;
   fraction gainGoal[6];
   fraction gain[6];      
} T_6ChanGainParams;


typedef struct{
    fraction gain;
    int scaling;
}T_SimpleGainParams;

typedef struct{
    fraction gain[4];
    int scaling[4];
}T_Simple4ChanGainParams;



typedef struct{
   fraction _XMEM *ins;
   fraction _XMEM *out;
} T_GainData;



// -------------------- Memory Defs ---------------------------------


// -------------------- Function Prototypes -----------------------
//void audio_gain_6ch_init(fraction *upDwnGain, T_SingleGainParams _YMEM *params);
//void audio_gain_4ch_init(fraction *upDwnGain, T_SingleGainParams _YMEM *params);
//void audio_gain_1ch_init(fraction *upDwnGain, T_SingleGainParams _YMEM *params);

void audio_gain_single(T_GainData _XMEM *data, fraction _YMEM *gain);
void audio_gain_6ch(T_GainData _XMEM *data, fraction _YMEM *gains);

void audio_gain_simple(T_GainData _XMEM *data, T_SimpleGainParams _YMEM *params);
void audio_gain_simple_4ch(T_GainData _XMEM *data, T_Simple4ChanGainParams _YMEM *params);

void audio_gain_6ch_smooth_update(T_6ChanGainParams _YMEM * params);
void audio_gain_smooth_update(T_SingleGainParams _YMEM * params);

#endif
