/***********************************************************************************/
/*!
*  \file      audio_lib.h
*
*  \brief     <i><b> Global Header File for Audio Modules </b></i>
*
*  \details   Global Header File for Audio Modules
*
*  \author    APG-MID Application Team
*
*  \author    (original version) Jan Ingerle
*
*  \version   1.0
*
*  \date      2007.07.17
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

#ifndef _AUDIO_LIB_H_
#define _AUDIO_LIB_H_

/*************************************************************************/
/*                            Type Dedinitions                           */ 
/*************************************************************************/

/* Audio Library signals */
typedef struct {
  struct a_Jingle2EnvDualData   _XMEM * p_jingle_data;            // memory cell for storing data pointer for jingle generator (2 pathes)
  struct a_Jingle3EnvDualCoeffs _YMEM * p_jingle_coeffs3;         // memory cell for storing coeff pointer for jingle generator (3 pathes)    struct a_Jingle3EnvDualCoeffs _YMEM * p_jingle_coeffs3;         // memory cell for storing coeff pointer for jingle generator (3 pathes)    struct a_Jingle3EnvDualCoeffs _YMEM * p_jingle_coeffs3;         // memory cell for storing coeff pointer for jingle generator (3 pathes)  
  struct a_Jingle3EnvDualData   _XMEM * p_jingle_data3;           // memory cell for storing data pointer for jingle generator (3 pathes)
  struct a_Jingle2EnvDualCoeffs _YMEM * p_jingle_coeffs;          // memory cell for storing coeff pointer for jingle generator (2 pathes)  
} T_AudioLib_Signals;

/* Audio Library Parameters */
typedef struct {
  fraction cd_demph_fltr_coeffs[3];                        // global constant for deemphasis filter coeffs
  fraction ar_sin_c_a11[6];                                // global constant for sin(x) approximation (A11)  
  fraction ar_y_ld_coeffs[3];                              // global constant for log2(x) approximation  
  fraction min_pos;                                        // global constant for (+0.000000119209)
  fraction ar_sin_c_a9[5];                                 // global constant for sin(x) approximation (A9)
  fraction div_6;                                          // global constant for (+1/6)   
  fraction min_neg;                                        // global constant for (-0.000000119209)
} T_AudioLib_Params;

/*************************************************************************/
/*                         X memory declaration                          */ 
/*************************************************************************/
/*
extern struct a_Jingle2EnvDualData   _XMEM * p_jingle_data;            // memory cell for storing data pointer for jingle generator (2 pathes)
extern struct a_Jingle2EnvDualCoeffs _YMEM * p_jingle_coeffs;          // memory cell for storing coeff pointer for jingle generator (2 pathes)

extern struct a_Jingle3EnvDualData   _XMEM * p_jingle_data3;           // memory cell for storing data pointer for jingle generator (3 pathes)
extern struct a_Jingle3EnvDualCoeffs _YMEM * p_jingle_coeffs3;         // memory cell for storing coeff pointer for jingle generator (3 pathes)
*/
extern T_AudioLib_Signals _XMEM audio_lib_signals;

/*************************************************************************/
/*                         Y memory declaration                          */ 
/*************************************************************************/
/*
extern fraction _YMEM  div_6;                                          // global constant for (+1/6) 
extern fraction _YMEM  min_pos;                                        // global constant for (+0.000000119209)
extern fraction _YMEM  min_neg;                                        // global constant for (-0.000000119209)

extern fraction _YMEM  ar_sin_c_a9[5];                                 // global constant for sin(x) approximation (A9)
extern fraction _YMEM  ar_sin_c_a11[6];                                // global constant for sin(x) approximation (A11)
extern fraction _YMEM  ar_y_ld_coeffs[3];                              // global constant for log2(x) approximation

extern fraction _YMEM  cd_demph_fltr_coeffs[3];                        // global constant for deemphasis filter coeffs
*/

extern T_AudioLib_Params _YMEM audio_lib_params;

extern int      _YMEM zero;
extern int      _YMEM one;
extern fraction _YMEM one_f;
extern fraction _YMEM minus_one;
extern fraction _YMEM max_value;
extern fraction _YMEM min_value;
extern fraction _YMEM one_half;
extern fraction _YMEM one24th;  
extern fraction _YMEM one10th; 


//extern fraction _YMEM st_version_info;
//extern fraction _YMEM affe42; 

// -------------------- Function Prototypes ---------------------

/****************************************************************/
/* common init routine for global constants                     */
/****************************************************************/
void audio_lib_coeffs_init(void);


#endif
