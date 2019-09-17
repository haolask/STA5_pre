/***********************************************************************************/
/*!
*  \file      audio_clip_limiter.h
*
*  \brief     <i><b> Header file for the Clip Limiter </b></i>
*
*  \details   Header file for the Clip Limiter
*
*  \author    APG-MID Application Team
*
*  \author    K.Yin, O.Douvenot
*
*  \version   2.0
*
*  \date      2016.02.17
*
*  \bug       Unknown
*
*  \warning  
* 
*  This file is part of STAudioLib and is dual licensed,
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
* STAudioLib is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* STAudioLib is distributed in the hope that it will be
* useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* Please refer to <http://www.gnu.org/licenses/>.
*/
/***********************************************************************************/

/* Limiter signal  I/O and clip flag in Input */
typedef struct {
	fraction _XMEM *din;	/* pointers to input data signals (1..N channels + clip signal) */
	fraction _XMEM *dout;	/* pointers to output data signals (1..N channels) */
} T_ClipLimiterData;

typedef struct {
	int			num_ch;	// number of channels (1..N)
	fraction	on_off;	// control signal bit 4 clip work or bypass
						// 0: OFF in bypass mode
						// 1: ON in attack-release mode
						// 2: ON in full-attack mode
	fraction	bit_mask;	// one bit selector
	fraction	polarity;	// clipping signal polarity
							// 0: low level clipping signal
							// bit_mask: high level clipping signal
	fraction	prev_clip;	// previous clip signal
	fraction	gain_goal;	// current gain goal
	fraction	tc_factor;	// current exponential time convergence factor

	fraction	gain_goal_att;	// gain goal for attack
	fraction	tc_factor_att;	// exponential time convergence factor for attack

	/*=========================================*/
	/* parameters for attack-release mode only */
	fraction	gain_goal_rel;	// gain goal for release
	fraction	tc_factor_rel;	// exponential time convergence factor for release
	/*=========================================*/

	/*======================================*/
	/* parameters for full-attack mode only */
	fraction	final_count_down;	// internal timer
	fraction	noise_rem_gain;		// noise removal gain
	/*======================================*/

	fraction	gain;	// internal gain
	fraction	inc;	// internal increment
} T_ClipLimiterNchParams;


void audio_clip_limiter_Nch_init(T_ClipLimiterData * data, T_ClipLimiterNchParams _YMEM * params);
void audio_clip_limiter_Nch(T_ClipLimiterData * data, T_ClipLimiterNchParams _YMEM * params);

