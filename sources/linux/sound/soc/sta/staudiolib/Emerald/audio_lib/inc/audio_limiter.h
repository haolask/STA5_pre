/***********************************************************************************/
/*!
*  \file      audio_limiter.h
*
*  \brief     <i><b> Header file for Dynamic Range Limiter Block </b></i>
*
*  \details   Header file for Dynamic Range Limiter Block
*
*  \author    APG-MID Application Team
*
*  \author    (original version) K.Grieshober
*
*  \version   1.1
*
*  \date      2007.07.19
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
* 2015.01.20: moved sub-data field "T_LimiterData  generic_data"
*             from below to top (C.Quarre)  
*
***********************************************************************************/

#ifndef _LIMITER_H_
#define _LIMITER_H_

// -------------------- Defines ---------------------------------

#define LOG_SCALE                5
#define ALimiterUpShift          4

#define _A_LIM_DELAY            80 
#define _A_LIM_MONO_BUF_LEN     80
#define _A_LIM_STEREO_BUF_LEN  160
#define _A_LIM_QUADRO_BUF_LEN  320
#define _A_LIM_6CH_BUF_LEN     480

// -------------------- Type Defs -------------------------------

typedef struct                  /* coefficients for dynamic range limiter */
{
   fraction     on_off;                 /* enable (!=0x000000) / disable (=0x000000) limiter module             */
   fraction     peak_at;                /* attack time for peak value calculation                               */
   fraction     peak_rt;                /* release time for peak value calculation                              */    
   fraction     threshold;              /* threshold setting for dynamic range limiter [20*log10(2)*32 dB]      */
   fraction     hysteresis;             /* hysteresis for t_current -> t_attack [20*log10(2)*32 dB]             */
   fraction     t_attack;               /* time constant for attenuation increase                               */
   fraction     t_release;              /* time constant for attenuation decrease                               */
   fraction     attenuation;            /* overall attenuation of dynamic range limiter [20*log10(2)*32 dB]     */ 
}T_LimiterCoeffs;

typedef struct                    /* common data structure for dynamic range limiter */
{
   fraction     peak_sig;               /* peak absolute input signal                                           */
   fraction     g_1;                    /* resulting multiply factor to be applied to input signal              */    
   fraction     t_current;              /* time constant for smoothing of multiply factor                       */    
   fraction     input_level;            /* equivalent level of peak_sig [20*log10(2)*32 dB]                     */
   fraction     control_value;          /* linear equivalent of control_level                                   */    
   fraction     control_level;          /* resulting control level to be applied to input [20*log10(2)*32 dB]   */
   fraction     min_control_level;      /* min. resulting control level (since last reset) [20*log10(2)*32 dB]  */
}T_LimiterData;

typedef struct                /* data structure for mono channel limiter */
{
   T_LimiterData  generic_data;         /* data structure for common variables                                  */   
   fraction _XMEM * din;                /* pointer to input data signal (mono channel)                          */
   fraction _XMEM * dout;               /* pointer to output data signal (mono channel)                         */    
   fraction _YMEM * delay_buffer;       /* start address of delay buffer                                        */
   fraction _YMEM * delay_ptr;          /* pointer to current address in delay buffer                           */
   fraction _YMEM * delay_buf_end;      /* end address of delay buffer                                          */
// T_LimiterData  generic_data;         /* data structure for common variables                                  */   
}T_LimiterMonoData;

typedef struct              /* data structure for stereo channel limiter */
{
   T_LimiterData  generic_data;         /* data structure for common variables                                  */   
   fraction _XMEM * din[2];             /* pointers to input data signals (stereo channel)                      */
   fraction _XMEM * dout[2];            /* pointers to output data signals (stereo channel)                     */
   fraction _YMEM * delay_buffer;       /* start address of delay buffer                                        */
   fraction _YMEM * delay_ptr;          /* pointer to current address in delay buffer                           */
   fraction _YMEM * delay_buf_end;      /* end address of delay buffer                                          */
}T_LimiterStereoData;

typedef struct               /* data structure for 4 channel limiter */ 
{
   T_LimiterData  generic_data;         /* data structure for common variables                                  */   
   fraction _XMEM * din[4];             /* pointers to input data signals (four channel)                        */
   fraction _XMEM * dout[4];            /* pointers to output data signals (four channel)                       */    
   fraction _YMEM * delay_buffer;       /* start address of delay buffer                                        */
   fraction _YMEM * delay_ptr;          /* pointer to current address in delay buffer                           */
   fraction _YMEM * delay_buf_end;      /* end address of delay buffer                                          */
}T_LimiterQuadroData;

typedef struct                  /* data structure for 6 channel limiter */ 
{                       
   T_LimiterData  generic_data;         /* data structure for common variables                                  */   
   fraction _XMEM * din[6];             /* pointers to input data signals (six channel)                         */                    
   fraction _XMEM * dout[6];            /* pointers to output data signals (six channel)                        */          
   fraction _YMEM * delay_buffer;       /* start address of delay buffer                                        */       
   fraction _YMEM * delay_ptr;          /* pointer to current address in delay buffer                           */       
   fraction _YMEM * delay_buf_end;      /* end address of delay buffer                                          */       
}T_Limiter6ChData;

// -------------------- Memory Defs -----------------------------


// -------------------- Function Prototypes ---------------------

/****************************************************************/
/* common init routine for dynamic range limiter                */
/****************************************************************/
void audio_limiter_init_rom(T_LimiterCoeffs _YMEM * coeffs, T_LimiterData _XMEM * data);

/****************************************************************/
/* complete limiter update within a single frame                */
/* featuring detection of minimum control level                 */
/* and level calculation with 2nd order interpolation           */
/****************************************************************/
void audio_limiter_update(T_LimiterCoeffs _YMEM * coeffs, T_LimiterData _XMEM * data);

/****************************************************************/
/* level measurement, data path delay, and re-scaling for a     */
/* single audio channel (WITHOUT up-shifting)                   */
/****************************************************************/
void audio_limiter_la_mono(T_LimiterCoeffs _YMEM * coeffs, T_LimiterMonoData _XMEM * data);

/****************************************************************/
/* level measurement, data path delay, and re-scaling for a     */
/* stereo audio signal (WITHOUT up-shifting)                    */
/****************************************************************/
void audio_limiter_la_stereo(T_LimiterCoeffs _YMEM * peak_coeffs, T_LimiterStereoData _XMEM * data);

/****************************************************************/
/* level measurement, data path delay, and re-scaling for a     */
/* four-channel audio signal (WITHOUT up-shifting)              */
/****************************************************************/
void audio_limiter_la_quadro(T_LimiterCoeffs _YMEM * peak_coeffs, T_LimiterQuadroData _XMEM * data);

/****************************************************************/
/* level measurement, data path delay, and re-scaling for a     */
/* six-channel audio signal (WITHOUT up-shifting)               */
/****************************************************************/
void audio_limiter_la_6ch(T_LimiterCoeffs _YMEM * peak_coeffs, T_Limiter6ChData _XMEM * data);

#endif

