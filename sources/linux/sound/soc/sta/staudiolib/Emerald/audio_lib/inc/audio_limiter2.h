/***********************************************************************************/
/*!
*  \file      audio_limiter2.h
*
*  \brief     <i><b> Header file for new Dynamic Range Limiter Block </b></i>
*
*  \details   Header file for new Dynamic Range Limiter Block
*
*  \author    APG-MID Application Team
*
*  \author    (original version) O. Douvenot
*
*  \version   1.0
*
*  \date      2015.12.14
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
*
*---------------------------------------------------------------------------------
* ChangeLog:
*
***********************************************************************************/

#ifndef _LIMITER2_H_
#define _LIMITER2_H_

// -------------------- Defines ---------------------------------


// -------------------- Type Defs -------------------------------

typedef struct                  /* coefficients for dynamic range limiter2 */
{
   fraction     on_off;                 /* enable (!=0x000000) / disable (=0x000000) limiter2 module             */
   fraction     peak_at;                /* attack time for peak value calculation                               */
   fraction     peak_rt;                /* release time for peak value calculation                              */    
   fraction     threshold;              /* linear threshold setting for dynamic range limiter2 */
   fraction     inv_hysteresis;         /* linear inverse hysteresis to swap from t_current -> t_attack */
   fraction     t_attack;               /* time constant for attenuation increase                               */
   fraction     t_release;              /* time constant for attenuation decrease                               */
   fraction     attenuation;            /* additional linear attenuation of dynamic range limiter2 */ 
}T_Limiter2Coeffs;

typedef struct                    /* common data structure for dynamic range limiter2 */
{
   fraction     peak_sig;               /* peak absolute input signal                                           */
   fraction     g_1;                    /* linear attenuation applied to input signal     */    
   fraction     t_current;              /* time constant for smoothing of attenuation                    */    
   fraction     control_value;          /* linear attenuation target of dynamic range limiter2 */    
   fraction     control_level;          /* previous linear gain to limit reach threshold */
}T_Limiter2Data;

typedef struct                /* data structure for mono channel limiter2 */
{
   T_Limiter2Data  generic_data;         /* data structure for common variables                                  */   
   fraction _XMEM * din;                /* pointer to input data signal (mono channel)                          */
   fraction _XMEM * dout;               /* pointer to output data signal (mono channel)                         */    
   fraction _YMEM * delay_buffer;       /* start address of delay buffer                                        */
   fraction _YMEM * delay_ptr;          /* pointer to current address in delay buffer                           */
   fraction _YMEM * delay_buf_end;      /* end address of delay buffer                                          */
}T_Limiter2MonoData;

typedef struct              /* data structure for stereo channel limiter2 */
{
   T_Limiter2Data  generic_data;         /* data structure for common variables                                  */   
   fraction _XMEM * din[2];             /* pointers to input data signals (stereo channel)                      */
   fraction _XMEM * dout[2];            /* pointers to output data signals (stereo channel)                     */
   fraction _YMEM * delay_buffer;       /* start address of delay buffer                                        */
   fraction _YMEM * delay_ptr;          /* pointer to current address in delay buffer                           */
   fraction _YMEM * delay_buf_end;      /* end address of delay buffer                                          */
}T_Limiter2StereoData;

typedef struct               /* data structure for 4 channel limiter2 */ 
{
   T_Limiter2Data  generic_data;         /* data structure for common variables                                  */   
   fraction _XMEM * din[4];             /* pointers to input data signals (four channel)                        */
   fraction _XMEM * dout[4];            /* pointers to output data signals (four channel)                       */    
   fraction _YMEM * delay_buffer;       /* start address of delay buffer                                        */
   fraction _YMEM * delay_ptr;          /* pointer to current address in delay buffer                           */
   fraction _YMEM * delay_buf_end;      /* end address of delay buffer                                          */
}T_Limiter2QuadroData;

typedef struct                  /* data structure for 6 channel limiter2 */ 
{                       
   T_Limiter2Data  generic_data;         /* data structure for common variables                                  */   
   fraction _XMEM * din[6];             /* pointers to input data signals (six channel)                         */                    
   fraction _XMEM * dout[6];            /* pointers to output data signals (six channel)                        */          
   fraction _YMEM * delay_buffer;       /* start address of delay buffer                                        */       
   fraction _YMEM * delay_ptr;          /* pointer to current address in delay buffer                           */       
   fraction _YMEM * delay_buf_end;      /* end address of delay buffer                                          */       
}T_Limiter2SixChData;

// -------------------- Memory Defs -----------------------------


// -------------------- Function Prototypes ---------------------

/****************************************************************/
/* common init routine for dynamic range limiter2                */
/****************************************************************/
void audio_limiter2_init_rom(T_Limiter2Data _XMEM * data, T_Limiter2Coeffs _YMEM * coeffs);

/****************************************************************/
/* complete limiter2 update within a single frame                */
/****************************************************************/
void audio_limiter2_update(T_Limiter2Data _XMEM * data, T_Limiter2Coeffs _YMEM * coeffs);

/****************************************************************/
/* level measurement, data path delay, and re-scaling for a     */
/* single audio channel (WITHOUT up-shifting)                   */
/****************************************************************/
void audio_limiter2_mono(T_Limiter2MonoData _XMEM * data, T_Limiter2Coeffs _YMEM * coeffs);

/****************************************************************/
/* level measurement, data path delay, and re-scaling for a     */
/* stereo audio signal (WITHOUT up-shifting)                    */
/****************************************************************/
void audio_limiter2_stereo(T_Limiter2StereoData _XMEM * data, T_Limiter2Coeffs _YMEM * peak_coeffs);

/****************************************************************/
/* level measurement, data path delay, and re-scaling for a     */
/* four-channel audio signal (WITHOUT up-shifting)              */
/****************************************************************/
void audio_limiter2_quadro(T_Limiter2QuadroData _XMEM * data, T_Limiter2Coeffs _YMEM * peak_coeffs);

/****************************************************************/
/* level measurement, data path delay, and re-scaling for a     */
/* six-channel audio signal (WITHOUT up-shifting)               */
/****************************************************************/
void audio_limiter2_6ch(T_Limiter2SixChData _XMEM * data, T_Limiter2Coeffs _YMEM * peak_coeffs);

#endif

