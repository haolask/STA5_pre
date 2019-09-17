/***********************************************************************************/
/*!
*
*  \file      internal.c
*
*  \brief     <i><b> STAudioLib internal functions </b></i>
*
*  \details   Internal functions for conversions....
*
*
*  \author    Quarre Christophe
*
*  \author    (original version) Quarre Christophe
*
*  \version
*
*  \date      2013/04/22
*
*  \bug       see Readme.txt
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
*
*/
/***********************************************************************************/

#include "internal.h"


void _STA_memset32(volatile void* ptr, int value, unsigned int num)
{
	vu32 *dst = ptr;
	u32 i;

	sta_assert((num % 4) == 0);
	num >>= 2;	//now num Words
	//we should also check that 'ptr' is word aligned...

	if (!dst) {goto _ret;}

	for (i = 0; i < num; i++) {
		*dst++ = (u32)value;}

_ret: return;
}

void _STA_memcpy32(volatile void* dst, const void* src, unsigned int num)
{
	const vu32 *_src = src;
	      vu32 *_dst = dst;
	u32 i;

	if (!_dst || !_src) {goto _ret;}

	sta_assert((num % 4) == 0);
	num >>= 2;	//now num Words
	//we should also check that 'dst' is word aligned...

	for (i = 0; i < num; i++) {
		*_dst++ = *_src++;}

_ret: return;
}

const char* const g_errorMsg[STA_NUM_ERROR_CODE] = {
	"STA_NO_ERROR",
	"STA_NOT_INITIALIZED",
	"STA_INVALID_MODULE",
	"STA_INVALID_MODULE_TYPE",
	"STA_INVALID_CHANNEL",
	"STA_INVALID_PARAM",
	"STA_INVALID_VALUE",
	"STA_INVALID_TYPE",
	"STA_INVALID_SIZE",
	"STA_OUT_OF_MEMORY",
	"STA_NOT_ON_SAME_DSP",
	"STA_DSP0_OUT_OF_XMEM",
	"STA_DSP1_OUT_OF_XMEM",
	"STA_DSP2_OUT_OF_XMEM",
	"STA_DSP0_OUT_OF_YMEM",
	"STA_DSP1_OUT_OF_YMEM",
	"STA_DSP2_OUT_OF_YMEM",
	"STA_DSP0_XMEM_NOT_LOADED",
	"STA_DSP1_XMEM_NOT_LOADED",
	"STA_DSP2_XMEM_NOT_LOADED",
	"STA_DSP0_YMEM_NOT_LOADED",
	"STA_DSP1_YMEM_NOT_LOADED",
	"STA_DSP2_YMEM_NOT_LOADED",
	"STA_DSP0_LIB_NOT_MATCHING",
	"STA_DSP1_LIB_NOT_MATCHING",
	"STA_DSP2_LIB_NOT_MATCHING",
	"STA_DSP0_XMEM_SIZE_ERROR",
	"STA_DSP1_XMEM_SIZE_ERROR",
	"STA_DSP2_XMEM_SIZE_ERROR",
	"STA_DSP0_YMEM_SIZE_ERROR",
	"STA_DSP1_YMEM_SIZE_ERROR",
	"STA_DSP2_YMEM_SIZE_ERROR",
	"STA_DSP0_NOT_STARTED",
	"STA_DSP1_NOT_STARTED",
	"STA_DSP2_NOT_STARTED",
	"STA_DSP0_PLAY_ERROR",
	"STA_DSP1_PLAY_ERROR",
	"STA_DSP2_PLAY_ERROR",
	"STA_DSP_STILL_RUNNING",
	"STA_DMA_OUT_OF_SLOT",
	"STA_DMA_STILL_RUNNING",
	"STA_INVALID_DMA_SRC_ADDR",
	"STA_INVALID_DMA_DST_ADDR",
	"STA_INVALID_DMA_TYPE",
	"STA_INVALID_FLOW",
	"STA_STILL_PLAYING",
	"STA_INVALID_AFTER_BUILT",
	"STA_INVALID_BEFORE_BUILT",
	"STA_INVALID_BEFORE_DSP_INIT_DONE",
	"STA_FAILED_TO_GENERATE_ID",
	"STA_ID_ALREADY_EXIST",
	"STA_WARN_CONNECTION_DO_EXIST",
	"STA_WARN_CONNECTION_NOT_EXIST",
};

//note: the enum names are shorten in order to reduce the rodata size
const char* const g_STA_ModuleTypeStr[STA_MOD_LAST] = {
	"XIN",
	"XOUT",
	"MUX_2OUT",
	"MUX_4OUT",
	"MUX_6OUT",
	"MUX_8OUT",
	"MUX_10OUT",
	"CD_DEEMPH",
	"GAIN_STATIC_NCH",
	"GAIN_SMOOTH_NCH",
	"GAIN_LINEAR_NCH",
	"LOUD_DP",
	"LOUD_STEREO_DP",
	"TONE_DP",
	"TONE_STEREO_DP",
	"EQ_1BAND_DP",
	"EQ_2BANDS_DP",
	"EQ_3BANDS_DP",
	"EQ_4BANDS_DP",
	"EQ_5BANDS_DP",
	"EQ_6BANDS_DP",
	"EQ_7BANDS_DP",
	"EQ_8BANDS_DP",
	"EQ_9BANDS_DP",
	"EQ_10BANDS_DP",
	"EQ_11BANDS_DP",
	"EQ_12BANDS_DP",
	"EQ_13BANDS_DP",
	"EQ_14BANDS_DP",
	"EQ_15BANDS_DP",
	"EQ_16BANDS_DP",
	"EQ_1BAND_SP",
	"EQ_2BANDS_SP",
	"EQ_3BANDS_SP",
	"EQ_4BANDS_SP",
	"EQ_5BANDS_SP",
	"EQ_6BANDS_SP",
	"EQ_7BANDS_SP",
	"EQ_8BANDS_SP",
	"EQ_9BANDS_SP",
	"EQ_10BANDS_SP",
	"EQ_11BANDS_SP",
	"EQ_12BANDS_SP",
	"EQ_13BANDS_SP",
	"EQ_14BANDS_SP",
	"EQ_15BANDS_SP",
	"EQ_16BANDS_SP",
	"MIXE_2INS_NCH",
	"MIXE_3INS_NCH",
	"MIXE_4INS_NCH",
	"MIXE_5INS_NCH",
	"MIXE_6INS_NCH",
	"MIXE_7INS_NCH",
	"MIXE_8INS_NCH",
	"MIXE_9INS_NCH",
	"MIXE_BALANCE",
	"MIXE_FADER",
	"COMP_1CH",
	"COMP_2CH",
	"COMP_4CH",
	"COMP_6CH",
	"LMTR_1CH",
	"LMTR_2CH",
	"LMTR_4CH",
	"LMTR_6CH",
	"CLIPLMTR_NCH",
	"DLAY_Y_2CH",
	"DLAY_Y_4CH",
	"DLAY_Y_6CH",
	"DLAY_X_2CH",
	"DLAY_X_4CH",
	"DLAY_X_6CH",
	"SINE_2CH",
	"PCMCHIME_12BIT_Y_2CH",
	"PCMCHIME_12BIT_X_2CH",
	"CHIMEGEN",
	"BITSHIFTER_NCH",
	"PEAKDETECT_NCH",
	"SPECMTR_NBANDS_1BIQ",
	"DCO_1CH",
	"DCO_4CH_MONOZCF"};

const char* const g_STA_FilterParamStr[4] = {
	"GAIN",
	"QUAL",
	"FREQ",
	"TYPE"};

//note: the enum names are shorten in order to reduce the rodata size
const char* const g_STA_FilterTypeStr[STA_BIQUAD_NUM] = {
	"BYPASS",
	"LOWP_BUTTW_1",
	"LOWP_BUTTW_2",
	"HIGP_BUTTW_1",
	"HIGP_BUTTW_2",
	"BASS_SHELV_1",
	"BASS_SHELV_2",
	"TREB_SHELV_1",
	"TREB_SHELV_2",
	"BAND_BOOST_2",
	"AP2_CB",
	"LP2_CB",
	"HP2_CB",
	"BP2_CB",
	"NOTCH_CB",
	"PEAKING_CB",
	"LSHELF_CB",
	"HSHELF_CB"};


#if 0
const char* const g_STA_LoudModeStr[3] = {
	"LOUD_OFF",
	"LOUD_AUTO",
	"LOUD_MANUAL"};

const char* const g_STA_ToneModeStr[3] = {
	"TONE_OFF",
	"TONE_AUTO_BASS_Q",
	"TONE_MANUAL"};

const char* const g_STA_ModeOnOffStr[2] = {
	"OFF",
	"ON"};

const char* const g_STA_EqModeStr[2] = {
	"EQ_OFF",
	"EQ_ON"};

const char* const g_STA_LimiterModeStr[2] = {
	"LMTR_OFF",
	"LMTR_ON"};
#endif

const char* const g_STA_ChimeGenParamStr[4] = {
	"REPEAT_COUNT",
	"PLAYING",
	"POST_REPEAT_RAMPS",
	"CHIME_MASTER_CHIME"};

const char* const g_STA_EnableParamStr[2] = {
	"FREE_CYCLE_COUNTER",
	"CALL_TRACE"};

