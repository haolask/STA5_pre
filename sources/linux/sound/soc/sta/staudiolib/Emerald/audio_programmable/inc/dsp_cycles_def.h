/**********************************************************************************/
/*!
*  \file      dsp_cycles_def.h
*
*  \brief     <i><b> DSP cycle costs definition </b></i>
*
*  \details   DSP cycle costs definition
*
*  \author    APG-MID Application Team
*
*  \author    (original version) Olivier Douvenot
*
*  \version   1.0
*
*  \date      2017/06/19
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
* Copyright (c) 2017 STMicroelectronics - All Rights Reserved
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

#ifndef DSP_CYCLES_DEF_H_
#define DSP_CYCLES_DEF_H_

/* Main loop costs */
#define MAIN_LOOP_DEFAULT_CYCLES                                                72
#define MAIN_LOOP_PER_MODULE_CYCLES                                             11


/* Connections costs */
#define PER_MONO_CONNECTION_CYCLES                                               7


/* Modules main costs */
#define MUX_2OUT_MAIN_CYCLES                                                    13
#define MUX_4OUT_MAIN_CYCLES                                                    22
#define MUX_6OUT_MAIN_CYCLES                                                    28
#define MUX_8OUT_MAIN_CYCLES                                                    34
#define MUX_10OUT_MAIN_CYCLES                                                   40

#define PEAKDETECT_NCH_MAIN_CYCLES                                              14
#define PEAKDETECT_NCH_PER_CH_CYCLES                                            13

#define GAIN_STATIC_NCH_MAIN_CYCLES                                             15
#define GAIN_STATIC_NCH_PER_STEREO_CH_CYCLES                                     7

#define GAIN_SMOOTH_NCH_MAIN_CYCLES                                             15
#define GAIN_SMOOTH_NCH_PER_CH_CYCLES                                           20

#define GAIN_LINEAR_NCH_MAIN_CYCLES                                             17
#define GAIN_LINEAR_NCH_PER_CH_CYCLES                                           14

#define CD_DEEMPH_2CH_MAIN_CYCLES                                               86

#define LOUDNESS_DP_1CH_MAIN_CYCLES                                             79
#define LOUDNESS_DP_2CH_MAIN_CYCLES                                            159

#define TONE_DP_1CH_MAIN_CYCLES                                                115
#define TONE_DP_2CH_MAIN_CYCLES                                                231

#define EQ_DP_MAIN_CYCLES                                                       10
#define EQ_DP_PER_CH_CYCLES                                                      7
#define EQ_DP_PER_BAND_PER_CH_CYCLES                                            22

#define EQ_SP_MAIN_CYCLES                                                       10
#define EQ_SP_PER_CH_CYCLES                                                      7
#define EQ_SP_PER_BAND_PER_CH_CYCLES                                            10

#define DELAY_Y_2CH_MAIN_CYCLES                                                 56
#define DELAY_Y_4CH_MAIN_CYCLES                                                104
#define DELAY_Y_6CH_MAIN_CYCLES                                                152
#define DELAY_X_2CH_MAIN_CYCLES                                                 60
#define DELAY_X_4CH_MAIN_CYCLES                                                112
#define DELAY_X_6CH_MAIN_CYCLES                                                164

#define LIMITER_1CH_MAIN_CYCLES                                                 51
#define LIMITER_2CH_MAIN_CYCLES                                                 70
#define LIMITER_4CH_MAIN_CYCLES                                                106
#define LIMITER_6CH_MAIN_CYCLES                                                154

#define CLIP_LIMITER_NCH_MAIN_CYCLES                                           109
#define CLIP_LIMITER_NCH_PER_CH_CYCLES                                           8

#define COMPANDER_1CH_MAIN_CYCLES                                               58
#define COMPANDER_2CH_MAIN_CYCLES                                               73
#define COMPANDER_4CH_MAIN_CYCLES                                               92
#define COMPANDER_6CH_MAIN_CYCLES                                              135

#define MIXER3_MAIN_CYCLES                                                      21
#define MIXER3_PER_CH_CYCLES                                                    24
#define MIXER3_PER_INPUT_PER_CH_CYCLES                                          12

#define SINE_2CH_MAIN_CYCLES                                                    42

#define CHIMEGEN_MAIN_CYCLES                                                   120

#define PCMCHIME_12BIT_Y_2CH_MAIN_CYCLES                                        79
#define PCMCHIME_12BIT_X_2CH_MAIN_CYCLES                                        99

#define SPECMTR_NBANDS_1BIQ_MAIN_CYCLES                                         33
#define SPECMTR_NBANDS_1BIQ_PER_BAND_CYCLES                                     21

#define BITSHIFTER_NCH_MAIN_CYCLES                                              15
#define BITSHIFTER_NCH_PER_CH_CYCLES                                             6


/* Modules update costs */
#define LIMITER_UPDATE_CYCLES                                                   70

#define COMPANDER_UPDATE_CYCLES                                                 66



#endif /* DSP_CYCLES_DEF_H_ */

