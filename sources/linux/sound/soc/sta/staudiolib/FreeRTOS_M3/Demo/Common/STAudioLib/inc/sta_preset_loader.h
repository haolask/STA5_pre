/***********************************************************************************/
/*!
*  \file      sta_preset_loader.h
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

#ifndef _STA_PRESET_LOADER_H
#define _STA_PRESET_LOADER_H


#define STA_PRESET_LOADER_VERSION		3


#include "staudio.h"

#ifdef TKERNEL
#pragma anon_unions
#endif


//-------------------------------
//Module Entry (of Module Table)
//-------------------------------
typedef struct STAModuleEntry_s {
	u8				id, nin, nout, dsp;		//note: 'nin' not used for MIXERS
	STA_ModuleType	staType;
	STAModule		staModule;				//handle returned by STA_AddModule()
	char*			name;
	//general purpose param
	union {
		s32			initGain;
		u32			initDelayLength;
		u32			initChimeMaxNumRamps;
		u32			initPCMMaxDataSize;
		u32			initSpecMeterNumBands;
	};// GPparam;
} STAModuleEntry;

#define MOD_ENTRY(id, name, type, nin, nout, dsp, GPparam)		{(id), (nin), (nout), (dsp), (type), 0, (name), {(GPparam)}}


//-------------------------------
//Connection Entry (of Connection Table)
//-------------------------------
typedef struct STAConnectionEntry_s {
	u8			from, chout;		//from channel out
	u8			to,   chin;			//to channel in
} STAConnectionEntry;

#define CON_ENTRY(from, chout, to, chin) 		{(from), (chout), (to), (chin)}


//-------------------------------
//ModuleLUT Entry (for GUI to DSP module translation)
//-------------------------------
typedef struct STAModuleLutEntry_s {
	u8			guiModuleId;
	u8			guiModuleCh;
	u8			moduleId;
	u8			moduleCh;
} STAModuleLutEntry;

#define MODLUT_ENTRY(guiModId, guiModCh, modId, modCh)	{(guiModId), (guiModCh), (modId), (modCh)}


//-------------------------------
// Preset
//-------------------------------
typedef struct STAPreset_s {
	//Modules
	STAModuleEntry*		moduleTable;
	u32				 	moduleTable_size;

	//Connections
	STAConnectionEntry*	connectionTable;
	u32					connectionTable_size;

	//GUI to DSP module LUT
	STAModuleLutEntry*	moduleLUT;
	u32					moduleLUT_size;

	//optional callback if a preset needs to catch a particular cmd from the cmd parser
	int (*catchCmd)(u32 *pCmdID, void* _data, u32* bytesize);	//returning 0 skips the cmdParsing

} STAPreset;


/*
typedef enum STA_BuildFlag_e {

} STA_BuildFlag;
*/

void STA_BuildPreset(STAPreset *m, u32 flags);

STAModuleEntry* STA_GetModuleEntry(const STAPreset *m, u32 id);

STAModule STA_GetSTAModule(const STAPreset *m, u32 id);
STAModule STA_GetSTAModule2(const STAModuleEntry *mods, int numMods, u32 id);

const char* STA_GetPresetModuleName(const STAPreset *m, u32 id);

STAModule STA_GetSTAModuleAndChFromGUI(const STAPreset *m, u32 guiModuleId, u32 guiCh, u32* staCh, char** modName);


#endif //_STA_PRESET_LOADER_H

