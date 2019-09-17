/***********************************************************************************/
/*!
*  \file      audio_loudness.h
*
*  \brief     <i><b> Header file for loudness module </b></i>
*
*  \details   Header file for loudness module
*
*  \author    APG-MID Application Team
*
*  \author    (original version) Ondrej Trubac
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

#ifndef _LOUDNESS_H_

#define _LOUDNESS_H_

#include "biquad_dp.h"


typedef struct
{
    T_Biquad_DP_Stage_2 chan_0;
    T_Biquad_DP_Stage_2 chan_1;
}T_Dual_Loudness_Data;


typedef struct
{
    fraction gain_Direct;
    fraction gain_LP;
    fraction gain_HP;
    fraction gainGoal_Direct;
    fraction gainGoal_LP;
    fraction gainGoal_HP;
    fraction gainStepUp;
    fraction gainStepDown;
}T_Loudness_Params;

typedef struct
{
  T_Biquad_Coefs LP_coefs;
  T_Biquad_Coefs HP_coefs;   
}T_Loudness_Coefs;

typedef struct
{
    T_Biquad_Coefs  coefs[2];
    T_Loudness_Params params;
}T_Loudness_Config;


void loudness_static_dual_chained(void);

#endif 
