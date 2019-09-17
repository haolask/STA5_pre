/***********************************************************************************/
/*!
*  \file      sta_cmd_parser.h
*
*  \brief     <i><b> STAudioLib Cmd Parser </b></i>
*
*  \details   STAudioLib Command Parser:
*
*
*  \author    Quarre Christophe
*
*  \author    (original version) Quarre Christophe
*
*  \version
*
*  \date      2016/04/18
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

#ifndef STA_CMD_PARSER_VERSION
#define STA_CMD_PARSER_VERSION		5


//Compatible with SAT up to v1.0

#include "staudio.h"
#include "sta_preset_loader.h"


//-----------
// STA Cmd
//-----------

//cmdCode flags (e.g. cmdID | STA_GET)
#define STA_SET		0   /* default */
#define STA_GET		0xF000


//tmp defines
//#define S	/* Set */
//#define G	/* Get */
//#define SG	/* Set/Get */
#define N	/* Not implemented (reserved) */

//Warning: some uart cmd parameters slightly differs from the corresponding STA api.
//lint -save -esym(960, 9.3) //misra2004 9.3 allow specific initialisation of block of cmd enums
typedef enum {
/*------ STA cmds from parser v4 (compatible with SAT 1.0) ------------*/
	STA_Init_ID = 3,
	STA_Reset_ID,
	STA_Exit_ID,
	STA_Play_ID,
	STA_Stop_ID,
	STA_DMAStart_ID,
	STA_DMAStop_ID,
	STA_DMAReset_ID,
N	STA_DMASetFSyncCK1_ID,
N	STA_DMASetFSyncCK2_ID,
/////////////////////////////////////////////////////////////////////////////////////////////
// Block of cmds used by Ark's SAT 1.0 (=> don't change this block of cmds for compatibility)
	STA_SetMode_ID = 13,			//(STAModule module, u32 ch, u32 mode)
	STA_SetFilterType_ID,			//(STAModule module, u32 ch, u32 band, STA_FilterType type)
	STA_SetFilter_ID,				//(STAModule module, u32 ch, u32 band, s32 Gain, s32 Qual, s32 Freq)
N	STA_SetFilterv_ID,				//(STAModule module, u32 ch, u32 band, s16 GQF[3])
	STA_SetFilterParam_ID,			//(STAModule module, u32 ch, u32 band, STA_FilterParam param, s32 val)
	STA_SetFilterParams_ID,			//(STAModule module, u32 ch, STA_FilterParam param, const s16* values)
	STA_SetFilterHalvedCoefs_ID,	//(STAModule module, u32 ch, u32 band, const q23* biquadCoefs)
	STA_SetFilterHalvedCoefsAll_ID,	//(STAModule module, u32 ch, u32 bandmask, const q23* allCoefs)
	STA_MuxSet_ID,					//(STAModule module, const u8* sels)
	STA_MuxSetOutChannel_ID,		//(STAModule module, u32 outch, u32 inch)
	STA_GainSetGain_ID,				//(STAModule module, u32 ch, s32 Gain)
	STA_GainSetGains_ID,			//(STAModule module, u32 chMask, s32 Gain)
	STA_SmoothGainSetRamp_ID,		//(STAModule module, u32 chMask, u32 msecUp, u32 msecDown)
	STA_ToneSetTypes_ID,			//(STAModule module, STA_ToneFilterType bassType, STA_ToneFilterType trebType)
	STA_ToneSetAutoBassQLevels_ID,	//(STAModule module, s32 Gstart, s32 Gstop, s32 Qstart, s32 Qstop)
	STA_LoudSetTypes_ID,			//(STAModule module, STA_LoudFilterType bassType, STA_LoudFilterType trebType)
	STA_LoudSetAutoBassQLevels_ID,	//(STAModule module, s32 Gstart, s32 Gstop, s32 Qstart, s32 Qstop)
	STA_LoudSetAutoGLevels_ID,		//(STAModule module, STA_LoudFilter filter, s32 Vstart, s32 Vstop, s32 Gmin, s32 Gmax)
	STA_MixerSetAbsoluteInGain_ID,	//(STAModule module, u32 inch, s32 inGain)
	STA_MixerSetRelativeInGain_ID,	//(STAModule module, u32 outCh, u32 inRel, s32 inGain)
	STA_MixerSetChannelInGains_ID,	//(STAModule module, u32 ch, const s16* Gains)
	STA_MixerSetOutGain_ID,			//(STAModule module, u32 outch, s32 Gain)
	STA_CompanderSetParams_ID,		//(STAModule module, u32 paramMask, const s16 *params)
	STA_LimiterSetParams_ID,		//(STAModule module, u32 paramMask, const s16 *params)
	STA_ClipLimiterSetParams_ID,	//(STAModule module, u32 paramMask, const s32 *params)
	STA_DelaySetDelay_ID,			//(STAModule module, u32 ch, u32 delay)
	STA_DelaySetDelays_ID,			//(STAModule module, u32 chMask, u32 delay)
	STA_SinewaveGenSet_ID,			//(STAModule module, u32 freq, u32 msec, s32 gainL, s32 gainR)
	STA_SinewaveGenPlay_ID,			//(STAModule module, u32 freq, u32 msec, s32 ampL, s32 ampR)
	STA_ChimeGenBeep_ID,
	STA_ChimeGenStop_ID,
// end of cmds used by the Ark SAT 1.0
//////////////////////////////////////////////////////////////////////////////////////
/*------ NEW cmds from parser v5 (post SAT 1.0) ------------*/
	STA_LoadDSP_ID,					//(STA_Dsp core)
N	STA_LoadDSP_ext_ID,				//(STA_Dsp core, const char *Paddr, u32 Psize, const char *Xaddr, u32 Xsize, const char *Yaddr, u32 Ysize)
	STA_StartDSP_ID,				//(STA_Dsp core)
	STA_StopDSP_ID,					//(STA_Dsp core)
	STA_Enable_ID,					//(STA_EnableParam option)
	STA_Disable_ID,					//(STA_EnableParam option)
N	STA_DMAAddTransfer_ID,			//(u16 srcAddr, u16 dstAddr, u32 numCh)
N	STA_DMAAddTransfer2_ID,			//(u16 srcAddr, u16 dstAddr, u32 numCh, u32 type)
N	STA_DMASetTransfer_ID,			//(u16 srcAddr, u16 dstAddr, u32 numCh, u32 dmaPos, u32 type);
N	STA_AddModule_ID,				//(STA_Dsp dsp, STA_ModuleType type )
	STA_AddModule2_ID,				//(STA_Dsp dsp, STA_ModuleType type, u32 id, u32 tags, const char* name)
N	STA_AddUserModule_ID,			//(STA_Dsp dsp, STA_ModuleType type, const STA_UserModuleInfo *info )
N	STA_AddUserModule2_ID,			//(STA_Dsp dsp, STA_ModuleType type, const STA_UserModuleInfo *info, u32 id, u32 tags, const char* name)
	STA_DeleteModule_ID,			//(STAModule module)
N	STA_Connect_ID,					//(STAModule from, u32 chout, STAModule to, u32 chin)
N	STA_Disconnect_ID,				//(STAModule from, u32 chout, STAModule to, u32 chin )
N	STA_ReconnectSrc_ID,
N	STA_ReconnectDst_ID,
	STA_BuildFlow_ID,				//(void)
	STA_DelaySetLength_ID,			//(STAModule module, u32 ch, u32 length)
N	STA_CompanderSetParam_ID,		//(STAModule module, u32 ch, u32 param, u32 val)
N	STA_LimiterSetParam_ID,			//(STAModule module, u32 ch, u32 param, u32 val)
N	STA_ClipLimiterSetParam_ID,		//(STAModule module, u32 ch, u32 param, u32 val)
/*------ Getters ---------------------------------------------*/
	STA_GetError_ID					= STA_GET| 1,	//(STA_ErrorCode)
	STA_GetInfo_ID 					= STA_GET| 2,	//(STA_Info info, u32 ret)
	STA_GetModuleInfo_ID			= STA_GET| 3,	//(STAModule module, STA_ModuleInfo info, u32 ret)
	STA_GetFreeCycles_ID			= STA_GET| 4,	//(u32 freeCycles[3])
	STA_GetMaxDspCycleCost_ID		= STA_GET| 5,	//(STA_Dsp core, u32 cycles)
	STA_GetDspMemCost_ID			= STA_GET| 6,	//(STA_Dsp core, u32 *xmemWords, u32* ymemWords)
	STA_GetMode_ID					= STA_GET|STA_SetMode_ID,				//(STAModule module, u32 ch, u32 mode)
	STA_GetFilter_ID 				= STA_GET|STA_SetFilter_ID,				//(STAModule module, u32 ch, u32 band, s32 G, s32 Q, s32 F)
	STA_GetFilterType_ID 			= STA_GET|STA_SetFilterType_ID, 		//(STAModule module, u32 ch, u32 band, STA_FilterType type)
	STA_GetFilterParam_ID 			= STA_GET|STA_SetFilterParam_ID,		//(STAModule module, u32 ch, u32 band, STA_FilterParam param, s32 value)
	STA_GetFilterParams_ID 			= STA_GET|STA_SetFilterParams_ID,		//(STAModule module, u32 ch, STA_FilterParam param, s16* values)
	STA_GetFilterHalvedCoefs_ID 	= STA_GET|STA_SetFilterHalvedCoefs_ID,	//(STAModule module, u32 ch, u32 band, q23* coefs )
	STA_GetFilterHalvedCoefsAll_ID	= STA_GET|STA_SetFilterHalvedCoefsAll_ID,//(STAModule module, u32 ch, u32 bandmask, q23* allCoefs )
	STA_MuxGet_ID 					= STA_GET|STA_MuxSet_ID,				//(STAModule module, u8* sels)
	STA_MixerGetRelativeInGain_ID 	= STA_GET|STA_MixerSetRelativeInGain_ID,//(STAModule module, u32 outCh, u32 inRel, s32 inGain)
	STA_MixerGetAbsoluteInGain_ID 	= STA_GET|STA_MixerSetAbsoluteInGain_ID,//(STAModule module, u32 inAbs, s32 inGain)
	STA_MixerGetOutGain_ID 			= STA_GET|STA_MixerSetOutGain_ID,		//(STAModule module, u32 outch, s32 outGain)
	STA_GainGetGain_ID 				= STA_GET|STA_GainSetGain_ID,			//(STAModule module, u32 ch, s32 Gain)
	STA_DelayGetDelay_ID 			= STA_GET|STA_DelaySetDelay_ID,			//(STAModule module, u32 ch, u32 delay)
	STA_DelayGetLength_ID 			= STA_GET|STA_DelaySetLength_ID,		//(STAModule module, u32 ch, u32 length)
	STA_LimiterGetParam_ID			= STA_GET|STA_LimiterSetParam_ID,		//(STAModule module, u32 ch, u32 param, u32 val)
	STA_ClipLimiterGetParam_ID		= STA_GET|STA_ClipLimiterSetParam_ID,	//(STAModule module, u32 ch, u32 param, u32 val)



//	STA_PCMSetMaxDataSize_ID,
//	STA_PCMLoadData_16bit_ID,
//	STA_PCMPlay_ID,
//	STA_PCMStop_ID,
//	STA_ChimeGenSetMaxNumRamps_ID,
//	STA_ChimeGenPlay_ID,
//	STA_ChimeGenSetParam_ID,
//	STA_ChimeGenGetParam_ID

//	STA_SetNumChannels_ID,
//	STA_SetNumOfUpdateSlots_ID,
//	STA_SetUpdateSlot_ID,

//	STA_MixerBindTo_ID,
//	STA_MixerSetBalance_ID,
//	STA_MixerSetBalanceLimit_ID,
//	STA_MixerSetBalanceCenter_ID,
} STA_CmdID;
//lint -restore

//#undef S
//#undef G
//#undef SG
#undef N


/* GetSTAModuleAndChannel handler */
typedef void (*STA_MOD_N_CH_FCT)(u32 GUIModuleID, u32 GUIChannel, STAModule *pModuleID, u8 *pChannel);

void STA_ParserSetPreset(const STAPreset *preset);
void STA_ParserSetModuleTable(const STAModuleEntry* moduleTable, u32 size);
void STA_ParserSetModuleLUT(const STAModuleLutEntry* moduleLUT, u32 size);
void STA_ParserSetModuleTranslationFunc(STA_MOD_N_CH_FCT func);

void STA_ParserCmd(STA_CmdID cmdID, void* data, u32* datasize);


#endif //STA_CMD_PARSER_VERSION

