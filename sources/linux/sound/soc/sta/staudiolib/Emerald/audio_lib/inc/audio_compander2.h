/***********************************************************************************/
/*!
*  \file      audio_compander2.h
*
*  \brief     <i><b> Header file for Dynamic Range Compressor2 Block </b></i>
*
*  \details   Header file for Dynamic Range Compressor2 Block
*
*  \author    APG-MID Application Team
*
*  \author    O. Douvenot
*
*  \version   2.0
*
*  \date      2016.02.17
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

#ifndef _COMPANDER2_H_
#define _COMPANDER2_H_

// -------------------- Defines ---------------------------------

#define LOG_SCALE                 5
#define ACompUpShift              4

#define _A_COMP_DELAY            80 
#define _A_COMP_MONO_BUF_LEN     80
#define _A_COMP_STEREO_BUF_LEN  160
#define _A_COMP_QUADRO_BUF_LEN  320
#define _A_COMP_6CH_BUF_LEN     480

// -------------------- Type Defs -------------------------------

struct a_ComprBaseCoeffs                /* standard coefficients for dynamic range compressor/expander/noise gate   */ 
{
   fraction     on_off;                 /* enable (!=0x000000) / disable (=0x000000) compander2 module              */
   fraction     mean_max;               /* mean (=0x000000) / max (!=0x000000) - method for calculate "mean_sig"    */
   fraction     t_av;                   /* averaging coefficient for input signal amplitude                         */
   fraction     gain;                   /* additional gain of dynamic range compander2 [20*log10(2)*32 dB]          */  
};

struct a_ComprSectionCoeffs             /* coefficient set for a single section of a dynamic range compressor/expander/noise gate */  
{
   fraction     threshold;              /* threshold settings [20*log10(2)*32 dB]                                   */
   fraction     slope;                  /*  0 = no compression / expansion                                          */
                                        /* -1 = full compression / max. expansion (diff. to threshold subtracted)   */
   fraction     hysteresis;             /* minimum change of current_level for setting t_current to t_attack        */
   fraction     t_attack;               /* time constant for attenuation increase/decrease of compression/expansion */
   fraction     t_release;              /* time constant for attenuation decrease/increase of compression/expansion */
};

struct a_CompanderCoeffs                /* complete coefficient set for a combined dynamic range compander2         */
{
   struct a_ComprBaseCoeffs      base_coeffs;           /* standard coefficient set                                 */
   struct a_ComprSectionCoeffs   compression_coeffs;    /* coefficient set for section #1 (compressor)              */
   struct a_ComprSectionCoeffs   expansion_coeffs;      /* coefficient set for section #2 (expander)                */
};

struct a_CompanderGenericData           /* generic data set of any dynamic range compander2 system                  */
{
   fraction     mean_sig;               /* mean absolute input signal                                               */
   fraction     control_level;          /* resulting control level to be applied to input signal [20*log10(2)*32 dB]*/
   fraction     control_value;          /* linear equivalent of control_level                                       */
   fraction     t_memory;               /* time constant factor to memorize gain                                    */
   fraction     g_1;                    /* resulting multiply factor to be applied to input signal                  */
};

struct a_CompanderMonoData              /* data structure for mono channel compander2                               */ 
{
   struct a_CompanderGenericData   generic_data;  /* data structure for common variables                            */ 
   fraction _XMEM * din;                /* pointer to input data signal (mono channel)                              */
   fraction _XMEM * dout;               /* pointer to output data signal (mono channel)                             */    
   fraction _YMEM * delay_buffer;       /* start address of delay buffer                                            */
   fraction _YMEM * delay_ptr;          /* pointer to current address in delay buffer                               */
   fraction _YMEM * delay_buf_end;      /* end address of delay buffer                                              */    
};

struct a_CompanderStereoData            /* data structure for stereo channel compander2                             */
{
   struct a_CompanderGenericData   generic_data;  /* data structure for common variables                            */ 
   fraction _XMEM * din[2];             /* pointers to input data signals (stereo channel)                          */
   fraction _XMEM * dout[2];            /* pointers to output data signals (stereo channel)                         */    
   fraction _YMEM * delay_buffer;       /* start address of delay buffer                                            */
   fraction _YMEM * delay_ptr;          /* pointer to current address in delay buffer                               */
   fraction _YMEM * delay_buf_end;      /* end address of delay buffer                                              */  
};

struct a_CompanderQuadroData            /* data structure for stereo channel compander2                             */
{
   struct a_CompanderGenericData   generic_data;  /* data structure for common variables                            */ 
   fraction _XMEM * din[4];             /* pointers to input data signals (stereo channel)                          */
   fraction _XMEM * dout[4];            /* pointers to output data signals (stereo channel)                         */    
   fraction _YMEM * delay_buffer;       /* start address of delay buffer                                            */
   fraction _YMEM * delay_ptr;          /* pointer to current address in delay buffer                               */
   fraction _YMEM * delay_buf_end;      /* end address of delay buffer                                              */  
};

struct a_Compander6ChData               /* data structure for stereo channel compander2                             */
{
   struct a_CompanderGenericData   generic_data;  /* data structure for common variables                            */ 
   fraction _XMEM * din[6];             /* pointers to input data signals (stereo channel)                          */
   fraction _XMEM * dout[6];            /* pointers to output data signals (stereo channel)                         */    
   fraction _YMEM * delay_buffer;       /* start address of delay buffer                                            */
   fraction _YMEM * delay_ptr;          /* pointer to current address in delay buffer                               */
   fraction _YMEM * delay_buf_end;      /* end address of delay buffer                                              */  
};

// -------------------- Memory Defs -----------------------------


// -------------------- Function Prototypes ---------------------

/****************************************************************/
/* common init routine for dynamic range compander2             */
/****************************************************************/
void audio_compander_init_rom(struct a_CompanderGenericData _XMEM * data, struct a_CompanderCoeffs _YMEM * coeffs);

/****************************************************************/
/* complete compander2 update within a single frame             */
/****************************************************************/
void audio_compander_update(struct a_CompanderGenericData _XMEM * data, struct a_CompanderCoeffs _YMEM * coeffs);

/****************************************************************/
/* level measurement, data path delay, and re-scaling for a     */
/* single audio channel                                         */
/****************************************************************/
void audio_compander_la_mono(struct a_CompanderMonoData _XMEM * data, struct a_ComprBaseCoeffs _YMEM * base_coeffs);

/****************************************************************/
/* level measurement, data path delay, and re-scaling for a     */
/* stereo audio channel (mean/max method configurable)          */
/****************************************************************/
void audio_compander_la_stereo(struct a_CompanderStereoData _XMEM * data, struct a_ComprBaseCoeffs _YMEM * base_coeffs);

/****************************************************************/
/* level measurement, data path delay, and re-scaling for a     */
/* four audio channel (mean/max method configurable)            */
/****************************************************************/
void audio_compander_la_quadro(struct a_CompanderQuadroData _XMEM * data, struct a_ComprBaseCoeffs _YMEM * base_coeffs);

/****************************************************************/
/* level measurement, data path delay, and re-scaling for a     */
/* six audio channel (mean/max method configurable)             */
/****************************************************************/
void audio_compander_la_6ch(struct a_Compander6ChData _XMEM * data, struct a_ComprBaseCoeffs _YMEM * base_coeffs);

#endif

