/***********************************************************************************/
/*!
*  \file      sta_preset_loader.c
*
*  \brief     <i><b> STAudioLib Preset Loader </b></i>
*
*  \details   STAudioLib Preset Loader:
*
*
*  \author    Quarre Christophe
*
*  \author    (original version) Quarre Christophe
*
*  \version
*
*  \date      2016/05/20
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
#include "sta_preset_loader.h"


//#define _PRINTF		STA_PRINTF


//-------------------------------------------------------
//
//-------------------------------------------------------
/*
static u32 findModIdxFromId(const STAModuleEntry* mods, int numMods, u32 id)
{
	int i;
	FORi(numMods) {
		if (mods[i].id == id)
			break;
	}
	sta_assert(i < numMods); //assumed to find an idx
	return i;
}
*/

STAModule STA_GetSTAModule(const STAPreset *m, u32 id)
{
	STAModule ret = 0;
	u32 i;

	if (!m || !m->moduleTable) {goto _ret;}

	if (id >= (u32)STA_XIN0) {
		ret = id; goto _ret;
	}

	FORi(m->moduleTable_size) {
		if (m->moduleTable[i].id == id) {
			ret = m->moduleTable[i].staModule;
			break;
		}
	}
	//sta_assert(i < m->moduleTable_size); //assumed to find an stamodule

_ret: return ret;
}

STAModuleEntry* STA_GetModuleEntry(const STAPreset *m, u32 id)
{
	STAModuleEntry* ret = 0;
	u32 i;

	if (!m || !m->moduleTable) {goto _ret;}

	FORi(m->moduleTable_size) {
		if (m->moduleTable[i].id == id) {
			ret = &m->moduleTable[i];
			break;
		}
	}
	//sta_assert(i < m->moduleTable_size); //assumed to find an stamodule

_ret: return ret;
}

//same as STA_GetSTAModule() but not passing a ptr to STAPreset (in case STAPreset is not used)
STAModule STA_GetSTAModule2(const STAModuleEntry* mods, int numMods, u32 id)
{
	STAModule  ret = 0;
	int i;

	if (!mods) {goto _ret;}

	if (id >= (u32)STA_XIN0) {
		ret = id; goto _ret;
	}

	FORi(numMods) {
		if (mods[i].id == id) {
			ret = mods[i].staModule;
			break;
		}
	}
	//sta_assert(i < numMods); //assumed to find an stamodule

_ret: return ret;
}

//-------------------------------------------------------
/*
STAModule STA_GetSTAModuleAndChFromGUI(const STAPreset *m, u32 guiModuleId, u32 guiCh, u32* stach)
{
	STAModule ret = 0;
	u32 i;

	//guiModule -> moduleId -> staModule
	for (i = m->moduleLUT_size - 1; i > 0; i--) {
		if (m->moduleLUT[i].guiModuleId == guiModuleId && m->moduleLUT[i].guiModuleCh == guiCh) {
			*stach = m->moduleLUT[i].moduleCh;
			ret = STA_GetSTAModule(m, m->moduleLUT[i].moduleId);
			break;
		}
	}
_ret: return ret;
}
*/

STAModule STA_GetSTAModuleAndChFromGUI(const STAPreset *m, u32 guiModuleId, u32 guiCh, u32* staCh, char** modName)
{
	STAModule ret = 0;
	STAModuleEntry* modEntry;
	u32 i;
	if (!m || !staCh || !m->moduleLUT) {goto _ret;}

	//guiModule -> moduleId -> staModule
	for (i = m->moduleLUT_size - 1; i > 0; i--) {
		if (m->moduleLUT[i].guiModuleId == guiModuleId && m->moduleLUT[i].guiModuleCh == guiCh) {
			*staCh = m->moduleLUT[i].moduleCh;
			modEntry = STA_GetModuleEntry(m, m->moduleLUT[i].moduleId);
			if (modEntry) {
				if (modName) {*modName = modEntry->name;}
				ret = modEntry->staModule;
			}
			break;
		}
	}

_ret: return ret;
}
//-------------------------------------------------------
const char* STA_GetPresetModuleName(const STAPreset *m, u32 id)
{
	static const char* const msg = "unknown module";
	const char *ret = msg;

	u32 i;
	if (!m || !m->moduleTable) {goto _ret;}

	FORi(m->moduleTable_size) {
		if (m->moduleTable[i].id == id) {
			ret = m->moduleTable[i].name;
			break;
		}
	}

_ret: return ret;
}

//-------------------------------------------------------
void STA_BuildPreset(STAPreset *m, u32 flags)
{
	if (!m) {goto _ret;}{

	STAModuleEntry      *mod;
	STAConnectionEntry  *con;

	u32 numMods = m->moduleTable_size;
	u32 numCons = m->connectionTable_size;

	u32 startDSPs = 0;

//	STA_ErrorCode err;
	u32 i,j;


	//flags not used for now.
	(void) flags;


	STA_Reset();

	//----- ADD MODULES ------

	mod = m->moduleTable;
	for (i = 0; i < numMods; i++)
	{
		STA_ModuleType type = mod->staType;

		mod->staModule = STA_AddModule2((STA_Dsp)mod->dsp, type, mod->id, 0, mod->name);

		//----- PRE-BUILD INITS -----

		//set num channel (for some types)
		if (STA_GetModuleInfo(mod->staModule, STA_INFO_IS_NCH_CHANGEABLE)){
			mod->staModule = STA_SetNumChannels(mod->staModule, mod->nout);}

		//EQ
		if (STA_EQUA_STATIC_1BAND_DP <= type && type <= STA_EQUA_STATIC_16BANDS_SP)
		{
			//note: set ON at init like on the SAT GUI
			//STA_SetMode(mod->staModule, STA_EQ_ON);
		}

		//MUX
		else if (STA_MUX_2OUT <= type && type <= STA_MUX_10OUT)
		{
			sta_assert(mod->nin <= STA_MAX_MUX_INS);

			//by default, set out[i] = in[i]
			FORj(MIN(mod->nin, mod->nout)){
				STA_MuxSetOutChannel(mod->staModule, j, j);}
		}

		//MIXER
		else if (STA_MIXE_2INS_NCH <= type && type <= STA_MIXE_9INS_NCH)
		{
			sta_assert(mod->nout <= STA_MAX_MIXER_CH);
		}

		//DELAY
		else if (STA_DLAY_Y_2CH <= type && type <= STA_DLAY_X_6CH)
		{
			STA_DelaySetLengths(mod->staModule, 0xFF, mod->initDelayLength);
		}

		//CHIME
		else if (type == STA_CHIMEGEN)
		{
			STA_ChimeGenSetMaxNumRamps(mod->staModule, mod->initChimeMaxNumRamps);
		}

		//CHIME
		else if (type == STA_PCMCHIME_12BIT_X_2CH || type == STA_PCMCHIME_12BIT_Y_2CH)
		{
			STA_PCMSetMaxDataSize(mod->staModule, mod->initPCMMaxDataSize);
		}

		//SPECTRUM METER
		else if (type == STA_SPECMTR_NBANDS_1BIQ)
		{
			mod->staModule = STA_SetFilterNumBands(mod->staModule, mod->initSpecMeterNumBands);
		}

		//OTHERS
		else {switch (type)
		{
			default: sta_assert(1);
		}}

		//check error
		sta_assert(STA_GetError() == STA_NO_ERROR);

		//check which DSP to start
		startDSPs |= (1u << mod->dsp);

		mod++;
	}


	//----- CONNECTIONS ------

	con = m->connectionTable;
	for (i = 0; i < numCons; i++)
	{
		STAModule from = STA_GetSTAModule(m, con->from);
		STAModule to   = STA_GetSTAModule(m, con->to);

		STA_Connect(from, con->chout, to, con->chin);

//		sta_assert(STA_GetError() == STA_NO_ERROR);

		con++;
	}


	//----- BUILD FLOW -----

	STA_BuildFlow();

	sta_assert(STA_GetError() == STA_NO_ERROR);


	//----- START DSPs -----
	//START DSPs
	//note: calling STA_Play() would start the DSP with at least one connection, but also the DMABUS

	if (startDSPs & (1 << (u32)STA_DSP0)){
		STA_StartDSP(STA_DSP0);}

	if (startDSPs & (1 << (u32)STA_DSP1)){
		STA_StartDSP(STA_DSP1);}

	if (startDSPs & (1 << (u32)STA_DSP2)){
		STA_StartDSP(STA_DSP2);}

	//STA_SLEEP(100);					//wait 100ms for DSP initialisation
	startDSPs = STA_WaitDspReady(startDSPs, 500);

	sta_assert(startDSPs);
	(void) startDSPs;

	//----- START DMABUS -----


	//----- POST_INIT -----
	//TODO: move in the pre-build section whenever possible.
	//     (the goal is to return from the function before the SLEEP)

	mod = m->moduleTable;
	for (i = 0; i < numMods; i++)
	{
		STA_ModuleType type = mod->staType;

		//GAIN
		if (STA_GAIN_STATIC_NCH <= type && type <= STA_GAIN_LINEAR_NCH)
		{
			STA_GainSetGains(mod->staModule, 0xFFFF, mod->initGain);
		}

		//MIXER
		if (STA_MIXE_2INS_NCH <= type && type <= STA_MIXE_9INS_NCH)
		{
			//Since all linear gains are 0 by default, we need to set some gains.
			//Each mixer applies x InGains x Softmute x volume
			STA_MixerSetInGains(mod->staModule, 0xFFFF, 0xFFFF, 0);		//set all inGains to 0 dB
			STA_MixerSetVolume(mod->staModule, 0xFFFF, 0);			 	//set all outvolumes to 0 dB

//			STA_MixerSetMute(mod->staModule, 0xFFFF, FALSE, FALSE, 1);	//don't update depth
		}

		mod++;
	}

	}
_ret: return;
}

