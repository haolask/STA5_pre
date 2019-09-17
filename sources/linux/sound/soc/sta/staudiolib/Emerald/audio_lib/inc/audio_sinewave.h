/***********************************************************************************/
/*!
*  \file      audio_sinewave.h
*
*  \brief     <i><b> Header file for Sinewave Generator Block </b></i>
*
*  \details   Header file for Sinewave Generator Block
*
*  \author    APG-MID Application Team
*
*  \author    (original version) K.Grieshober
*
*  \version   1.0
*
*  \date      2007.08.14
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

#ifndef _SINWAVE_H_
#define _SINWAVE_H_

// -------------------- Type Defs -------------------------------

// >> SINEWAVE GENERATOR << //

struct a_SinewaveData                   /* data structure for sinewave generator (two output channels)*/
{
   fraction _XMEM * dout[2];            /* pointer to output data signal (two output channels)                  */      
   fraction         phase;              /* variable for storing current phase for sinewave approximation        */
};

struct a_SinewaveCoeffs                 /* coefficient structure for sinewave generator (two output channels) */
{
   int              on_off;             /* enable (!=0x000000) / disable (=0x000000) sinewave generator         */
   fraction         out_gain[2];        /* scaling factor for output signals (channel #1 / channel #2)          */
   fraction         phase_inc;          /* phase increment (=frequency setting) for sinewave generation         */        
};

// >> SINEWAVE SWEEP GENERATOR << //

enum SweepState                         /* enumeration for "state" register */
{
   SweepIdle         = 0,               /* define SweepIdle        => '0'                                       */
   SweepRunning      = 1                /* define SweepRunning     => '1'                                       */
};

enum SweepRequestBits                   /* enumeration for "request" register */
{
   SweepStopBit      = 0,               /* define SweepStopBit     => bit #0                                    */ 
   SweepStartBit     = 1,               /* define SweepStartBit    => bit ä1                                    */
   SweepContinueBit  = 2                /* define SweepContinueBit => bit #2                                    */
};

struct a_SinewaveSweepData              /* data structure for sinewave sweep generator (two output channels)*/
{
   fraction _XMEM * dout[2];            /* pointer to output data signal (two output channels)                  */      
   fraction         phase;              /* variable for storing current phase for sinewave approximation        */
   int	            state;              /* state register for controlling sinewave sweep                        */
   int              time_cnt;           /* counter for implementing programmable time intervalls (sweep mode)   */
   int              step_trig;          /* trigger output for indicating a jump to a new frequency step         */
};

struct a_SinewaveSweepCoeffs            /* coefficient structure for sinewave sweep generator (two output channels */
{
   fraction         on_off;             /* enable (!=0x000000) / disable (=0x000000) sinewave sweep generator    */
   fraction         sweep;              /* enable sweep (!=0x000000) / sine (=0x000000) mode                     */     
   fraction         out_gain[2];        /* scaling factor for output signals (channel #1 / channel #2)           */
   fraction         phase_inc;          /* phase increment (=frequency setting) for sinewave generation          */         
   fraction         phase_inc_start;    /* 2*f_start/fs  (start frequency for sinewave sweep)                    */
   fraction         phase_inc_stop;     /* 2*f_stop/fs (stop frequency for sinewave sweep)                       */
   fraction         phase_inc_factor;   /* logarithmic increase for phase increment (frequency sweep)            */
   int              phase_inc_shift;    /* left-shift value corresponding to "phase_inc_factor"                  */
   int              n_time;             /* time intervall per frequency step                                     */
   int              request;            /* request register for controlling sinewave sweep from external device  */        
};

// -------------------- Memory Defs -----------------------------

// -------------------- Function Prototypes ---------------------

/****************************************************************/
/* initialization routine for sinewave generator                */
/****************************************************************/
void audio_sinewave_init_rom(struct a_SinewaveData _XMEM * data, struct a_SinewaveCoeffs _YMEM * coeffs);

/****************************************************************/
/* routine for implementing sinewave generator                  */
/****************************************************************/
void audio_sinewave(struct a_SinewaveData _XMEM * data, struct a_SinewaveCoeffs _YMEM * coeffs);

/****************************************************************/
/* initialization routine for sinewave sweep generator          */
/****************************************************************/
void audio_sinewave_sweep_init_rom(struct a_SinewaveSweepData _XMEM * data, struct a_SinewaveSweepCoeffs _YMEM * coeffs);

/****************************************************************/
/* routine for implementing sinewave sweep generator            */
/****************************************************************/
void audio_sinewave_sweep(struct a_SinewaveSweepData _XMEM * data, struct a_SinewaveSweepCoeffs _YMEM * coeffs);


#endif
