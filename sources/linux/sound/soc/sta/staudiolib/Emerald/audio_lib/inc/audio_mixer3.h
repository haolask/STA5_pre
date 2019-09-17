/***********************************************************************************/
/*!
*  \file      audio_mixer3.h
*
*  \brief     <i><b> Linear Smooth Mixer Block </b></i>
*
*  \details   Linear Smooth Mixer Block
*
*  \author    APG-MID Application Team
*
*  \author    (original version) Christophe Quarre
*
*  \version   1.0
*
*  \date      2016.11.01
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

#ifndef _MIXER3_H_
#define _MIXER3_H_

/* 
MIXER3 (smooth): 
- input gains are linear, and only < 0 dB
- output gains are exponential and can be > 0 dB
*/

#define MIXER_MAX_INS      9   /* max inputs per channel */
#define MIXER_MAX_CH       6   /* max output channels */

#define TEST                     0


//------ X data ------

typedef struct {
    fraction      *inset;
    fraction       out;
} T_Mixer3_1chData;

typedef struct {
    int               sync;
    int               numCh;
    T_Mixer3_1chData  chData[MIXER_MAX_CH];
} T_Mixer3_NchData;

//------ Y params ------

//input gains are linear < 1.0
typedef struct {
    fraction          gain;
    fraction          target;
    fraction          inc;            //for linear
//  fraction          incFactor;      //for exponential
} T_Mixer3_inGain;

//output gains are exponential and can be > 1
typedef struct {
    fraction          gain;
    fraction          target;
    int               lshift;
    fraction          inc;            //for linear
    fraction          incFactor;      //for exponential
} T_Mixer3_outGain;

typedef struct {
    T_Mixer3_inGain    inGains[2];
    T_Mixer3_outGain   outGain;
} T_Mixer3_2in1chParams;

typedef struct {
    T_Mixer3_inGain    inGains[3];
    T_Mixer3_outGain   outGain;
} T_Mixer3_3in1chParams;

typedef struct {
    T_Mixer3_inGain    inGains[4];
    T_Mixer3_outGain   outGain;
} T_Mixer3_4in1chParams;

typedef struct {
    T_Mixer3_inGain    inGains[5];
    T_Mixer3_outGain   outGain;
} T_Mixer3_5in1chParams;

typedef struct {
    T_Mixer3_inGain    inGains[6];
    T_Mixer3_outGain   outGain;
} T_Mixer3_6in1chParams;

typedef struct {
    T_Mixer3_inGain    inGains[7];
    T_Mixer3_outGain   outGain;
} T_Mixer3_7in1chParams;

typedef struct {
    T_Mixer3_inGain    inGains[8];
    T_Mixer3_outGain   outGain;
} T_Mixer3_8in1chParams;

typedef struct {
    T_Mixer3_inGain    inGains[9];
    T_Mixer3_outGain   outGain;
} T_Mixer3_9in1chParams;


typedef struct {T_Mixer3_2in1chParams chParams[MIXER_MAX_CH];}  T_Mixer3_2inNchParams;
typedef struct {T_Mixer3_3in1chParams chParams[MIXER_MAX_CH];}  T_Mixer3_3inNchParams;
typedef struct {T_Mixer3_4in1chParams chParams[MIXER_MAX_CH];}  T_Mixer3_4inNchParams;
typedef struct {T_Mixer3_5in1chParams chParams[MIXER_MAX_CH];}  T_Mixer3_5inNchParams;
typedef struct {T_Mixer3_6in1chParams chParams[MIXER_MAX_CH];}  T_Mixer3_6inNchParams;
typedef struct {T_Mixer3_7in1chParams chParams[MIXER_MAX_CH];}  T_Mixer3_7inNchParams;
typedef struct {T_Mixer3_8in1chParams chParams[MIXER_MAX_CH];}  T_Mixer3_8inNchParams;
typedef struct {T_Mixer3_9in1chParams chParams[MIXER_MAX_CH];}  T_Mixer3_9inNchParams;


void mixer3_2inNch(T_Mixer3_NchData *x,  T_Mixer3_2inNchParams _YMEM *y);
void mixer3_3inNch(T_Mixer3_NchData *x,  T_Mixer3_3inNchParams _YMEM *y);
void mixer3_4inNch(T_Mixer3_NchData *x,  T_Mixer3_4inNchParams _YMEM *y);
void mixer3_5inNch(T_Mixer3_NchData *x,  T_Mixer3_5inNchParams _YMEM *y);
void mixer3_6inNch(T_Mixer3_NchData *x,  T_Mixer3_6inNchParams _YMEM *y);
void mixer3_7inNch(T_Mixer3_NchData *x,  T_Mixer3_7inNchParams _YMEM *y);
void mixer3_8inNch(T_Mixer3_NchData *x,  T_Mixer3_8inNchParams _YMEM *y);
void mixer3_9inNch(T_Mixer3_NchData *x,  T_Mixer3_9inNchParams _YMEM *y);

#endif

