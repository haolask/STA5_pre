/***********************************************************************************/
/*!
*  \file      sta_cmd_parser.c
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

//#include "common.h"
//#include "staudio.h"
#include "internal.h"

//Compatible with SATv0.7
#include "sta_cmd_parser.h"

//lint -esym(960, _PRINTF)   inhibits misra2004 19.4 (allow macro definition as alias)
#define _PRINTF				STA_PRINTF


//------------
//Cmd LUT
//------------

typedef void  (*STAFUNC) (STAModule, ...);	    //lint !e960 misra2004 16.1 "function has variable number of arguments" (required for STA cmd parser)
typedef u32   (*STAFUNCRET) (STAModule, ...);	//lint !e960 misra2004 16.1 "function has variable number of arguments" (required for STA cmd parser)
typedef void  (*FUNCVOID) (void);
//typedef void  (*FUNC1) (u32);
//typedef void  (*FUNC2) (u32, u32);


typedef struct {
//	const char* 	staName;
	STAFUNC			staFunc;
	u16 			cmdID;		// = STA_GET | STA_CmdID
//	u8 				numParams;
	u8				flags;
} STACmdLutEntry;

//cmd's internal flags.
//#define NO_FLAG		0
#define HAS_MOD		0x01
#define HAS_CH		0x02
#define HAS_CHMASK	0x04

//tmp
#define MOD			(HAS_MOD)
#define CH			(HAS_CH)
#define CHMASK		(HAS_CHMASK)

//lint -estring(960, CMD_ENTRY)  misra2004 19.12: allow use of #/## in this macro
//lint -esym(960, 11.1) //misra2004 allow casting of function pointer. Solving with an intermediate casting with (u32) would trigger an armcc compilation warning.

#ifdef TKERNEL
//(the .attribute assignment doesn't work in armcc...)
#define CMD_ENTRY(func, numParamz, flagz) {\
	/*	  #func, */ \
		 (STAFUNC)&(func), \
		 (u16)func##_ID, \
	/*	 (u8)(numParamz),*/ \
		 (u8)(flagz) }
#else
#define CMD_ENTRY(func, numParamz, flagz) {\
	/*	 .staName   = #func, */ \
		 .staFunc   = (STAFUNC)&(func), \
		 .cmdID     = (u16)func##_ID, \
	/*	 .numParams = (u8)(numParamz),*/ \
		 .flags     = (u8)(flagz) }
#endif


//(note: the order of the entries does not need to match the one of STA_CmdID)
static const STACmdLutEntry g_cmd_lut[] = {
/*------ STA cmds from parser v4 (compatible with SAT 1.0) ------------*/
	CMD_ENTRY(STA_Init, 1, 0),
	CMD_ENTRY(STA_Reset, 0, 0),
	CMD_ENTRY(STA_Exit, 0, 0),
	CMD_ENTRY(STA_Play, 0, 0),
	CMD_ENTRY(STA_Stop, 0, 0),
	CMD_ENTRY(STA_DMAStart, 0, 0),
	CMD_ENTRY(STA_DMAStop, 0, 0),
	CMD_ENTRY(STA_DMAReset, 0, 0),
	CMD_ENTRY(STA_DMASetFSyncCK1, 1, 0),
	CMD_ENTRY(STA_DMASetFSyncCK2, 1, 0),
/////////////////////////////////////////////////////////////////////////////////////////////
// Block of cmds used by Ark's SAT 1.0 (=> don't change this block of cmds for compatibility)
	CMD_ENTRY(STA_SetMode, 2, MOD | CH), 				//note: the uart cmd takes an additionnal 'ch' vs the STA API !
	CMD_ENTRY(STA_SetFilterType, 3, MOD | CH),			//note: the uart cmd takes an additionnal 'ch' vs the STA API !
	CMD_ENTRY(STA_SetFilter, 5, MOD | CH),				//note: the uart cmd takes an additionnal 'ch' vs the STA API !
	CMD_ENTRY(STA_SetFilterv, 3, MOD | CH),				//note: the uart cmd takes an additionnal 'ch' vs the STA API !
	CMD_ENTRY(STA_SetFilterParam, 4, MOD | CH),			//note: the uart cmd takes an additionnal 'ch' vs the STA API !
	CMD_ENTRY(STA_SetFilterParams, 4, MOD | CH),		//note: the uart cmd takes an additionnal 'ch' vs the STA API !
	CMD_ENTRY(STA_SetFilterHalvedCoefs, 3, MOD | CH),	//note: the uart cmd takes an additionnal 'ch' vs the STA API !
	CMD_ENTRY(STA_SetFilterHalvedCoefsAll, 3, MOD | CH),//note: the uart cmd takes an additionnal 'ch' vs the STA API !
	CMD_ENTRY(STA_MuxSet, 2, MOD),
	CMD_ENTRY(STA_MuxSetOutChannel, 3, MOD | CH),
	CMD_ENTRY(STA_GainSetGain, 3, MOD | CH),
	CMD_ENTRY(STA_GainSetGains, 3, MOD | CHMASK),
	CMD_ENTRY(STA_SmoothGainSetRamp, 4, MOD | CHMASK),
	CMD_ENTRY(STA_ToneSetTypes, 3, MOD),
	CMD_ENTRY(STA_ToneSetAutoBassQLevels, 5, MOD),
	CMD_ENTRY(STA_LoudSetTypes, 3, MOD),
	CMD_ENTRY(STA_LoudSetAutoBassQLevels, 5, MOD),
	CMD_ENTRY(STA_LoudSetAutoGLevels, 6, MOD),
	CMD_ENTRY(STA_MixerSetAbsoluteInGain, 3, MOD | CH),
	CMD_ENTRY(STA_MixerSetRelativeInGain, 4, MOD | CH),
	CMD_ENTRY(STA_MixerSetChannelInGains, 3, MOD),
	CMD_ENTRY(STA_MixerSetOutGain, 3, MOD | CH),
	CMD_ENTRY(STA_CompanderSetParams, 3, MOD),
	CMD_ENTRY(STA_LimiterSetParams, 3, MOD),
	CMD_ENTRY(STA_ClipLimiterSetParams, 3, MOD),
	CMD_ENTRY(STA_DelaySetDelay, 3, MOD | CH),
	CMD_ENTRY(STA_DelaySetDelays, 3, MOD | CHMASK),
	CMD_ENTRY(STA_SinewaveGenSet, 5, MOD),
	CMD_ENTRY(STA_SinewaveGenPlay, 5, MOD),
	CMD_ENTRY(STA_ChimeGenBeep, 2, MOD),
	CMD_ENTRY(STA_ChimeGenStop, 3, MOD),
// end of cmds used by the Ark SAT 1.0
//////////////////////////////////////////////////////////////////////////////////////
/*------ support for NEW cmds (post SAT 1.0) ------------*/
	CMD_ENTRY(STA_LoadDSP, 1, 0),
	CMD_ENTRY(STA_LoadDSP_ext, 7, 0),
	CMD_ENTRY(STA_StartDSP, 1, 0),
	CMD_ENTRY(STA_StopDSP, 1, 0),
	CMD_ENTRY(STA_Enable, 1, 0),
	CMD_ENTRY(STA_Disable, 1, 0),
	CMD_ENTRY(STA_DMAAddTransfer, 3, 0),
	CMD_ENTRY(STA_DMAAddTransfer2, 4, 0),
	CMD_ENTRY(STA_DMASetTransfer, 5, 0),
	CMD_ENTRY(STA_AddModule, 2, 0),
	CMD_ENTRY(STA_AddModule2, 5, 0),
	CMD_ENTRY(STA_AddUserModule, 3, 0),
	CMD_ENTRY(STA_AddUserModule2, 6, 0),
	CMD_ENTRY(STA_DeleteModule, 1, 0),
	CMD_ENTRY(STA_Connect, 4, 0),
	CMD_ENTRY(STA_Disconnect, 4, 0),
//	CMD_ENTRY(STA_ReconnectSrc, 4, 0),
//	CMD_ENTRY(STA_ReconnectDst, 4, 0),
	CMD_ENTRY(STA_BuildFlow, 0, 0),
	CMD_ENTRY(STA_DelaySetLength, 3, MOD | CH),
//	CMD_ENTRY(STA_CompanderSetParam, 4, MOD | CH),
//	CMD_ENTRY(STA_LimiterSetParam, 4, MOD | CH),
//	CMD_ENTRY(STA_ClipLimiterSetParam, 4, MOD | CH),
/*------ Getters ---------------------------------------------*/
	CMD_ENTRY(STA_GetError, 0, 0),
	CMD_ENTRY(STA_GetInfo, 1, 0),
	CMD_ENTRY(STA_GetModuleInfo, 2, MOD),
	CMD_ENTRY(STA_GetFreeCycles, 1, 0),
	CMD_ENTRY(STA_GetMaxDspCycleCost, 1, 0),
	CMD_ENTRY(STA_GetDspMemCost, 3, 0),
	CMD_ENTRY(STA_GetMode, 2, MOD | CH),
	CMD_ENTRY(STA_GetFilter, (u8)-1, MOD | CH),
	CMD_ENTRY(STA_GetFilterType, (u8)-1, MOD | CH),
	CMD_ENTRY(STA_GetFilterParam, (u8)-1, MOD | CH),
	CMD_ENTRY(STA_GetFilterParams, (u8)-1, MOD | CH),
	CMD_ENTRY(STA_GetFilterHalvedCoefs, (u8)-1, MOD | CH),
	CMD_ENTRY(STA_GetFilterHalvedCoefsAll, (u8)-1, MOD | CH),
	CMD_ENTRY(STA_MuxGet, 2, MOD),
	CMD_ENTRY(STA_MixerGetRelativeInGain, 3, MOD | CH),
	CMD_ENTRY(STA_MixerGetAbsoluteInGain, 4, MOD | CH),
	CMD_ENTRY(STA_MixerGetOutGain, 3, MOD | CH),
	CMD_ENTRY(STA_GainGetGain, 3, MOD | CH),
	CMD_ENTRY(STA_DelayGetDelay, 3, MOD | CH),
	CMD_ENTRY(STA_DelayGetLength, 3, MOD | CH),
	CMD_ENTRY(STA_LimiterGetParam, -1, MOD | CH),
	CMD_ENTRY(STA_ClipLimiterGetParam, -1, MOD | CH),



//	CMD_ENTRY(STA_ChimeGenSetParam, 3),
//	CMD_ENTRY(STA_PCMSetMaxDataSize, 2),
//	CMD_ENTRY(STA_PCMLoadData_16bit, 3),
//	CMD_ENTRY(STA_PCMPlay, 2),
//	CMD_ENTRY(STA_PCMStop, 1),
//	CMD_ENTRY(STA_ChimeGenSetMaxNumRamps, 2),
//	CMD_ENTRY(STA_ChimeGenPlay, 2),
//	CMD_ENTRY(STA_ChimeGenGetParam, 2),

//	CMD_ENTRY(STA_SetNumChannels, 2),
//	CMD_ENTRY(STA_SetNumOfUpdateSlots, 2),
//	CMD_ENTRY(STA_SetUpdateSlot, 2),

//	CMD_ENTRY(STA_MixerBindTo, 4),
//	CMD_ENTRY(STA_MixerSetBalance, 2),
//	CMD_ENTRY(STA_MixerSetBalanceLimit, 2),
//	CMD_ENTRY(STA_MixerSetBalanceCenter, 4),

};
#undef  MOD
#undef  CH
#undef  CHMASK
#undef  CMD_ENTRY

#define CMD_LUT_SIZE	(sizeof(g_cmd_lut) / sizeof(g_cmd_lut[0]))


//-----------
// STA Parser
//-----------
typedef struct {
	const STAPreset*		 	preset;

	//Module Table
	const STAModuleEntry*		moduleTable;		//Module Table set from the DSP Preset
	u32							moduleTable_size;

	//Module LUT
	const STAModuleLutEntry*	moduleLUT;			//Module LUT set from the DSP Preset
	u32							moduleLUT_size;

	STA_MOD_N_CH_FCT			GetSTAModuleAndChannelFunc;
} tSTAParser;


static tSTAParser g_staParser = {0};


//-----------------------------
// External module lut function
//-----------------------------

void STA_ParserSetPreset(const STAPreset *preset)
{
	tSTAParser* const m = &g_staParser;

	m->preset = preset;
}

void STA_ParserSetModuleTable(const STAModuleEntry* moduleTable, u32 size)
{
	tSTAParser* const m = &g_staParser;

	m->moduleTable		= moduleTable;
	m->moduleTable_size	= size;
}

void STA_ParserSetModuleLUT(const STAModuleLutEntry* moduleLUT, u32 size)
{
	tSTAParser* const m = &g_staParser;

	m->moduleLUT		= moduleLUT;
	m->moduleLUT_size	= size;
}

void STA_ParserSetModuleTranslationFunc(STA_MOD_N_CH_FCT func)
{
	tSTAParser* const m = &g_staParser;

	if (func != NULL) {
		m->GetSTAModuleAndChannelFunc = func;
	}
}

/*
const char* STA_GetSTAFuncName(STA_CmdID cmdID)
{
	static const char* const msg = "Unknown cmdID";
	const char *ret = msg;

	u32 i;
	for (i = CMD_LUT_SIZE - 1; i > 0; i--) {
		if (cmdID == (STA_CmdID)g_cmd_lut[i].cmdID) {
			ret = g_cmd_lut[i].staName;
			break;
		}
	}
	return ret;
}
*/

//----------------------
// GUI to STA conversion
//----------------------
typedef struct {
	u32			guiId;
	u32			guiCh;
	STAModule	staModule;
	u32			staCh;
	char*		name;
} tModuleAndCh;

static void getSTAModuleAndCh(u32 guiId, u32 guiCh, tModuleAndCh* mod);

static void getSTAModuleAndCh(u32 guiId, u32 guiCh, tModuleAndCh* mod)
{
	tSTAParser* const m = &g_staParser;
	static char* const s_noname = "NONAME!";

	if (!mod) {goto _ret;}

	mod->guiId = guiId;
	mod->guiCh = guiCh;
	mod->name  = s_noname;


	//using a user defined callback function for the conversion Gui to STAmodule
	if (m->GetSTAModuleAndChannelFunc)
	{
		u8 chu8 = 0;
		m->GetSTAModuleAndChannelFunc(guiId, guiCh, &mod->staModule, &chu8);
		mod->staCh = chu8; //conv from u8 to u32
	}

	//using a bound STAPreset
	else if (m->preset)
	{
		mod->staModule = STA_GetSTAModuleAndChFromGUI(m->preset, guiId, guiCh, &mod->staCh, &mod->name);
	}

	/*
	//using a bound ModuleLUT (guiId -> moduleId) + a bound ModuleTable (moduleId -> stamodule)
	else if (m->moduleLUT && m->moduleTable)
	{
		int i;
		for (i = m->moduleLUT_size - 1; i > 0; i--) {
			if (m->moduleLUT[i].guiModuleId == guiId && m->moduleLUT[i].guiModuleCh == guiCh) {
				mod->staCh     = m->moduleLUT[i].moduleCh;
				mod->staModule = STA_GetSTAModule2(m->moduleTable, m->moduleTable_size, m->moduleLUT[i].moduleId);
				mod->name      =
				break;
			}
		}
	}
	*/
	else
	{
//		_PRINTF("getSTAModuleAndCh(guiModuleId:%d,...) don't know how to convert the guiModuleId to STAModule\n", guiId);
		//assuming to translation is needed
		mod->staModule = mod->guiId;
		mod->staCh     = mod->guiCh;
	}


_ret:
	return;
}

/*
//lint -esym(960, GET_STAMODULE) inhibits misra2004 19.4 "disallowed definition for macro"  (can't add parenthesis or bracket in this macro or it won't compile)
#define GET_STAMODULE(guiId, guiCh, pModAndCh) \
	do { \
		getSTAModuleAndCh((guiId), (guiCh), (pModAndCh)); \
		if ((pModAndCh)->staModule == (STAModule)0) { \
			goto error_mod;	} \
	} while (0)
*/

//----------------------------------------------------------------
// Audio Cmd Parser
//----------------------------------------------------------------

void STA_ParserCmd(STA_CmdID cmdID, void* data, u32* datasize)
{
	STA_ErrorCode err = STA_NO_ERROR;
	const STACmdLutEntry* cmd = 0;
	tModuleAndCh mod = {0};
	u32* dat = data;
	u32 _dat[2];
	u32 not_processed = 0;
	u32 num, i;
//	u32 cmdGet;


	if (!dat || !datasize) {goto _ret;}

	//Let the preset catch a peculiar cmd.
	if (g_staParser.preset && g_staParser.preset->catchCmd) {
		if (g_staParser.preset->catchCmd((u32*)&cmdID, data, datasize) == 0) {
			goto _ret; //skip the parsing
		}
	}

	//read the flags
//	cmdGet  = (cmdID & STA_GET) ? 1 : 0;
//	cmdID  &= 0x8FFFFFFF;  //remove the flags

	//search if it is a known cmdID
	FORi(CMD_LUT_SIZE) {
		if (cmdID == (STA_CmdID)g_cmd_lut[i].cmdID) {
			cmd = &g_cmd_lut[i];
			break;
		}
	}

	if (!cmd) {
		_PRINTF("STA: Received an unknown cmdID= %d\n", cmdID);
		goto _ret; //cmdID error
	}

//	_PRINTF("%s", cmd->staName);

	//GET STAMODULE
	_dat[0] = dat[0];
	_dat[1] = dat[1];
	if (cmd->flags & HAS_MOD)
	{
		getSTAModuleAndCh(dat[0], (cmd->flags & HAS_CH) ? dat[1] : 0, &mod);

		if (mod.staModule == (STAModule)0) {
//			_PRINTF("%s() error with guiModuleID(%d) or guiCh(%d)\n", cmd->staName, mod.guiId, mod.guiCh);
			_PRINTF("STA cmd %d: error with guiModuleID(%d) or guiCh(%d)\n", cmd->cmdID, mod.guiId, mod.guiCh);
			goto _ret;
		}

		if (cmd->flags & HAS_CHMASK) {
			mod.staCh = mod.guiCh;	// = original chmask passed as dat[1]
		}

		if (cmd->flags & HAS_CH) {
			_dat[1] = mod.staCh; 	// = translated ch
		}
		_dat[0] = mod.staModule;	// = translated module
	}


	//parse and process the STA cmd
	switch (cmdID)
	{
	case STA_GetError_ID: //getter
		dat[0] = (u32)STA_GetError();
		*datasize = sizeof(STA_ErrorCode);
		break;

	case STA_GetInfo_ID: //getter
		dat[1] = (u32)STA_GetInfo((STA_Info)dat[0]);
		*datasize = 2*sizeof(u32);
		break;

	case STA_GetModuleInfo_ID: //getter
		dat[2] = (u32)STA_GetModuleInfo(mod.staModule, (STA_ModuleInfo)dat[1]);
		*datasize = 3*sizeof(u32);
		break;

	case STA_Init_ID: //not generic because of passing a buffer
		STA_Init(&dat[0]);
		break;

	case STA_MuxSet_ID:	//not generic because passing a buffer
	case STA_MuxGet_ID:
		cmd->staFunc(mod.staModule, &dat[1]);
		num = STA_GetModuleInfo(mod.staModule, STA_INFO_NUM_OUT_CHANNELS);
		*datasize = sizeof(u32) + num * sizeof(u8);
		break;

	case STA_SetMode_ID:	//not generic because the uart cmd takes an additionnal 'ch' vs the STA API !
		cmd->staFunc(mod.staModule, dat[2]);
		break;

	case STA_GetMode_ID:	//not generic because the uart cmd takes an additionnal 'ch' vs the STA API !
		dat[2] = STA_GetMode(mod.staModule);
		*datasize = 3*sizeof(u32);
		break;

	case STA_GetFreeCycles_ID: //getter
		(void)STA_GetFreeCycles(&dat[0]); //dat = (u32) freeCycles[3]
		*datasize = 3*sizeof(u32);
		break;

	case STA_GetMaxDspCycleCost_ID: //getter
		dat[1] = STA_GetMaxDspCycleCost((STA_Dsp)dat[0]);
		*datasize = 2*sizeof(u32);
		break;

	case STA_GetDspMemCost_ID: //getter
		STA_GetDspMemCost((STA_Dsp)dat[0], &dat[1], &dat[2]);
		*datasize = 3*sizeof(u32);
		break;

	//Where to get the bin data? uart buffer? In this case shall we design a separate Load for each mems?
/*	case STA_LoadDSP_ext_ID:
		_PRINTF("STA_LoadDSP_ext(dsp%d,,,,,,)", dat[0]);
		STA_LoadDSP_ext((STA_Dsp)dat[0],,,,,,);
		break;
*/

	case STA_AddModule2_ID: //note generic because of 'name'
		(void)STA_AddModule2((STA_Dsp)dat[0], (STA_ModuleType)dat[1], dat[2], dat[3], (const char*)&dat[4]);
		break;

	case STA_DeleteModule_ID: //TODO: shall we had a ch?
		STA_DeleteModule((STAModule*)mod.staModule);
		break;

	//---------- Generics --------------------

	//0 param, no return
	case STA_Reset_ID:
	case STA_Exit_ID:
	case STA_Play_ID:
	case STA_Stop_ID:
	case STA_DMAStart_ID:
	case STA_DMAStop_ID:
	case STA_DMAReset_ID:
	case STA_BuildFlow_ID:
		((FUNCVOID)(u32)cmd->staFunc)();
		break;

	//1 param, no return
	case STA_LoadDSP_ID:
	case STA_StartDSP_ID:
	case STA_StopDSP_ID:
	case STA_Enable_ID:
	case STA_Disable_ID:
		cmd->staFunc(dat[0]);
		break;

	//stamod + 2 params   (eg. stamod, ch, val)
	case STA_GainSetGain_ID:
	case STA_GainSetGains_ID:				//assuming no conversion on the chmask
	case STA_DelaySetDelay_ID:
	case STA_DelaySetDelays_ID:				//assuming no conversion on the chmask
	case STA_DelaySetLength_ID:
	case STA_MixerSetAbsoluteInGain_ID:
	case STA_MixerSetOutGain_ID:
	case STA_MuxSetOutChannel_ID:
	case STA_ToneSetTypes_ID:
	case STA_LoudSetTypes_ID:
		cmd->staFunc(mod.staModule, _dat[1], dat[2]);
		break;

	//stamod + 1 params + 1 ret (get)
	case STA_GainGetGain_ID:
	case STA_DelayGetDelay_ID:
	case STA_DelayGetLength_ID:
	case STA_MixerGetAbsoluteInGain_ID:
	case STA_MixerGetOutGain_ID:
		dat[2] = ((STAFUNCRET)(u32)cmd->staFunc)(mod.staModule, _dat[1]);
		*datasize = 3*sizeof(u32);
		break;

	//stamod + 3 params
	case STA_SmoothGainSetRamp_ID:			//assuming no conversion on the chmask
		cmd->staFunc(mod.staModule, _dat[1], dat[2], dat[3]);
		break;

	//stamod + 2 params + 1 ret (get)
	case STA_LimiterGetParam_ID:
	case STA_ClipLimiterGetParam_ID:
		dat[3] = ((STAFUNCRET)(u32)cmd->staFunc)(mod.staModule, _dat[1], dat[2]);
		*datasize = 4*sizeof(u32);
		break;

	//stamod + 4 params
	case STA_ToneSetAutoBassQLevels_ID:
	case STA_LoudSetAutoBassQLevels_ID:
	case STA_SinewaveGenSet_ID:
	case STA_SinewaveGenPlay_ID:
		cmd->staFunc(mod.staModule, _dat[1], dat[2], dat[3], dat[4]);
		break;

	//stamod + 5 params
	case STA_LoudSetAutoGLevels_ID:
		cmd->staFunc(mod.staModule, _dat[1], dat[2], dat[3], dat[4], dat[5]);
		break;

	//---------- Mixer --------------------

	case STA_MixerSetRelativeInGain_ID:
	case STA_MixerGetRelativeInGain_ID: //translating outch (dat[1]), not inRel.
		if (cmdID == STA_MixerSetRelativeInGain_ID) {
			cmd->staFunc(mod.staModule, mod.staCh, dat[2], dat[3]);
		} else {
			dat[3] = (u32)STA_MixerGetRelativeInGain(mod.staModule, mod.staCh, dat[2]);
		}
		*datasize = 4*sizeof(u32);
		break;

	case STA_MixerSetChannelInGains_ID:		//note: not generic because passing a pointer
		cmd->staFunc(mod.staModule, mod.staCh, &dat[2]);
		break;

	//---------- EQ -----------------------

	case STA_SetFilter_ID:	//(note: the cmds take a ch but not the sta apis!)
	case STA_GetFilter_ID:
		if (cmdID == STA_SetFilter_ID) {
			cmd->staFunc(mod.staModule, dat[2], dat[3], dat[4], dat[5]);
		} else {
			cmd->staFunc(mod.staModule, dat[2], &dat[3], &dat[4], &dat[5]);
		}
		*datasize = 6*sizeof(s32);
		break;

	case STA_SetFilterType_ID:	//(note: the cmds take a ch but not the sta apis!)
	case STA_GetFilterType_ID:
		if (cmdID == STA_SetFilterType_ID) {
			cmd->staFunc(mod.staModule, dat[2], dat[3]);
		} else {
			dat[3] = (u32)STA_GetFilterType(mod.staModule, dat[2]);
		}
		*datasize = 4*sizeof(u32);
		break;

	case STA_SetFilterParam_ID:	//(note: the cmds take a ch but not the sta apis!)
	case STA_GetFilterParam_ID:
		if (cmdID == STA_SetFilterParam_ID) {
			cmd->staFunc(mod.staModule, dat[2], dat[3], dat[4]);
		} else {
			dat[4] = (u32)STA_GetFilterParam(mod.staModule, dat[2], (STA_FilterParam)dat[3]);
		}
		*datasize = 5*sizeof(s32);
		break;

	case STA_SetFilterParams_ID:
	case STA_GetFilterParams_ID:
		cmd->staFunc(mod.staModule, dat[2], &dat[3]);
		num = STA_GetModuleInfo(mod.staModule, STA_INFO_NUM_BANDS);
		*datasize = 3*sizeof(u32) + num * sizeof(s16);
		break;

	case STA_SetFilterHalvedCoefs_ID:
	case STA_GetFilterHalvedCoefs_ID:
		cmd->staFunc(mod.staModule, dat[2], &dat[3]);
		*datasize = 3*sizeof(s32) + 6*sizeof(q23);
		break;

	case STA_SetFilterHalvedCoefsAll_ID:
	case STA_GetFilterHalvedCoefsAll_ID:
		cmd->staFunc(mod.staModule, dat[2], &dat[3]);
		num = STA_GetModuleInfo(mod.staModule, STA_INFO_NUM_BANDS);
		*datasize = 3*sizeof(s32) + num*6*sizeof(q23);
		break;


	//---------- Chime --------------------

	case STA_ChimeGenBeep_ID:		//note: not generic because passing a pointer
		cmd->staFunc(mod.staModule, &dat[1]);
		break;

	case STA_ChimeGenStop_ID:
		cmd->staFunc(mod.staModule, dat[1], dat[2]);
		break;

#if 0
	case STA_ChimeGenGetParam_ID:
		*(s32*)dat = STA_ChimeGenGetParam(mod.staModule, dat[1]);
		*datasize = sizeof(s32);
		break;
#endif

	//---------- Limiter/Compander --------------------

	//3 params: stamod + paramMask + ptr
	case STA_LimiterSetParams_ID:
	case STA_CompanderSetParams_ID:
	case STA_ClipLimiterSetParams_ID:
		cmd->staFunc(mod.staModule, dat[1], &dat[2]);
		break;

/*
	case STA_LimiterGetParams_ID:
		STA_LimiterGetParams(mod.staModule, (const void*)&dat[2]);
		*datasize = sizeof(s32) + STA_LMTR_NUMBER_OF_IDX*sizeof(s16);
		break;
*/

	//---------- NOT PROCESSED CMDs --------------------
	default:
//		_PRINTF("%s()... NOT PROCESSED\n", cmd->staName);
		_PRINTF("STA cmd %d NOT PROCESSED\n", cmd->cmdID);
		not_processed = 1;
		break;
	}


/*
	//parse OTHER standard CMDs, based on number of params (first param being an STAModule) AND without returned value
	switch (cmd->numParams)
	{
	case 0: cmd->staFunc(); goto check_error;
	case 1: cmd->staFunc(dat[0]); goto check_error;
	case 2: cmd->staFunc(dat[0], dat[1]); goto check_error;
	case 3: cmd->staFunc(dat[0], dat[1], dat[2]); goto check_error;
	case 4: cmd->staFunc(dat[0], dat[1], dat[2], dat[3]); goto check_error;
	case 5: cmd->staFunc(dat[0], dat[1], dat[2], dat[3], dat[4]); goto check_error;
	case 6: cmd->staFunc(dat[0], dat[1], dat[2], dat[3], dat[4], dat[5]); goto check_error;

	//NOT PROCESSED CMDs
	default:
		_PRINTF("NOT PROCESSED\n");
		not_processed = 1;
		sta_assert(0); //cmd not processed
		break;
	}
*/
	if (not_processed) {goto _ret;}

	//check STA error
	STA_Push(STA_CALL_TRACE, 0);
	err = (STA_ErrorCode)STA_GetInfo(STA_INFO_LAST_ERROR);
	STA_Pop(STA_CALL_TRACE);

	if (err != STA_NO_ERROR) {
		_PRINTF("... STA_ErrorCode= %d\n", err);
	} else {
//		_PRINTF("\n%s", "");
	}

_ret: return;
}

