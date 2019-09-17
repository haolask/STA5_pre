/***********************************************************************************/
/*!
*
*  \file      API.c
*
*  \brief     <i><b> STAudioLib APIs </b></i>
*
*  \details   STAudioLib API for Emerald DSP on Accordo2:
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

#ifdef CORTEX_M3
#include "sta10xx_nvic.h"
#endif

#undef STA_API
#define STA_API

#if (STA_LIB_VERSION != 57)
#error "STA: staudio.h NOT matching API.c !!"
#endif

//hack for emIDE
#if !defined(MIXER_VER_1) && !defined (MIXER_VER_2) && !defined (MIXER_VER_3)
#define MIXER_VER_3
#endif

//1/eps for exp ramp for Limiter, ClipLim and Compander
#define LIM_OOEPS	((f32)10)


//external globals
extern const char* const g_errorMsg[STA_NUM_ERROR_CODE];
extern const char* const g_STA_ModuleTypeStr[STA_MOD_LAST];
extern const char* const g_STA_FilterParamStr[4];
extern const char* const g_STA_FilterTypeStr[STA_BIQUAD_NUM];
extern const char* const g_STA_ChimeGenParamStr[4];
extern const char* const g_STA_EnableParamStr[2];


//internal prototypes
static const char* MODNAME(const tModule *mod);
static void DRV_Init(void* dev);
static STA_ErrorCode _STA_StartDSP(STA_Dsp core);
static bool _isNumChChangeable(STA_ModuleType type);
static void EnableFreeCycleCount(bool enable);
static tModule* findModuleByID(u32 id);
static tModule* findModuleByName(const char* name);
static u32 generateModuleId(void);
static void BackupLastConnections(tListElmt* oldLast[3]);
static void DeleteNewConnections(const tListElmt* oldLast[3]);
static tModule* getModuleOrIO(STAModule stamod);


//---- DSP_BASE -------------------------------------------
/*
DSP_BASE[3] is defined in
- sta10xx_dsp.c in FREE_RTOS
- api.c (below) and set at compile time in TKERNEL
- api.c (below) and set at runtime time in LINUX_OS
*/

#ifdef TKERNEL
u32 DSP_BASE[3] = {DSP0_BASE, DSP1_BASE, DSP2_BASE};
#endif

#ifdef LINUX_OS
u32 DSP_BASE[3] = {0};
void *AUDIO_CR_BASE = 0;
void *AUDIO_P4_BASE = 0;
#endif

//---- Globals -------------------------------------------
//(scatter these until we want to gather into g_driver)
tDSP			g_dsp[3];
tDriver 		g_drv;

#if STA_WITH_DMABUS
static tDMABUS	g_dma;
#endif

//---- TRACE -------------------------------------------

//lint -esym(960, e9022, STA_TRACE) inhibits misra2004 19.10 "unparenthesized macro parameter" because of __VA_ARGS__
//lint -estring(9022, STA_TRACE)
#define STA_TRACE(fmt, ...) \
	do {if (g_drv.m_trace_sta_calls) {STA_PRINTF(fmt, __VA_ARGS__);}} while (0)

#define STA_TRACE_PUSH(enable) \
	do {g_drv.m_trace_sta_calls_b = g_drv.m_trace_sta_calls; g_drv.m_trace_sta_calls = (u32)(enable);} while (0)

#define STA_TRACE_POP() \
	do {g_drv.m_trace_sta_calls = g_drv.m_trace_sta_calls_b;} while (0)

//#define MODNAME(mod)	((mod) ? (mod)->m_name : "?")

static const char* MODNAME(const tModule *mod)
{
	static char s_modname[16];
	const char* ret = s_modname;

	if (mod) {
		//return the name or replace with the 'id' if "NONAME!"
		if (mod->m_name[6] == '!') {
			STA_SPRINTF(s_modname, "id:%d", mod->m_id);
		} else {
			ret = mod->m_name;
		}
	} else {
		s_modname[0] = '?';
		s_modname[1] = '\0';
	}

	return ret;
}


//===========================================================================
// Driver management
//===========================================================================

void _SetError(STA_ErrorCode err, u32 line)
{
#ifdef LINUX_OS
#define __SHORTFILE__ strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__

	dev_err((struct device *)g_drv.m_dev, "%s:%d: STA: Error %s !\n",
			__SHORTFILE__, line, g_errorMsg[err]);
#else
	STA_PRINTF("%s:%d: STA Error %s !\n", __FILE__, line, g_errorMsg[err]);
#endif

	//first error (till last getError())
	if (g_drv.m_error == STA_NO_ERROR) {
		g_drv.m_error = err; }

	//last error
	g_drv.m_errorLast = err;
}


//not used
/*
//returns error code
static int CheckFilterValue(STA_FilterParamType param, s32 val)
{
	switch (param) {
	case STA_GAIN: if (-24*BGS <= val && val <= 24*BGS ) return 0; break;
	case STA_QUAL: if (      1 <= val && val <= 100 ) return 0; break;
	case STA_FREQ: if (     20 <= val && val <= 20000 ) return 0; break;
	default: sta_assert(0); break;
	}
	//invalid value
	SetError(STA_INVALID_VALUE);
	return STA_INVALID_VALUE;
}
*/

//----------------------------------------------------------------------------
STA_API STA_ErrorCode STA_GetError(void)
{
	STA_ErrorCode err = g_drv.m_error;

	g_drv.m_error = STA_NO_ERROR;					//reset the error

	STA_TRACE("%s() = %s\n", __FUNCTION__, g_errorMsg[err]);
	return err;
}
//----------------------------------------------------------------------------
static void DRV_Init(void* dev)
{
	g_drv.m_error 		= STA_NO_ERROR;
	g_drv.m_errorLast	= STA_NO_ERROR;
	g_drv.m_isPlaying 	= 0;
	g_drv.m_dev			= dev;
	g_drv.m_fsyncISR	= 0;
	g_drv.m_lastModuleId = 0;
}
//----------------------------------------------------------------------------
STA_API void STA_Init(void* p)
{
	void* dev = p;

	STA_TRACE("%s()\n", __FUNCTION__);

	if (g_dsp[0].m_xin.m_refCode == STA_MODULE_REF_CODE) {
		goto _ret;} //STA is already initialized

//	STA_PRINTF("STA: Initializing... ", 0);

	//cut version
#ifdef ACCORDO2
	g_drv.m_cutVersion	= *((vu32*) 0x10047FF8) & 0xFFF;
//	g_drv.m_cutVersion = 0x100;	//force CUT1
#else
#ifndef LINUX_OS
#error "must define ACCORDO2"
#endif
#endif


#ifdef LINUX_OS
	if (p) {
		AUDIO_CR_BASE = ((STA_Params*)p)->audio_cr_base;
		AUDIO_P4_BASE = ((STA_Params*)p)->audio_p4_base;
		DSP_BASE[0]   = (u32)((STA_Params*)p)->dsp0_base;
		DSP_BASE[1]   = (u32)((STA_Params*)p)->dsp1_base;
		DSP_BASE[2]   = (u32)((STA_Params*)p)->dsp2_base;
		g_drv.m_cutVersion = ((STA_Params*)p)->cutversion;
		dev = ((STA_Params*)p)->dev;
	}
#endif

	//Driver
	DRV_Init(dev);

	//DMABus
#if STA_WITH_DMABUS
	DMAB_Reset(&g_dma);
#endif

	//DSPs
	DSP_Init(&g_dsp[0], STA_DSP0, DSP_BASE[0]);
	DSP_Init(&g_dsp[1], STA_DSP1, DSP_BASE[1]);
	DSP_Init(&g_dsp[2], STA_DSP2, DSP_BASE[2]);

	//enable IRQs
#ifdef CORTEX_M3
	NVIC_EnableIRQChannel(EXT0_IRQChannel,  0, 0);	//enable DSP0 IRQ
	NVIC_EnableIRQChannel(EXT4_IRQChannel,  0, 0);	//enable DSP1 IRQ
	NVIC_EnableIRQChannel(EXT1_IRQChannel,  0, 0);	//enable DSP2 IRQ
	NVIC_EnableIRQChannel(EXT13_IRQChannel, 0, 0);	//enable FSYNC IRQ
#else
	//TODO: set VIC for R4...
#endif

//	STA_PRINTF("done.\n", 0);
_ret: return;
}

//----------------------------------------------------------------------------
STA_API void STA_Reset(void)
{
	u32 i;

	STA_TRACE("%s()\n", __FUNCTION__);

#if STA_WITH_DMABUS
	DMAB_Reset(&g_dma); //empty and stop
#endif

	FORi(3) {
		if (DSP_isLoaded(&g_dsp[i])) {
			DSP_Stop_(&g_dsp[i]);
			DSP_Clean(&g_dsp[i]);
		}
	}

	DRV_Init(g_drv.m_dev);
}
//----------------------------------------------------------------------------
STA_API void STA_Exit(void)
{
	STA_TRACE("%s()\n", __FUNCTION__);

	STA_Reset();
}

//===========================================================================
// DSP management
//===========================================================================
/*
//version NOT needed for now
STA_API int STA_LoadDSPCode( STA_Dsp core, const void* addr, int size )
{
	if ((u32)core > STA_DSP2 ) {
		SetError(STA_INVALID_PARAM); return 0; }

	//...

	return 1;
}
*/
STA_API void STA_LoadDSP( STA_Dsp core )
{
	tDSP* dsp = &g_dsp[core];

	STA_TRACE("%s(%d)\n", __FUNCTION__, core);

	CHECK_PARAM((u32)core <= (u32)STA_DSP2);
	CHECK_INITIALIZED(dsp->m_axi);

	DSP_Load(dsp);

	//check that XMEM and YMEM are loaded
	CHECK_LOADED(dsp);

_ret: return;
}
//----------------------------------------------------------------------------
STA_API void STA_LoadDSP_ext( STA_Dsp core, const char *Paddr, u32 Psize, const char *Xaddr, u32 Xsize, const char *Yaddr, u32 Ysize )
{
	tDSP* dsp = &g_dsp[core];

	STA_TRACE("%s(%d,...)\n", __FUNCTION__, core);

	CHECK_PARAM((u32)core <= (u32)STA_DSP2);
	CHECK_INITIALIZED(dsp->m_axi);

	DSP_Load_ext(dsp, Paddr, Psize, Xaddr, Xsize, Yaddr, Ysize);

	//check that XMEM and YMEM are loaded
	CHECK_LOADED(dsp);

_ret: return;
}

//----------------------------------------------------------------------------
//Release DSP from reset
//return error
static STA_ErrorCode _STA_StartDSP( STA_Dsp core )
{
	STA_ErrorCode ret = STA_NO_ERROR;
	tDSP* dsp = &g_dsp[core];
	volatile tAXI* axi =  g_dsp[core].m_axi;
//	int sleepcounter = 4;

	/* (can't use CHECK_ macros because we want to return the error)
	CHECK_PARAM((u32)core <= STA_DSP2);
	CHECK_INITIALIZED(axi);
	CHECK_LOADED(dsp);
	*/
	if (!axi) {ret = STA_NOT_INITIALIZED; goto _ret;}
	if ((u32)core > (u32)STA_DSP2) {ret = STA_INVALID_PARAM; goto _ret;}
//#if CHECK_DSP
	if (DSP_READ(&dsp->m_axi->chk_xmem_loaded) != DSP_XMEM_LOADED_CODE) {
		ret = (STA_ErrorCode)((u32)STA_DSP0_XMEM_NOT_LOADED + (u32)core); goto _ret;}
	if (DSP_READ(&dsp->m_ayi->chk_ymem_loaded) != DSP_YMEM_LOADED_CODE) {
		ret = (STA_ErrorCode)((u32)STA_DSP0_YMEM_NOT_LOADED + (u32)core); goto _ret;}
//#endif
	//check already started
	if (DSP_READ(&axi->chk_dsp_running) == DSP_RUNNING_CODE) {
		dsp->m_isRunning = 1;
		goto _ret; //DSP already started (silent error)
	}


	//Checking valid flow
	//NOTE: Here we don't check if a valid flow is built (because for testing
	//      purpose the DSP	can be started with the default stereo passthrough
	//      firmware. However, a valid flow is checked if calling STA_Play()

	//start dsp now...
	DSP_Start_(dsp);

	//Checking DSP is running
	//NOTE: Here we should check that the dsp is correctly running
	//      ->  (axi->chk_dsp_running == DSP_RUNNING_CODE)
	//		and that it has successfully completed the module initialisations
	//      ->  (axi->isInitsDone == 1),
	//		BUT let's avoid using sleep for now...

/*
	//wait that the dsp responds
	//sleep();

	//Check that the dsp is running
	if (axi->chk_dsp_running != DSP_RUNNING_CODE) {
		dsp->m_isRunning = 0;
		ret = STA_DSP0_NOT_STARTED + core;
		goto _ret;
	}
*/
	//we assume that the dsp is running
	dsp->m_isRunning = 1;

_ret: return ret;
}

STA_API void STA_StartDSP( STA_Dsp core )
{
	STA_ErrorCode err;

	STA_TRACE("%s(%d)\n", __FUNCTION__, core);

	err = _STA_StartDSP(core);
	if (err) {SetError(err);}
}
//----------------------------------------------------------------------------
STA_API void STA_StopDSP( STA_Dsp core )
{
	tDSP* dsp = &g_dsp[core];

	STA_TRACE("%s(%d)\n", __FUNCTION__, core);

	CHECK_PARAM((u32)core <= (u32)STA_DSP2);
	CHECK_INITIALIZED(dsp->m_axi);

	/*
	if (dsp->m_axi->chk_dsp_running != DSP_RUNNING_CODE) {
		dsp->m_isRunning = 0;
		goto _ret; //DSP was not started (silent error)
	}
	*/
	//Let's stop the dsp anyway
	DSP_Stop_(dsp);
	dsp->m_isRunning = 0;

_ret: return;
}
//----------------------------------------------------------------------------
STA_API u32 STA_WaitDspReady(u32 dspMask, u32 timerMsec)
{
	s32 ready, i;
	s64 time0 = STA_GETTIME();

	do {
		ready = 1;
		FORMASKEDi(3, dspMask) {
			ready &= DSP_isLoaded(&g_dsp[i]);
			ready &= (DSP_READ(&g_dsp[i].m_axi->isInitsDone) > 0) ? 1:0;
		}
		STA_SLEEP(1);
	} while ((STA_GETTIME() - time0) < (s64)timerMsec && !ready);

	STA_TRACE("%s(0x%x, %d) = %d\n", __FUNCTION__, dspMask, timerMsec, ready);
	return (u32)ready;
}

//----------------------------------------------------------------------------
//Start the DSPs and the DMABus
STA_API void STA_Play(void)
{
	STA_ErrorCode err;
	int i;

	STA_TRACE("%s()\n", __FUNCTION__);

	if (g_drv.m_isPlaying) {
		goto _ret;} //already playing (silent error)

	//Start only DSPs which have connections and a valid flow
	for ( i = 0; i < 3; i++ ) {
		if (g_dsp[i].m_connections.m_size > 0) {
			if (!g_dsp[i].m_isFlowBuilt) {err = STA_INVALID_FLOW; goto _error;}
			err = _STA_StartDSP((STA_Dsp)i);		//(performs more checks)
			if (err) {goto _error;}
		}
	}

	//Also start the DMABus
#if STA_WITH_DMABUS
	STA_DMAStart();
#endif

	g_drv.m_isPlaying = 1;
	goto _ret;

_error:
	SetError(err);
	STA_Stop();
_ret:
	return;
}
//----------------------------------------------------------------------------
STA_API void STA_Stop(void)
{
    u32 i;
	STA_TRACE("%s()\n", __FUNCTION__);

	FORi(3) {
		if (DSP_isLoaded(&g_dsp[i])) {
			DSP_Stop_(&g_dsp[i]);
		}
	}

#if STA_WITH_DMABUS
	STA_DMAStop();
#endif

	g_drv.m_isPlaying = 0;
}

//===========================================================================
// DSP STATS
//===========================================================================

STA_API u32 STA_GetMaxDspCycleCost( STA_Dsp core )
{
	u32 ret = 0;
	tDSP* dsp = &g_dsp[core];
	u32 i;

	CHECK_PARAM((u32)core <= (u32)STA_DSP2);

	//get the max of all update slots
	ret = dsp->m_updateCycleCost[0];
	FORi(dsp->m_numOfUpdateSlots) {
		ret = max(ret, dsp->m_updateCycleCost[i]);
	}
	ret += dsp->m_mainCycleCost;

_ret:
	STA_TRACE("%s(DSP%d) = %d\n", __FUNCTION__, core, ret);
	return ret;
}
//----------------------------------------------------------------------------
STA_API void STA_GetDspMemCost(STA_Dsp core, u32 *xmemWords, u32* ymemWords)
{
	STA_ErrorCode err = STA_NO_ERROR;
	tDSP* dsp = &g_dsp[core];

	CHECK_PARAM((u32)core <= (u32)STA_DSP2);

	err = DSP_UpdateEstimatedSizes(dsp);
	if (err != STA_NO_ERROR) {SetError(err); goto _ret;}

	//return the x|ymem costs
	if (xmemWords) {*xmemWords = dsp->m_xmemPoolEstimatedSize;}
	if (ymemWords) {*ymemWords = dsp->m_ymemPoolEstimatedSize;}

_ret:
	STA_TRACE("%s(DSP%d) = xmem:%d, ymem:%d\n", __FUNCTION__, core, dsp->m_xmemPoolEstimatedSize, dsp->m_ymemPoolEstimatedSize);
	return;
}
//----------------------------------------------------------------------------
STA_API void STA_GetDspMaxMem(STA_Dsp core, u32 *xmemMaxWords, u32* ymemMaxWords)
{
	STA_ErrorCode err;
	tDSP* dsp = &g_dsp[core];

	CHECK_PARAM((u32)core <= (u32)STA_DSP2);

	//DSP clk must be on to access its mem
	err = (STA_ErrorCode)((u32)STA_DSP0_NOT_STARTED+(u32)core);
	if (DSP_CLOCK_IS_ENABLED(dsp->m_core))
	{
		//DSP must be loaded to have a valid 'xmem_pool_size' and 'ymem_pool_size'
		err = (STA_ErrorCode)((u32)STA_DSP0_XMEM_NOT_LOADED+(u32)core);
		if (DSP_isLoaded(dsp))
		{
			err = STA_NO_ERROR;
		}
	}

	if (err != STA_NO_ERROR) {SetError(err); goto _ret;}

	//return the x|ymem costs
	if (xmemMaxWords) {*xmemMaxWords = DSP_READ(&dsp->m_axi->xmem_pool_size);}
	if (ymemMaxWords) {*ymemMaxWords = DSP_READ(&dsp->m_ayi->ymem_pool_size);}

_ret:
	STA_TRACE("%s(DSP%d)\n", __FUNCTION__, core);
	return;
}

//===========================================================================
// MODULES
//===========================================================================

static tModule* findModuleByID(u32 id)
{
	tModule* mod = 0;
	u32 i;

	FORi(3) {
		if (g_dsp[i].m_modules.m_size > 0) {
			mod = DSP_FindModuleByID(&g_dsp[i], id);
			if (mod) {goto _ret;}
		}
	}

_ret: return mod;
}
//----------------------------------------------------------------------------
static tModule* findModuleByName(const char* name)
{
	tModule* mod = 0;
	u32 i;

	FORi(3) {
		if (g_dsp[i].m_modules.m_size > 0) {
			mod = DSP_FindModuleByName(&g_dsp[i], name);
			if (mod) {goto _ret;}
		}
	}

_ret: return mod;
}
//----------------------------------------------------------------------------
tModule* getModule(STAModule stamod)
{
	tModule* mod = (tModule*)stamod;

	if (stamod < 200) {
		mod = findModuleByID(stamod);
		if (mod) {goto _ret;}
	}

	if (!mod || mod->m_refCode == STA_MODULE_REF_CODE) {goto _ret;}

	//TODO: in order to be sure to avoid a crash because of 'mod->m_refCode' in case of name, shall decide to move
	//		findModuleByName() before, adding a lot of "find" over-head even when passing directy stamod as the mod pointer...?
	mod = findModuleByName((const char*)stamod);

_ret: return mod;
}
//----------------------------------------------------------------------------
static tModule* getModuleOrIO(STAModule stamod)
{
	tModule* ret;

	switch (stamod) {
	case STA_XIN0:  ret = &g_dsp[0].m_xin; break;
	case STA_XIN1:  ret = &g_dsp[1].m_xin; break;
	case STA_XIN2:  ret = &g_dsp[2].m_xin; break;
	case STA_XOUT0: ret = &g_dsp[0].m_xout; break;
	case STA_XOUT1: ret = &g_dsp[1].m_xout; break;
	case STA_XOUT2: ret = &g_dsp[2].m_xout; break;
	default:		ret = getModule(stamod);
	}
	return ret;
}
//----------------------------------------------------------------------------
static u32 generateModuleId(void)
{
	u32 id = g_drv.m_lastModuleId + 1;

	//find next available id
	while (findModuleByID(id)) {
		id++;
	}

	//check that the generated id is valid
	if (id < (u32)STA_XIN0) {
		g_drv.m_lastModuleId = id;
	} else {
		id = 0; //error: could not generate an ID....
	}

	return id;
}
//----------------------------------------------------------------------------
STA_API STAModule STA_AddModule( STA_Dsp dsp, STA_ModuleType type )
{
	tModule* ret = 0;
	u32 id = 0;

	CHECK_PARAM((u32)dsp <= (u32)STA_DSP2);
	CHECK_TYPE((u32)type < (u32)STA_MOD_LAST);
	CHECK_STOPPED();
	//note: we can't check STA_USER_MAX....

	//if XIN/XOUT module, just return its handle
	if (type == STA_XIN)  {ret = &g_dsp[dsp].m_xin;  goto _ret;}
	if (type == STA_XOUT) {ret = &g_dsp[dsp].m_xout; goto _ret;}

	//generate a new module ID
	id = generateModuleId();
	if (id == 0) {SetError(STA_FAILED_TO_GENERATE_ID); goto _ret;}

	//add a new module in this dsp list
	ret = DSP_AddModule2(&g_dsp[dsp], type, NULL, id, NULL, 0);
	if (!ret) {SetError(STA_OUT_OF_MEMORY);}

_ret:
	if (type < STA_MOD_LAST) {
		STA_TRACE("%s(DSP%d, %s) = 0x%p:%d\n", __FUNCTION__, dsp, g_STA_ModuleTypeStr[type], ret, id);
	} else {
		STA_TRACE("%s(DSP%d, type:0x%x) = 0x%p (id:%d)\n", __FUNCTION__, dsp, type, ret, id);
	}
	return (STAModule)ret;
}
//----------------------------------------------------------------------------
STA_API STAModule STA_AddModule2(STA_Dsp dsp, STA_ModuleType type, u32 id, u32 tags, const char* name)
{
	tModule* ret = 0;

	CHECK_PARAM((u32)dsp <= (u32)STA_DSP2);
	CHECK_TYPE((u32)type < (u32)STA_MOD_LAST);
	CHECK_STOPPED();

	//check that the ID does not already exist
	if (findModuleByID(id)) {SetError(STA_ID_ALREADY_EXIST); goto _ret;}

	//if XIN/XOUT module, just return its handle
	if (type == STA_XIN)  {ret = &g_dsp[dsp].m_xin;  goto _ret;}
	if (type == STA_XOUT) {ret = &g_dsp[dsp].m_xout; goto _ret;}

	//add a new module in this dsp list
	ret = DSP_AddModule2(&g_dsp[dsp], type, NULL, id, name, tags);
	if (!ret) {SetError(STA_OUT_OF_MEMORY);}

_ret:
	if (type < STA_MOD_LAST) {
		STA_TRACE("%s(DSP%d, %s, id:%d, tags:0x%x, name:%s) = 0x%p\n", __FUNCTION__, dsp, g_STA_ModuleTypeStr[type], id, tags, name, ret);
	} else {
		STA_TRACE("%s(DSP%d, type:0x%x, id:%d, tags:0x%x, name:%s) = 0x%p\n", __FUNCTION__, dsp, type, id, tags, name, ret);
	}
return (STAModule)ret;
}
//----------------------------------------------------------------------------
STA_API void STA_AddModulev( STA_Dsp dsp, const STA_ModuleType* types, u32 num, STAModule* modules )
{
	STA_ErrorCode err = STA_NO_ERROR;
	tDSP*     _dsp = &g_dsp[(u32)dsp];
	tListElmt* oldLast; //use this to clean in case of out of memory
	tModule*   mod;
	u32 i;

	STA_TRACE("%s(DSP%d,...)\n", __FUNCTION__, dsp);

	if (!types || num == 0)	{goto _ret;} //silent error

	CHECK_PARAM((u32)dsp <= (u32)STA_DSP2);
	FORi(num) {CHECK_TYPE((u32)types[i] < (u32)STA_MOD_LAST);}
	CHECK_STOPPED();

	oldLast = _dsp->m_modules.m_last; //to use in case of error

	for ( i = 0; i < num; i++ )
	{
		mod = 0;
		if (types[i] == STA_XIN)		{mod = &g_dsp[dsp].m_xin;}
		else if (types[i] == STA_XOUT)	{mod = &g_dsp[dsp].m_xout;}
		else
		{
			//generate a new module ID
			u32 id = generateModuleId();
			if (id == 0) {err = STA_FAILED_TO_GENERATE_ID; goto _err;}

			mod = DSP_AddModule2(_dsp, types[i], NULL, id, NULL, 0);
		}
		if (!mod) {err = STA_OUT_OF_MEMORY; goto _err;}

		//return the handle
		if (modules) {
			modules[i] = (STAModule)mod;}
	}

_err:
	if (err != STA_NO_ERROR) {
		//delete all the modules that just have been added
		tListElmt* new = (oldLast) ? oldLast->m_next : _dsp->m_modules.m_first;
		if (new) {
			LIST_DelElmts(&_dsp->m_modules, new, NULL);}
		SetError(err);
	}

_ret: return;
}
//----------------------------------------------------------------------------
STA_API STAModule STA_AddUserModule( STA_Dsp dsp, STA_ModuleType type, const STA_UserModuleInfo *info )
{
	tModule* ret = 0;
	u32 id = 0;

	CHECK_PARAM((u32)dsp <= (u32)STA_DSP2);
	CHECK_TYPE((u32)type >= (u32)STA_USER_0);
	CHECK_STOPPED();
	//note: we can't check STA_USER_MAX....

	if (!info) {goto _ret;}
	//TODO: check info's fields

	//generate a new module ID
	id = generateModuleId();
	if (id == 0) {SetError(STA_FAILED_TO_GENERATE_ID); goto _ret;}

	//add a new module in this dsp list
	ret = DSP_AddModule2(&g_dsp[dsp], type, info, id, NULL, 0);
	if (!ret) {SetError(STA_OUT_OF_MEMORY);}

_ret:
	STA_TRACE("%s(DSP%d, type:0x%x, [moduleInfo]) = 0x%p (id:%d)\n", __FUNCTION__, dsp, type, ret, id);
	return (STAModule)ret;
}
//----------------------------------------------------------------------------
STA_API STAModule STA_AddUserModule2( STA_Dsp dsp, STA_ModuleType type, const STA_UserModuleInfo *info,
									  u32 id, u32 tags, const char* name)
{
	tModule* ret = 0;

	CHECK_PARAM((u32)dsp <= (u32)STA_DSP2);
	CHECK_TYPE((u32)type >= (u32)STA_USER_0);
	CHECK_STOPPED();

	if (!info) {goto _ret;}
	//TODO: check info's fields

	//check that the ID does not already exist
	if (findModuleByID(id)) {SetError(STA_ID_ALREADY_EXIST); goto _ret;}

	//add a new module in this dsp list
	ret = DSP_AddModule2(&g_dsp[dsp], type, info, id, name, tags);
	if (!ret) {SetError(STA_OUT_OF_MEMORY);}

_ret:
	STA_TRACE("%s(DSP%d, type:0x%x, id:%d, tags:0x%x, name:%s) = 0x%p\n", __FUNCTION__, dsp, type, id, tags, name, ret);
	return (STAModule)ret;
}
//----------------------------------------------------------------------------
STA_API void STA_DeleteModule( STAModule* module )
{
	tModule* mod = 0;
	bool is_a_pointer_to_a_stamodule = FALSE;

	if (!module) {
		SetError(STA_INVALID_MODULE); goto _ret;
	}

	//first check if  'module' is a STAModule, ID or name,
	//else, check if '*module' is a STAModule, ID or name
	mod = getModule((STAModule)module);
	if (!IS_STAMODULE(mod))
	{
		if (IS_STAMODULE(*module)) {
			mod = (tModule*)*module;
			is_a_pointer_to_a_stamodule = TRUE;
		} else {
			mod = CHECK_MODULE(*module);
		}
	}

	//don't delete XIN/XOUT
	if (mod->m_type == STA_XIN || mod->m_type == STA_XOUT) {
		goto _ret; //not to be deleted
	}

	CHECK_STOPPED();

	DSP_DelModule(mod->m_dsp, mod);
	if (is_a_pointer_to_a_stamodule) {*module = 0;}

	//TODO(low) STA_DeleteModule: remove all the connections from/to this module

_ret:
	STA_TRACE("%s(%s)\n", __FUNCTION__, MODNAME(mod));
	return;
}
//===========================================================================
// CONNECTIONS
//===========================================================================
STA_API void STA_Connect( STAModule from, u32 chout, STAModule to, u32 chin )
{
	tModule 	*_from = getModuleOrIO(from);
	tModule 	*_to   = getModuleOrIO(to);
	tConnection *con;

	CHECK_MODULES(IS_STAMODULE(_from) && IS_STAMODULE(_to));
	CHECK_CHANNEL(chout < _from->m_nout && chin < _to->m_nin);
	CHECK_SAME_DSP(_from->m_dsp == _to->m_dsp);
	CHECK_STOPPED();

	//check if alread exist
	con = DSP_GetConnection(_from->m_dsp, _from, chout, _to, chin);
	if (con) {SetError(STA_WARN_CONNECTION_DO_EXIST); goto _ret;}

	con = DSP_AddConnection(_from->m_dsp, _from, chout, _to, chin);
	if (!con) {SetError(STA_OUT_OF_MEMORY);}

_ret:
	STA_TRACE("%s(%s, ch%d, %s, ch%d)\n", __FUNCTION__, MODNAME(_from), chout, MODNAME(_to), chin);
	return;
}
//----------------------------------------------------------------------------
STA_API void STA_Disconnect( STAModule from, u32 chout, STAModule to, u32 chin )
{
	tModule 	*_from = getModuleOrIO(from);
	tModule 	*_to   = getModuleOrIO(to);
	tConnection *con;

	CHECK_MODULES(IS_STAMODULE(_from) && IS_STAMODULE(_to));
	CHECK_SAME_DSP(_from->m_dsp == _to->m_dsp);
	CHECK_STOPPED();

	//check if exist
	con = DSP_GetConnection(_from->m_dsp, _from, chout, _to, chin);

	if (con) {
		DSP_DelConnection(_from->m_dsp, con);
	} else {
		SetError(STA_WARN_CONNECTION_NOT_EXIST);
	}

_ret:
	STA_TRACE("%s(%s, ch%d, %s, ch%d)\n", __FUNCTION__, MODNAME(_from), chout, MODNAME(_to), chin);
	return;
}
//----------------------------------------------------------------------------
STA_API void STA_ReconnectFrom( STAModule oldFrom, u32 oldChout, STAModule to, u32 chin, STAModule newFrom, u32 newChout )
{
	tModule 	*oldfrom = getModuleOrIO(oldFrom);
	tModule 	*newfrom = getModuleOrIO(newFrom);
	tModule 	*_to     = getModuleOrIO(to);
	tConnection *con;

	CHECK_MODULES(IS_STAMODULE(oldfrom) && IS_STAMODULE(newfrom) && IS_STAMODULE(_to));
	CHECK_CHANNEL(newChout < newfrom->m_nout && chin < _to->m_nin);
	CHECK_SAME_DSP(newfrom->m_dsp == _to->m_dsp);

	//get the connection
	con = DSP_GetConnection(oldfrom->m_dsp, oldfrom, oldChout, _to, chin);
	if (!con) {SetError(STA_WARN_CONNECTION_NOT_EXIST); goto _ret;}

	//reconnect
	DSP_ReconnectFrom(con, newfrom, newChout);

_ret:
	STA_TRACE("%s(%s, ch%d, %s, ch%d, newfrom:%s, ch%d)\n", __FUNCTION__,
		MODNAME(oldfrom), oldChout, MODNAME(_to), chin, MODNAME(newfrom), newChout);
	return;
}
//----------------------------------------------------------------------------
STA_API void STA_ReconnectTo( STAModule from, u32 chout, STAModule oldTo, u32 oldChin, STAModule newTo, u32 newChin )
{
	tModule 	*_from = getModuleOrIO(from);
	tModule 	*oldto = getModuleOrIO(oldTo);
	tModule 	*newto = getModuleOrIO(newTo);
	tConnection *con;

	CHECK_MODULES(IS_STAMODULE(_from) && IS_STAMODULE(oldto) && IS_STAMODULE(newto));
	CHECK_CHANNEL(chout < _from->m_nout && newChin < newto->m_nin);
	CHECK_SAME_DSP(_from->m_dsp == newto->m_dsp);

	//get the connection
	con = DSP_GetConnection(_from->m_dsp, _from, chout, oldto, oldChin);
	if (!con) {SetError(STA_WARN_CONNECTION_NOT_EXIST); goto _ret;}

	//reconnect
	DSP_ReconnectTo(con, newto, newChin);

_ret:
	STA_TRACE("%s(%s, ch%d, %s, ch%d, newto:%s, ch%d)\n", __FUNCTION__,
		MODNAME(_from), chout, MODNAME(oldto), oldChin, MODNAME(newto), newChin);
	return;
}
//----------------------------------------------------------------------------
static void BackupLastConnections(tListElmt* oldLast[3])
{
	sta_assert(oldLast);
	if (!oldLast) {goto _ret;}

	oldLast[0] = g_dsp[0].m_connections.m_last;
	oldLast[1] = g_dsp[1].m_connections.m_last;
	oldLast[2] = g_dsp[2].m_connections.m_last;

_ret: return;
}

static void DeleteNewConnections(const tListElmt* oldLast[3])
{
	int i;

	sta_assert(oldLast);
	if (!oldLast) {goto _ret;}

	//delete all the connections that just have been added
	for ( i = 0; i < 3; i++ ) {
		tListElmt* new = (oldLast[i]) ? oldLast[i]->m_next : g_dsp[i].m_connections.m_first;
		if (new) {
			LIST_DelElmts(&g_dsp[i].m_connections, new, NULL);}
	}

_ret: return;
}

STA_API void STA_Connectv( const STAConnector* ctors, u32 num, STAConnection* connections )
{
	u32 i;
	//use this to clean in case of out of memory
	tListElmt* oldLast[3]; BackupLastConnections(oldLast);

	STA_TRACE("%s()\n", __FUNCTION__);

	if (!ctors || num == 0) {
		goto _ret;} //silent error

	CHECK_STOPPED();

	for ( i = 0; i < num; i++ )
	{
		tModule 	*from = getModuleOrIO(ctors[i].from);
		tModule 	*to   = getModuleOrIO(ctors[i].to);
		tConnection *con;

		CHECK_MODULES(IS_STAMODULE(from) && IS_STAMODULE(to));
		CHECK_CHANNEL(ctors[i].chout < from->m_nout && ctors[i].chin < to->m_nin);
		CHECK_SAME_DSP(from->m_dsp == to->m_dsp);

		//TODO(low) STA_Connectv: check that the modules exist in the dsp list

		//add new connection
		con = DSP_AddConnection(from->m_dsp, from, ctors[i].chout, to, ctors[i].chin);
		if (!con) {goto _error;} //out of memory

		//return handle
		if (connections) {
			connections[i] = (STAConnection) con; }
	}
	goto _ret;

_error:
	//delete all the connections that just have been added
	DeleteNewConnections((const tListElmt**)oldLast);
	SetError(STA_OUT_OF_MEMORY);

_ret: return;
}
//----------------------------------------------------------------------------
//'ctors' gives the indices used to find the corresponding module handle from 'mods'
STA_API void STA_ConnectFromIndices( const STAConnector* ctors, u32 num, const STAModule* mods, STAConnection* cons )
{
	u32 i;
	//use this to clean in case of out of memory
	tListElmt* oldLast[3]; BackupLastConnections(oldLast);

	if (!ctors || num == 0 || !mods) {
		goto _ret;} //silent error

	CHECK_STOPPED();

	for ( i = 0; i < num; i++ )
	{
		tModule *from = (tModule*) mods[(u32)ctors[i].from];
		tModule *to   = (tModule*) mods[(u32)ctors[i].to];
		tConnection *con;

		CHECK_MODULES(IS_STAMODULE(from) && IS_STAMODULE(to));
		CHECK_CHANNEL(ctors[i].chout < from->m_nout && ctors[i].chin < to->m_nin);
		CHECK_SAME_DSP(from->m_dsp == to->m_dsp);

		//add new connection
		con = DSP_AddConnection(from->m_dsp, from, ctors[i].chout, to, ctors[i].chin);
		if (!con) {goto _error;} //out of memory

		//return handle
		if (cons) {
			cons[i] = (STAConnection) con;}
	}
	goto _ret;

_error:
	//delete all the connections that just have been added
	DeleteNewConnections((const tListElmt**)oldLast);
	SetError(STA_OUT_OF_MEMORY);

_ret: return;
}

//===========================================================================
// BUILD FLOW
//===========================================================================
STA_API void STA_BuildFlow(void)
{
	STA_ErrorCode err;
	int i;

	STA_TRACE("%s()\n", __FUNCTION__);

	CHECK_STOPPED();

	//build the flow only on the DSPs which have connections
	for ( i = 0; i < 3; i++ ) {
		if ( g_dsp[i].m_connections.m_size > 0 ) {
			err = DSP_FillTables( &g_dsp[i] );	//<-- performs more checks
			if (err) {goto _error;}
		}
	}

	goto _ret;

_error:
	//cancel all
	g_dsp[0].m_isFlowBuilt = 0;
	g_dsp[1].m_isFlowBuilt = 0;
	g_dsp[2].m_isFlowBuilt = 0;
	SetError(err);

_ret: return;
}

//===========================================================================
// DMABUS
//===========================================================================
#if !STA_WITH_DMABUS
STA_API int  STA_DMAAddTransfer( u16 srcAddr, u16 dstAddr, u32 numCh ){return 0;}
STA_API int  STA_DMAAddTransfer2( u16 srcAddr, u16 dstAddr, u32 numCh, u32 type){return 0;}
STA_API void STA_DMAAddTransferv( const u16* srcDstNch, u32 size ){}
STA_API void STA_DMASetTransfer(u16 srcAddr, u16 dstAddr, u32 numCh, u32 dmaPos, u32 type){}
STA_API void STA_DMAStart(void){}
STA_API void STA_DMAStop(void){}
STA_API void STA_DMAReset(void){}
STA_API void STA_DMASetFSyncCK1(u32 ck1){}
STA_API void STA_DMASetFSyncCK2(u32 ck2){}
#else

//NEW from CUT2
STA_API void STA_DMASetFSyncCK1(u32 ck1sel)
{
	//TODO: add check...
	AUDDMA->DMA_CR.BIT.FSYNC_CK1_SEL = ck1sel;
}

STA_API void STA_DMASetFSyncCK2(u32 ck2sel)
{
	//TODO: add check...
	AUDDMA->DMA_CR.BIT.FSYNC_CK2_SEL = ck2sel;
}


//returns the first dma position or -1 if failed
STA_API int STA_DMAAddTransfer( u16 srcAddr, u16 dstAddr, u32 numCh )
{
	s32 ret = -1;
	STA_ErrorCode err;
	u32 dmaPos = g_dma.m_numt; //record the first position

//	CHECK_DMA_STOPPED();

	if (g_dma.m_pTx != DMA_TX_BASE) { //mainly to be sure that its size and content are correct.
		SetError(STA_NOT_INITIALIZED); goto _ret;}

	if (DMA_isRUNNING) {
		SetError(STA_DMA_STILL_RUNNING); goto _ret;}

	if (g_dma.m_numt + numCh > STA_DMA_SIZE) {
		SetError(STA_DMA_OUT_OF_SLOT); goto _ret;}

	err = DMAB_CheckAddress(srcAddr, dstAddr, numCh);
	if (err) {SetError(err); goto _ret;}

	g_dma.m_numt = DMAB_SetTransfers(&g_dma, srcAddr, dstAddr, numCh, g_dma.m_numt, STA_DMA_FSYNC); //FSYNC 48Khz

	ret = (s32)dmaPos;

_ret: return ret;
}
//----------------------------------------------------------------------------
//NEW from CUT2
//returns the first dma position or -1 if failed
STA_API int STA_DMAAddTransfer2( u16 srcAddr, u16 dstAddr, u32 numCh, u32 type)
{
	s32 ret = -1;
	STA_ErrorCode err;
	u32 dmaPos = g_dma.m_numt; //record the first position

//	CHECK_DMA_STOPPED();

	if (g_dma.m_pTx != DMA_TX_BASE) { //mainly to be sure that its size and content are correct.
		SetError(STA_NOT_INITIALIZED); goto _ret;}

	if (DMA_isRUNNING) {
		SetError(STA_DMA_STILL_RUNNING); goto _ret;}

	if (g_dma.m_numt + numCh > STA_DMA_SIZE) {
		SetError(STA_DMA_OUT_OF_SLOT); goto _ret;}

	err = DMAB_CheckAddress(srcAddr, dstAddr, numCh);
	if (err) {SetError(err); goto _ret;}

	//NEW from CUT2
	err = DMAB_CheckType(type);
	if (err) {SetError(err); goto _ret;}


	g_dma.m_numt = DMAB_SetTransfers(&g_dma, srcAddr, dstAddr, numCh, g_dma.m_numt, type);

	ret = (s32)dmaPos;

_ret: return ret;
}
//----------------------------------------------------------------------------
STA_API void STA_DMAAddTransferv( const u16* srcDstNch, u32 size )
{
	STA_ErrorCode err;
	const u16* p = srcDstNch;
	u32 i;

	CHECK_INITIALIZED(g_dma.m_pTx == DMA_TX_BASE); //mainly to be sure that its size and content are correct.
	CHECK_DMA_STOPPED();
	if (!p || size == 0) {goto _ret;} //nothing to do

	for (i = 0; i < size; i++)
	{
		u16 src = p[0];
		u16 dst = p[1];
		u16 nch = p[2];
		p += 3;

		if (g_dma.m_numt + nch > STA_DMA_SIZE) {
			SetError(STA_DMA_OUT_OF_SLOT); goto _ret; }

		err = DMAB_CheckAddress(src, dst, nch);
		if (err) {SetError(err); goto _ret;}

		g_dma.m_numt = DMAB_SetTransfers(&g_dma, src, dst, nch, g_dma.m_numt, STA_DMA_FSYNC); //FSYNC 48Khz
	}

_ret: return;
}
//----------------------------------------------------------------------------
STA_API void STA_DMASetTransfer(u16 srcAddr, u16 dstAddr, u32 numCh, u32 dmaPos, u32 type)
{
	STA_ErrorCode err;

	CHECK_INITIALIZED(g_dma.m_pTx == DMA_TX_BASE); //mainly to be sure that its size and content are correct.
//	CHECK_DMA_STOPPED();

	if (dmaPos + numCh > STA_DMA_SIZE) {
		SetError(STA_DMA_OUT_OF_SLOT); goto _ret; }

	err = DMAB_CheckAddress(srcAddr, dstAddr, numCh);
	if (err) {SetError(err); goto _ret;}

	//NEW from CUT2
	err = DMAB_CheckType(type);
	if (err) {SetError(err); goto _ret;}

	dmaPos = DMAB_SetTransfers(&g_dma, srcAddr, dstAddr, numCh, dmaPos, type);
	(void) dmaPos;//(note: dummy assignment for misra)

_ret: return;
}
//----------------------------------------------------------------------------
STA_API void STA_DMAStart(void)
{
	STA_TRACE("%s()\n", __FUNCTION__);

	CHECK_INITIALIZED(g_dma.m_pTx == DMA_TX_BASE); //mainly to be sure that its size and content are correct.
	DMA_RUN(1);

_ret: return;
}
//---------------------------------------------------------------------------
STA_API void STA_DMAStop(void)
{
	STA_TRACE("%s()\n", __FUNCTION__);

	DMA_RUN(0);
}
//---------------------------------------------------------------------------
STA_API void STA_DMAReset(void)
{
	STA_TRACE("%s()\n", __FUNCTION__);

	DMAB_Reset(&g_dma);
}
#endif
//===========================================================================
// MODULE COMMON
//===========================================================================
STA_API void STA_SetMode( STAModule module, u32 mode )
{
	STA_ErrorCode err;
	tModule* mod = CHECK_MODULE(module);

	err = MOD_SetMode(mod, mode);
	if (err) {SetError(err);}

_ret:
	STA_TRACE("%s(%s, %d)\n", __FUNCTION__, MODNAME(mod), mode);
	return;
}
//---------------------------------------------------------------------------
STA_API u32 STA_GetMode(STAModule module)
{
	u32 ret = 0;
	tModule* mod = CHECK_MODULE(module);

	ret = mod->m_mode;

_ret:
	STA_TRACE("%s(%s) = %d\n", __FUNCTION__, MODNAME(mod), ret);
	return ret;
}
//---------------------------------------------------------------------------

//For GAINS, MIXER (output channels), EQ
STA_API STAModule STA_SetNumChannels( STAModule module, u32 nch )
{
	STA_ErrorCode err = STA_NO_ERROR;
	tModule* mod = CHECK_MODULE(module);

	CHECK_VALUE(nch >= 1);
	CHECK_BEFORE_BUILT(mod->m_dspCoefs); //must be called before BuildFlow()

	if (mod->m_type == STA_PEAKDETECT_NCH) {
		if (nch == mod->m_nin) {goto _ret;} //nothing to do
	} else {
		if (nch == mod->m_nout) {goto _ret;} //nothing to do
	}

	//EQ
	if (STA_EQUA_STATIC_1BAND_DP <= mod->m_type && mod->m_type <= STA_EQUA_STATIC_16BANDS_SP)
	{
		mod->m_nout = mod->m_nin = nch;

		EQUA_AdjustDspMemSize(mod);

		//Note: don't need to re-allocated the EQ module (same params are applied to all channels)
	}
	//Others
	else {switch (mod->m_type)
	{
	case STA_GAIN_STATIC_NCH:
		GAIN_SetNch(mod, nch);
		break;

	case STA_BITSHIFTER_NCH:
		BITSHIFT_SetNch(mod, nch);
		break;

	case STA_PEAKDETECT_NCH:
		PEAKDETECT_SetNch(mod, nch);
		break;

	case STA_GAIN_SMOOTH_NCH:
	case STA_GAIN_LINEAR_NCH:
		mod = GAINS_SetNumChannels(mod, nch); //reallocates host mem
		if (!mod) {err = STA_OUT_OF_MEMORY;}
		break;

	case STA_CLIPLMTR_NCH:
		mod = CLIP_SetNumChannels(mod, nch); //reallocates host mem
		if (!mod) {err = STA_OUT_OF_MEMORY;}
		break;

	case STA_MIXE_2INS_NCH:
	case STA_MIXE_3INS_NCH:
	case STA_MIXE_4INS_NCH:
	case STA_MIXE_5INS_NCH:
	case STA_MIXE_6INS_NCH:
	case STA_MIXE_7INS_NCH:
	case STA_MIXE_8INS_NCH:
	case STA_MIXE_9INS_NCH:
		if (nch > STA_MAX_MIXER_CH) {err = STA_INVALID_VALUE; break;}
		mod->m_nout = nch;
		mod->m_nin  = (nch > 1) ? 2 * mod->m_ninpc : mod->m_ninpc; //always max 2 insets of ninpc
		break;

	default:
		err = STA_INVALID_MODULE_TYPE; break;
	}}

	if (err != STA_NO_ERROR) {
		SetError(err); goto _ret;
	}

	if (mod) {
		//update module and dsp cycles
		MOD_UpdateModuleAndDspCycles(mod);

		mod->m_dsp->m_dirtyFlow = 1; //need to re-buildFlow anyway...
	}

_ret:
	STA_TRACE("%s(%s, %d) = 0x%p\n", __FUNCTION__, MODNAME(mod), nch, mod);
	return (STAModule) mod;
}
//---------------------------------------------------------------------------
//Must be called before STA_BuildFlow
STA_API void STA_SetNumOfUpdateSlots( STA_Dsp dsp, u32 numOfUpdateSlots)
{
	STA_TRACE("%s(DSP%d, %d)\n", __FUNCTION__, dsp, numOfUpdateSlots);

	CHECK_PARAM(dsp <= STA_DSP2);
	CHECK_VALUE(numOfUpdateSlots <= STA_MAX_UPDATE_SLOTS);
//	CHECK_BEFORE_BUILT(); //must be called before BuildFlow()

	g_dsp[dsp].m_numOfUpdateSlots = numOfUpdateSlots;
	g_dsp[dsp].m_dirtyFlow = 1;

_ret: return;
}
//---------------------------------------------------------------------------
STA_API void STA_SetUpdateSlot(STAModule module, u32 slot)
{
	tModule* mod = CHECK_MODULE(module);

	CHECK_VALUE(slot < mod->m_dsp->m_numOfUpdateSlots);

    //remove module cost from old slot
	mod->m_dsp->m_updateCycleCost[mod->m_updateSlot] -= mod->m_updateCycleCost;

    //update slot
	mod->m_updateSlot = slot;

	//add module cost to new slot
	mod->m_dsp->m_updateCycleCost[mod->m_updateSlot] += mod->m_updateCycleCost;

	mod->m_dsp->m_dirtyFlow = 1;

_ret:
	STA_TRACE("%s(%s, %d)\n", __FUNCTION__, MODNAME(mod), slot);
	return;
}

//===========================================================================
// BIQUAD FILTER
//===========================================================================
//MUST be called before the BuildFlow
STA_API STAModule STA_SetFilterNumBands(STAModule module, u32 numBands)
{
	tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(mod->m_type == STA_SPECMTR_NBANDS_1BIQ);
	CHECK_VALUE(numBands > 0);
	CHECK_BEFORE_BUILT(mod->m_dspCoefs); //must be called before BuildFlow()

	if (mod->m_type == STA_SPECMTR_NBANDS_1BIQ)
	{
		mod = SPECMTR_SetNumBands(mod, numBands);

		//update module and dsp cycles
		MOD_UpdateModuleAndDspCycles(mod);
	}
	//TODO?: also for the EQ

	if (!mod) {SetError(STA_OUT_OF_MEMORY);}

_ret:
	STA_TRACE("%s(%s, %d) = 0x%p\n", __FUNCTION__, MODNAME(mod), numBands, mod);
	return (STAModule) mod;
}
//---------------------------------------------------------------------------
STA_API void STA_SetFilterParam(STAModule module, u32 band, STA_FilterParam param, s32 val )
{
	STA_ErrorCode err = STA_NO_ERROR;
	tBiquad* biq;
	tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(mod->m_pFilters); //must have filters
	CHECK_PARAM(band < mod->m_nfilters);

	biq = &mod->m_pFilters[band];
	switch (param) {
	case STA_GAIN:
		if (val < -24*BGS || val > 24*BGS) {err = STA_INVALID_VALUE; break;}
		BIQ_SetGain(biq, val);
		break;
	case STA_QUAL:
		if (val < 1 /*|| val > 100*/) {err = STA_INVALID_VALUE; break;}
		BIQ_SetQual(biq, val);
		break;
	case STA_FREQ:
		if (val < 10 || val > 20000) {err = STA_INVALID_VALUE; break;}
		BIQ_SetFreq(biq, val, STA_SAMPLING_FREQ);
		break;
	case STA_TYPE:
		if ((u32)val >= (u32)STA_BIQUAD_NUM) {err = STA_INVALID_VALUE; break;}
		BIQ_SetFilterType(biq, (STA_FilterType)val);
		break;
	default: err = STA_INVALID_PARAM; break;
	}

	if (err != STA_NO_ERROR) {
		SetError(err); goto _ret;
	}

	mod->m_dirtyDspCoefs |= 1u << band;

	//update dsp now (or later in BuildFlow)
	UPDATE_DSPCOEFS(mod);

_ret:
	if (param == STA_TYPE){
		STA_FilterType type = (STA_FilterType) val;
		STA_TRACE("%s(%s, band:%d, %s, %s)\n", __FUNCTION__, MODNAME(mod), band, g_STA_FilterParamStr[STA_TYPE], (type<STA_BIQUAD_NUM)?g_STA_FilterTypeStr[type]:"?");
	} else {
		STA_TRACE("%s(%s, band:%d, %s, %d)\n", __FUNCTION__, MODNAME(mod), band, (param<=STA_TYPE)?g_STA_FilterParamStr[param]:"?", val);
	}
	return;
}
//---------------------------------------------------------------------------
STA_API void STA_SetFilterParams(STAModule module, STA_FilterParam param, const s16* values )
{
	u32 i;
	tModule* mod = CHECK_MODULE(module);

	if (!values) {goto _ret;} //silently
	//other checks are done in STA_SetFilterParam()

	FORi(mod->m_nfilters) {
		STA_SetFilterParam(module, i, param, values[i]);
	}

_ret:
	STA_TRACE("%s(%s, %s, [values])\n", __FUNCTION__, MODNAME(mod), g_STA_FilterParamStr[param]);
	return;
}
//---------------------------------------------------------------------------
STA_API void STA_SetFilter(STAModule module, u32 band, s32 G, s32 Q, s32 F)
{
	tBiquad* biq;
	tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(mod->m_pFilters); //must have filters
	CHECK_PARAM(band < mod->m_nfilters);

	CHECK_VALUE( -24*BGS <= G && G <= 24*BGS
			 &&        1 <= Q /*&& Q <= 100*/
			 &&       20 <= F && F <= 20000);

	biq = &mod->m_pFilters[band];

	BIQ_SetFreq(biq, F, STA_SAMPLING_FREQ);
	BIQ_SetQual(biq, Q);
	BIQ_SetGain(biq, G);

	mod->m_dirtyDspCoefs |= 1u << band;

	//update dsp now (or later in BuildFlow)
	UPDATE_DSPCOEFS(mod);

_ret:
	STA_TRACE("%s(%s, band:%d, G:%d, Q:%d, F:%d)\n", __FUNCTION__, MODNAME(mod), band, G, Q, F);
	return;
}
//---------------------------------------------------------------------------
STA_API void STA_GetFilter(STAModule module, u32 band, s32* G, s32* Q, s32* F)
{
	const tBiquad* biq;
	s32 _G = 0, _Q = 0, _F = 0;
	const tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(mod->m_pFilters); //must have filters
	CHECK_PARAM(band < mod->m_nfilters);

	biq = &mod->m_pFilters[band];

	_G = BIQ_GetGain(biq);
	_Q = BIQ_GetQual(biq);
	_F = BIQ_GetFreq(biq);

	if (G) {*G = _G;}
	if (Q) {*Q = _Q;}
	if (F) {*F = _F;}

_ret:
	STA_TRACE("%s(%s, band:%d, G:%d, Q:%d, F:%d)\n", __FUNCTION__, MODNAME(mod), band, _G, _Q, _F);
	return;
}
//---------------------------------------------------------------------------
STA_API void STA_SetFilterv(STAModule module, u32 band, const s16 GQF[3])
{
	if (GQF) {
		STA_SetFilter(module, band, GQF[0], GQF[1], GQF[2]);
	}
}
//---------------------------------------------------------------------------
STA_API void STA_SetFilterType(STAModule module, u32 band, STA_FilterType type)
{
	tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(mod->m_pFilters); //must have filters
	CHECK_PARAM(band < mod->m_nfilters);
	CHECK_PARAM(type < STA_BIQUAD_NUM);

	BIQ_SetFilterType(&mod->m_pFilters[band], type);

	mod->m_dirtyDspCoefs |= 1u << band;

	//update dsp now (or later in BuildFlow)
	UPDATE_DSPCOEFS(mod);

_ret:
	STA_TRACE("%s(%s, band:%d, %s)\n", __FUNCTION__, MODNAME(mod), band, (type<STA_BIQUAD_NUM)?g_STA_FilterTypeStr[type]:"?");
	return;
}
//---------------------------------------------------------------------------
STA_API STA_FilterType STA_GetFilterType(STAModule module, u32 band)
{
	STA_FilterType ret = (STA_FilterType)0;
	const tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(mod->m_pFilters); //must have filters
	CHECK_PARAM(band < mod->m_nfilters);

	ret = mod->m_pFilters[band].m_type;

_ret:
	STA_TRACE("%s(%s, band:%d) = %s\n", __FUNCTION__, MODNAME(mod), band, g_STA_FilterTypeStr[ret]);
	return ret;
}
//---------------------------------------------------------------------------
//sets new gains, AND UPDATES the DSP coefs
//Gains: array must contains all the Gains, including for the bands not to be updated.
STA_API void STA_SetFilterGains(STAModule module, u32 bandmask, const s16* Gains)
{
	u32 i;
	tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(mod->m_pFilters); //must have filters
	if (!Gains) {goto _ret;} //silently

	FORMASKEDi(mod->m_nfilters, bandmask)
	{
		CHECK_VALUE(-24*BGS <= Gains[i] && Gains[i] <= 24*BGS);

		BIQ_SetGain(&mod->m_pFilters[i], Gains[i]);

		mod->m_dirtyDspCoefs |= 1u << i;	//(flag also set when re-computing the biquad coefs)
	}

	UPDATE_DSPCOEFS(mod);

_ret:
	STA_TRACE("%s(%s, bandmask:0x%x, [Gains])\n", __FUNCTION__, MODNAME(mod), bandmask);
	return;
}
//---------------------------------------------------------------------------
//coefs[] = {(b0/2)>>bshift, (b1/2)>>bshift, (b2/2)>>bshift, a1/2, a2/2, bshift}, with a0 = 1.0 (implicitly)
STA_API void STA_SetFilterHalvedCoefs(STAModule module, u32 band, const q23* coefs )
{
	tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(mod->m_pFilters); //must have filters
	CHECK_PARAM(band < mod->m_nfilters);
	if (!coefs) {goto _ret;} //silently
	CHECK_VALUE(IS_Q23(coefs[0]) && IS_Q23(coefs[1]) && IS_Q23(coefs[2]) && IS_Q23(coefs[3]) && IS_Q23(coefs[4]));
	CHECK_VALUE(coefs[5] >= 0);

	BIQ_SetHalvedCoefs_q23((tBiquad*)&mod->m_pFilters[band], coefs);

	mod->m_dirtyDspCoefs |= 1u << band;

	UPDATE_DSPCOEFS(mod);

_ret:
	STA_TRACE("%s(%s, band:%d, [coefs])\n", __FUNCTION__, MODNAME(mod), band);
	return;
}
//---------------------------------------------------------------------------
STA_API void STA_SetFilterHalvedCoefsAll(STAModule module, u32 bandmask, const q23* allCoefs )
{
	u32 i;
	tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(mod->m_pFilters); //must have filters
	if (allCoefs == NULL) {goto _ret;}

	FORMASKEDi(mod->m_nfilters, bandmask)
	{
		const q23* coefi = &allCoefs[i*6];
		CHECK_VALUE(IS_Q23(coefi[0]) && IS_Q23(coefi[1]) && IS_Q23(coefi[2]) && IS_Q23(coefi[3]) && IS_Q23(coefi[4]));
		CHECK_VALUE(coefi[5] >= 0);

		BIQ_SetHalvedCoefs_q23((tBiquad*)&mod->m_pFilters[i], coefi);

		mod->m_dirtyDspCoefs |= 1u << i;
	}

	UPDATE_DSPCOEFS(mod);

_ret:
	STA_TRACE("%s(%s, bandmask:0x%x, [coefs])\n", __FUNCTION__, MODNAME(mod), bandmask);
	return;
}
//---------------------------------------------------------------------------
//coefs[] = {(b0/2)>>bshift, (b1/2)>>bshift, (b2/2)>>bshift, a1/2, a2/2, bshift}, with a0 = 1.0 (implicitly)
STA_API void STA_GetFilterHalvedCoefs(STAModule module, u32 band, q23* coefs )
{
	tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(mod->m_pFilters); //must have filters
	CHECK_PARAM(band < mod->m_nfilters);
	if (coefs == NULL) {goto _ret;}

	//before to get, update the biquad coefs if dirty
	if (mod->m_pFilters[band].m_dirtyBiquad) {
		BIQ_UpdateCoefs(&mod->m_pFilters[band]);
		mod->m_dirtyDspCoefs |= 1u << band;
	}

	BIQ_GetHalvedCoefs_q23((tBiquad*)&mod->m_pFilters[band], coefs);

_ret:
	STA_TRACE("%s(%s, band:%d, [coefs])\n", __FUNCTION__, MODNAME(mod), band);
	return;
}
//---------------------------------------------------------------------------
STA_API void STA_GetFilterHalvedCoefsAll(STAModule module, u32 bandmask, q23* allCoefs )
{
	u32 i;
	tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(mod->m_pFilters); //must have filters
	if (allCoefs == NULL) {goto _ret;}

	FORMASKEDi(mod->m_nfilters, bandmask)
	{
		//before to get, update the biquad coefs if dirty
		if (mod->m_pFilters[i].m_dirtyBiquad) {
			BIQ_UpdateCoefs(&mod->m_pFilters[i]);
			mod->m_dirtyDspCoefs |= 1u << i;
		}
		BIQ_GetHalvedCoefs_q23((tBiquad*)&mod->m_pFilters[i], &allCoefs[i*6]);
	}

_ret:
	STA_TRACE("%s(%s, bandmask:0x%x, [coefs])\n", __FUNCTION__, MODNAME(mod), bandmask);
	return;
}
//---------------------------------------------------------------------------
STA_API s32 STA_GetFilterParam(STAModule module, u32 band, STA_FilterParam param)
{
	s32 val = 0;
	u32 numBiqPerBand;
	tBiquad* biq;
	tModule* mod = CHECK_MODULE(module);

	numBiqPerBand = (mod->m_type == STA_SPECMTR_NBANDS_1BIQ) ? ((tSpectrumMeter*)mod->m_sub)->m_numBiqPerBand : 1;

	CHECK_TYPE(mod->m_pFilters); //must have filters
	CHECK_PARAM(band * numBiqPerBand < mod->m_nfilters);

	//read from the first biquad of the band
	biq = &mod->m_pFilters[band * numBiqPerBand];

	switch (param) {
	case STA_GAIN: val = BIQ_GetGain(biq); break;
	case STA_QUAL: val = BIQ_GetQual(biq); break;
	case STA_FREQ: val = BIQ_GetFreq(biq); break;
	case STA_TYPE: val = (s32)biq->m_type; break;
	default: SetError(STA_INVALID_PARAM); break;
	}

_ret:
	if (param == STA_TYPE){
		STA_TRACE("%s(%s, %d, %s) = %s\n", __FUNCTION__, MODNAME(mod), band, g_STA_FilterParamStr[param], ((u32)val<(u32)STA_BIQUAD_NUM)?g_STA_FilterTypeStr[val]:"?");
	} else {
		STA_TRACE("%s(%s, %d, %s) = %d\n", __FUNCTION__, MODNAME(mod), band, g_STA_FilterParamStr[param], val);
	}
	return val;
}
//---------------------------------------------------------------------------
STA_API void STA_GetFilterParams(STAModule module, STA_FilterParam param, s16* values)
{
	u32 numBiqPerBand, numBands, i;
	tBiquad* biq;
	tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(mod->m_pFilters); //must have filters
	if (!values) {goto _ret;} //silently

	//read from the first biquad of each band
	if (mod->m_type == STA_SPECMTR_NBANDS_1BIQ) {
		numBands      = ((tSpectrumMeter*)mod->m_sub)->m_numBands;
		numBiqPerBand = ((tSpectrumMeter*)mod->m_sub)->m_numBiqPerBand;
	} else {
		numBands      = mod->m_nfilters;
		numBiqPerBand = 1;
	}
	biq = &mod->m_pFilters[0];

	switch (param) {
	case STA_GAIN: FORi(numBands) {values[i] = (s16)BIQ_GetGain(biq); biq += numBiqPerBand;} break;
	case STA_QUAL: FORi(numBands) {values[i] = (s16)BIQ_GetQual(biq); biq += numBiqPerBand;} break;
	case STA_FREQ: FORi(numBands) {values[i] = (s16)BIQ_GetFreq(biq); biq += numBiqPerBand;} break;
	case STA_TYPE: FORi(numBands) {values[i] = (s16)biq->m_type;      biq += numBiqPerBand;} break;
	default: SetError(STA_INVALID_PARAM); break;
	}

_ret:
	STA_TRACE("%s(%s, %s, [values])\n", __FUNCTION__, MODNAME(mod), g_STA_FilterParamStr[param]);
	return;
}
//===========================================================================
// MUX / SELECTOR
//===========================================================================
STA_API void STA_MuxSet(STAModule module, const u8* sels)
{
	u32 i;
	tMux*    mux;
	tModule* mod = CHECK_MODULE(module);
	mux = (tMux*) mod->m_sub;

	CHECK_TYPE(STA_MUX_2OUT <= mod->m_type && mod->m_type <= STA_MUX_10OUT);

	if (sels ==  NULL) {goto _ret;}

	FORi(mod->m_nout) {
		CHECK_CHANNEL(sels[i] < mod->m_nin);
		mux->m_sels[i] = sels[i];
	}

	//update DSP
	if (mod->m_dspCoefs) {
		vu32* dsp_sels = (vu32*)((volatile T_MuxParam*)mod->m_dspCoefs)->sel;
		FORi(mod->m_nout) {
			DSP_WRITE(&dsp_sels[i], mux->m_sels[i]);
		}
	}

_ret:
	STA_TRACE("%s(%s, [sels])\n", __FUNCTION__, MODNAME(mod));
	return;
}
//---------------------------------------------------------------------------
STA_API void STA_MuxGet(STAModule module, u8* sels)
{
	u32 i;
	tMux* mux;
	tModule* mod = CHECK_MODULE(module);
	mux = (tMux*) mod->m_sub;

	CHECK_TYPE(STA_MUX_2OUT <= mod->m_type && mod->m_type <= STA_MUX_10OUT);

	if (sels ==  NULL) {goto _ret;}

	FORi(mod->m_nout) {
		sels[i] = mux->m_sels[i];
	}

_ret:
	STA_TRACE("%s(%s, [sels])\n", __FUNCTION__, MODNAME(mod));
	return;
}
//---------------------------------------------------------------------------
STA_API void STA_MuxSetOutChannel(STAModule module, u32 inch, u32 outch)
{
	tMux*    mux;
	tModule* mod = CHECK_MODULE(module);
	mux = (tMux*) mod->m_sub;

	CHECK_TYPE(STA_MUX_2OUT <= mod->m_type && mod->m_type <= STA_MUX_10OUT);
	CHECK_CHANNEL(outch < mod->m_nout && inch < mod->m_nin);

	mux->m_sels[outch] = (u8)inch;

	//update DSP
	if (mod->m_dspCoefs) {
		vu32* dsp_sels = (vu32*)((volatile T_MuxParam*)mod->m_dspCoefs)->sel;
		DSP_WRITE(&dsp_sels[outch], inch);
	}

_ret:
	STA_TRACE("%s(%s, in:%d, out:%d)\n", __FUNCTION__, MODNAME(mod), inch, outch);
	return;
}


//===========================================================================
// GAIN (STATIC, EXP and LINEAR)
//===========================================================================
STA_API void STA_GainSetGain( STAModule module, u32 ch, s32 Gain )
{
	tModule* mod = CHECK_MODULE(module);

	CHECK_CHANNEL(!mod->m_dspCoefs || ch < mod->m_nin);
//	CHECK_VALUE(-1200 <= Gain && Gain <= 240);
	CHECK_TYPE(mod->m_type == STA_GAIN_STATIC_NCH || mod->m_type == STA_GAIN_SMOOTH_NCH || mod->m_type == STA_GAIN_LINEAR_NCH);

	//STATIC GAIN
	if (mod->m_type == STA_GAIN_STATIC_NCH)
	{
		//if called before buildFlow (dspCoefs=NULL), set initGain (and ch is not relevant)
		GAIN_SetPositiveGain(mod, ch, Gain);
	}

	//SMOOTH GAIN
	else //if (mod->m_type == STA_GAIN_SMOOTH_NCH || mod->m_type == STA_GAIN_LINEAR_NCH)
	{
		tSmoothGain* gain = (tSmoothGain*)mod->m_sub + ch;

		CHECK_VALUE(Gain <= (s32)gain->m_maxGain * 10);

		if (gain->m_goalDb != Gain) {
			gain->m_goalDb = Gain;
			gain->m_goalLn = _STA_db2lin(Gain, 10, &gain->m_lshift, mod->m_mode|DB_CLAMP);

			mod->m_dirtyDspCoefs |= 1u << ch;

			//update dsp now (or later in BuildFlow)
			if (mod->m_dspCoefs) {
				GAINS_UpdateDspCoefs(mod, ch); }
		}
	}

_ret:
	STA_TRACE("%s(%s, ch%d, %d)\n", __FUNCTION__, MODNAME(mod), ch, Gain);
	return;
}
//---------------------------------------------------------------------------
STA_API s32 STA_GainGetGain(STAModule module, u32 ch)
{
	s32 ret = 0;
	tModule* mod = CHECK_MODULE(module);

	CHECK_CHANNEL(!mod->m_dspCoefs || ch < mod->m_nin);
	CHECK_TYPE(mod->m_type == STA_GAIN_STATIC_NCH || mod->m_type == STA_GAIN_SMOOTH_NCH || mod->m_type == STA_GAIN_LINEAR_NCH);


	//STATIC GAIN
	if (mod->m_type == STA_GAIN_STATIC_NCH)
	{
		//if called before buildFlow (dspCoefs=NULL), get initGain (and ch is not relevant)
		ret = GAIN_GetPositiveGain(mod, ch);
	}

	//SMOOTH GAIN
	else //if (mod->m_type == STA_GAIN_SMOOTH_NCH || mod->m_type == STA_GAIN_LINEAR_NCH)
	{
		ret = ((const tSmoothGain*)mod->m_sub)[ch].m_goalDb;
	}

_ret:
	STA_TRACE("%s(%s, ch%d) = %d\n", __FUNCTION__, MODNAME(mod), ch, ret);
	return ret;
}
//---------------------------------------------------------------------------
STA_API void STA_GainSetGains( STAModule module, u32 chMask, s32 Gain )
{
	tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE((mod->m_type == STA_GAIN_STATIC_NCH)||(mod->m_type == STA_GAIN_SMOOTH_NCH)||(mod->m_type == STA_GAIN_LINEAR_NCH));
	CHECK_CHANNEL(mod->m_nin <= 32);
//	CHECK_VALUE(-1200 <= Gain && Gain <= 480); //note: max +48 dB because of fixed shift of 8 bits

	//STATIC GAIN
	if (mod->m_type == STA_GAIN_STATIC_NCH)
	{
		//must be called after BuildFLow() AND dsp initialisation
		CHECK_DSP_INITIALIZED(mod->m_dsp);

		//	if (mod->m_dspCoefs && mod->m_dsp->m_axi->isInitsDone)
		if (mod->m_dspCoefs) {
			GAIN_SetPositiveGains(mod, chMask, Gain); }
	}

	//SMOOTH GAIN
	else //if (mod->m_type == STA_GAIN_SMOOTH_NCH || mod->m_type == STA_GAIN_LINEAR_NCH)
	{
		tSmoothGain* gains = (tSmoothGain*)mod->m_sub;
		u32 i;

		static s32 s_gainDb = 0;
		static s32 s_gainLn = ONE;
		static s32 s_lshift = 0;

		FORMASKEDi(mod->m_nin, chMask)
		{
			CHECK_VALUE(Gain <= (s32)gains[i].m_maxGain * 10);

			if (s_gainDb != Gain || s_lshift != gains[i].m_lshift) {
				s_gainDb = Gain;
				s_gainLn = _STA_db2lin(Gain, 10, &gains[i].m_lshift, mod->m_mode|DB_CLAMP);
				s_lshift = gains[i].m_lshift;
			}

			gains[i].m_goalDb = Gain;		//gain dB
			gains[i].m_goalLn = s_gainLn;	//gain linear frac
			gains[i].m_lshift = s_lshift;

			mod->m_dirtyDspCoefs |= 1u << i;

			//update dsp
			if (mod->m_dspCoefs) {
				GAINS_UpdateDspCoefs(mod, i); }
		}
	}

_ret:
	STA_TRACE("%s(%s, chmask:0x%x, %d)\n", __FUNCTION__, MODNAME(mod), chMask, Gain);
	return;
}
//---------------------------------------------------------------------------
//maxGain in dB
STA_API void STA_GainSetMaxGain( STAModule module, u32 ch, u32 maxGain )
{
	u32 shift;
	tSmoothGain* gain;
	tModule* mod = CHECK_MODULE(module);

	//note: don't allow setting the shift for static gain, as currently static gain is using auto shift (DB_AUTOSHIFT)
//	CHECK_TYPE((mod->m_type == STA_GAIN_STATIC_NCH)||(mod->m_type == STA_GAIN_SMOOTH_NCH)||(mod->m_type == STA_GAIN_LINEAR_NCH));
	CHECK_TYPE(mod->m_type == STA_GAIN_SMOOTH_NCH || mod->m_type == STA_GAIN_LINEAR_NCH);
	CHECK_CHANNEL(ch < mod->m_nin);
	CHECK_VALUE(maxGain <= 96); //note: max +96 dB => shift = 16


	//find the bitshift such that maxGain = 1 << shift
	shift  = maxGain / 6;
	if (maxGain % 6) {
		shift++;
	}

	gain = &((tSmoothGain*)mod->m_sub)[ch];

	gain->m_maxGain = maxGain;
	gain->m_lshift  = (s32)shift;
	gain->m_goalLn  = _STA_db2lin(gain->m_goalDb, 10, &gain->m_lshift, mod->m_mode|DB_CLAMP);

_ret:
	STA_TRACE("%s(%s, ch%d, %d)\n", __FUNCTION__, MODNAME(mod), ch, maxGain);
	return;
}
//---------------------------------------------------------------------------
STA_API void STA_GainSetMaxGains( STAModule module, u32 chMask, u32 maxGain )
{
	u32 shift, i;
	tSmoothGain* gains;
	tModule* mod = CHECK_MODULE(module);

//	CHECK_TYPE((mod->m_type == STA_GAIN_STATIC_NCH)||(mod->m_type == STA_GAIN_SMOOTH_NCH)||(mod->m_type == STA_GAIN_LINEAR_NCH));
	CHECK_TYPE(mod->m_type == STA_GAIN_SMOOTH_NCH || mod->m_type == STA_GAIN_LINEAR_NCH);
	CHECK_CHANNEL(mod->m_nin <= 32);
	CHECK_VALUE(maxGain <= 96); //note: max +96 dB => shift = 16

	//note: don't allow setting the shift for static gain, as currently static gain is using auto shift (DB_AUTOSHIFT)

	//find the bitshift such that maxGain = 1 << shift
	shift  = maxGain / 6;
	if (maxGain % 6) {
		shift++;
	}

	gains = (tSmoothGain*)mod->m_sub;

	FORMASKEDi(mod->m_nin, chMask) {
		gains[i].m_maxGain = maxGain;
		gains[i].m_lshift  = (s32)shift;
		gains[i].m_goalLn  = _STA_db2lin(gains[i].m_goalDb, 10, &gains[i].m_lshift, mod->m_mode|DB_CLAMP);
	}

_ret:
	STA_TRACE("%s(%s, chmask:0x%x, %d)\n", __FUNCTION__, MODNAME(mod), chMask, maxGain);
	return;
}
//---------------------------------------------------------------------------
STA_API u32 STA_GainGetMaxGain( STAModule module, u32 ch)
{
	u32 ret = 0;
	const tModule* mod = CHECK_MODULE(module);

//	CHECK_TYPE(mod->m_type == STA_GAIN_STATIC_NCH || mod->m_type == STA_GAIN_SMOOTH_NCH || mod->m_type == STA_GAIN_LINEAR_NCH);
	CHECK_TYPE(mod->m_type == STA_GAIN_SMOOTH_NCH || mod->m_type == STA_GAIN_LINEAR_NCH);
	CHECK_CHANNEL(ch < mod->m_nin);

	ret = ((const tSmoothGain*)mod->m_sub)[ch].m_maxGain;

_ret:
	STA_TRACE("%s(%s, ch%d) = %d\n", __FUNCTION__, MODNAME(mod), ch, ret);
	return ret;
}
//---------------------------------------------------------------------------
STA_API void STA_GainSetPolarity( STAModule module, u32 ch, s32 polarity )
{
	tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(mod->m_type == STA_GAIN_STATIC_NCH || mod->m_type == STA_GAIN_SMOOTH_NCH || mod->m_type == STA_GAIN_LINEAR_NCH);
//	CHECK_TYPE(mod->m_type == STA_GAIN_SMOOTH_NCH || mod->m_type == STA_GAIN_LINEAR_NCH);
	CHECK_CHANNEL(ch < mod->m_nin);

	if (mod->m_type == STA_GAIN_STATIC_NCH)
	{
		//TODO
	}
	else
	{
		tSmoothGain* gain = &((tSmoothGain*)mod->m_sub)[ch];

		gain->m_polarity = (polarity > 0) ? +1 : -1;

		mod->m_dirtyDspCoefs |= 1u << ch;

		if (mod->m_dspCoefs) {
			GAINS_UpdateDspCoefs(mod, ch); }
	}

_ret:
	STA_TRACE("%s(%s, ch%d, %d)\n", __FUNCTION__, MODNAME(mod), ch, polarity);
	return;
}
//---------------------------------------------------------------------------
STA_API s32 STA_GainGetPolarity( STAModule module, u32 ch)
{
	s32 ret = 0;
	const tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(mod->m_type == STA_GAIN_STATIC_NCH || mod->m_type == STA_GAIN_SMOOTH_NCH || mod->m_type == STA_GAIN_LINEAR_NCH);
//	CHECK_TYPE(mod->m_type == STA_GAIN_SMOOTH_NCH || mod->m_type == STA_GAIN_LINEAR_NCH);
	CHECK_CHANNEL(ch < mod->m_nin);

	if (mod->m_type == STA_GAIN_STATIC_NCH)
	{
		//TODO
	}
	else
	{
		ret = ((const tSmoothGain*)mod->m_sub)[ch].m_polarity;
	}

_ret:
	STA_TRACE("%s(%s, ch%d) = %d\n", __FUNCTION__, MODNAME(mod), ch, ret);
	return ret;
}
//---------------------------------------------------------------------------
STA_API void STA_GainSetLinearGainsAndShifts( STAModule module, u32 chMask, q23 linGain , s32 leftShift)
{
	tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(mod->m_type == STA_GAIN_STATIC_NCH);
	CHECK_CHANNEL(mod->m_nin <= 32);
	CHECK_VALUE(IS_Q23(linGain));
	CHECK_VALUE(-31 <= leftShift && leftShift <= 31);

	//must be called after BuildFLow() AND dsp initialisation
	CHECK_DSP_INITIALIZED(mod->m_dsp);

	//	if (mod->m_dspCoefs && mod->m_dsp->m_axi->isInitsDone)
	if (mod->m_dspCoefs) {
		GAIN_SetLinearGainsAndShifts(mod, chMask, linGain, leftShift); }

_ret:
	STA_TRACE("%s(%s, chmask:0x%x, q23:0x%x, shift:%d)\n", __FUNCTION__, MODNAME(mod), chMask, linGain, leftShift);
	return;
}
//---------------------------------------------------------------------------
STA_API void STA_SmoothGainSetTC( STAModule module, u32 ch, u32 msUp, u32 msDown )
{
	tSmoothGain* gains;
	tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(mod->m_type == STA_GAIN_SMOOTH_NCH || mod->m_type == STA_GAIN_LINEAR_NCH);
	CHECK_CHANNEL(ch < mod->m_nin);
	CHECK_VALUE( msUp <= 10000 && msDown <= 10000);

	gains = (tSmoothGain*)mod->m_sub;

	if (mod->m_type == STA_GAIN_SMOOTH_NCH)
	{
		GAINS_SetRampExponential(&gains[ch], msUp, msDown);
	}
	else
	{
		GAINS_SetRampLinear(&gains[ch], msUp, msDown);
	}

_ret:
	STA_TRACE("%s(%s, ch%d, up:%d, down:%d)\n", __FUNCTION__, MODNAME(mod), ch, msUp, msDown);
	return;
}
//---------------------------------------------------------------------------
STA_API void STA_SmoothGainSetRamp( STAModule module, u32 chMask, u32 msUp, u32 msDown )
{
	tSmoothGain* gains;
	u32 i;
	tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(mod->m_type == STA_GAIN_SMOOTH_NCH || mod->m_type == STA_GAIN_LINEAR_NCH);
	CHECK_CHANNEL(mod->m_nin <= 32);
	CHECK_VALUE(msUp <= 10000 && msDown <= 10000);

	gains = (tSmoothGain*)mod->m_sub;

	//update the masked channels
	if (mod->m_type == STA_GAIN_SMOOTH_NCH)
	{
		FORMASKEDi(mod->m_nin, chMask) {
			GAINS_SetRampExponential(&gains[i], msUp, msDown);
		}
	}
	else
	{
		FORMASKEDi(mod->m_nin, chMask) {
			GAINS_SetRampLinear(&gains[i], msUp, msDown);
		}
	}

_ret:
	STA_TRACE("%s(%s, chmask:0x%x, up:%d, down:%d)\n", __FUNCTION__, MODNAME(mod), chMask, msUp, msDown);
	return;
}
//---------------------------------------------------------------------------
STA_API void STA_SmoothGainGetRamp( STAModule module, u32 ch, u32* msUp, u32* msDown )
{
	tSmoothGain* gains;
	u32 _msUp = 0, _msDown = 0;
	const tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(mod->m_type == STA_GAIN_SMOOTH_NCH || mod->m_type == STA_GAIN_LINEAR_NCH);
	CHECK_CHANNEL(ch < mod->m_nin);

	gains   = (tSmoothGain*)mod->m_sub;

	_msUp   = gains[ch].m_tup;
	_msDown = gains[ch].m_tdown;

	if (msUp)   {*msUp   = _msUp;}
	if (msDown) {*msDown = _msDown;}

_ret:
	STA_TRACE("%s(%s, ch%d, up:%d, down:%d)\n", __FUNCTION__, MODNAME(mod), ch, _msUp, _msDown);
	return;
}
//===========================================================================
// PEAK DETECTOR
//===========================================================================
STA_API void STA_PeakDetectorResetLevel( STAModule module, u32 channel)
{
	tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(mod->m_type == STA_PEAKDETECT_NCH);
	CHECK_CHANNEL(channel <= mod->m_nin);

	if (mod->m_dspCoefs) {
		PEAKDETECT_Reset(mod, channel);
	}

_ret: return;
}
//---------------------------------------------------------------------------
STA_API s32 STA_PeakDetectorGetLevel( STAModule module, u32 channel, u32 flags)
{
	s32 ret = (flags & (u32)STA_PEAK_DB) ? -1400 : 0;
	tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(mod->m_type == STA_PEAKDETECT_NCH);
	CHECK_CHANNEL(channel <= mod->m_nin);

	if (mod->m_dspCoefs)
	{
		//get the peak
		ret = DSP_READ(&((tPeakDetect*)mod->m_sub)->m_peaks[channel]);

		//convert to (dB/10)
		if (flags & (u32)STA_PEAK_DB) {
			ret = _STA_lin2db(ret)/10;}

		//reset
		if (flags & (u32)STA_PEAK_RESET) {
			PEAKDETECT_Reset(mod, channel);}
	}

_ret: return ret;
}

//===========================================================================
// BITSHIFTER
//===========================================================================
STA_API void STA_BitShifterSetLeftShifts( STAModule module, u32 chmask, s32 leftShift)
{
	tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(mod->m_type == STA_BITSHIFTER_NCH);
	CHECK_CHANNEL(mod->m_nin <= 32);
	CHECK_VALUE(-35 <= leftShift && leftShift <= 27);

	//must be called after the BuildFLow()
	CHECK_BUILT(mod->m_dspCoefs);

	if (mod->m_dspCoefs) {
		BITSHIFT_SetLeftShifts(mod, chmask, leftShift);}

_ret:
	STA_TRACE("%s(%s, chmask:0x%x, shift:%d)\n", __FUNCTION__, MODNAME(mod), chmask, leftShift);
	return;
}
//===========================================================================
// LOUDNESS
//===========================================================================
STA_API void STA_LoudSetTypes( STAModule module, STA_LoudFilterType bassType, STA_LoudFilterType trebType )
{
	tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(mod->m_type == STA_LOUD_STATIC_DP || mod->m_type == STA_LOUD_STATIC_STEREO_DP);
	CHECK_PARAM(bassType <= STA_LOUD_BANDBOOST_2 &&	trebType <= STA_LOUD_BANDBOOST_2);

	LOUD_SetTypes(mod, bassType, trebType);

_ret:
	STA_TRACE("%s(%s, %d, %d)\n", __FUNCTION__, MODNAME(mod), bassType, trebType);
	return;
}
//---------------------------------------------------------------------------
STA_API void STA_LoudSetAutoBassQLevels( STAModule module, s32 Gstart, s32 Gstop, s32 Qstart, s32 Qstop )
{
	tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(mod->m_type == STA_LOUD_STATIC_DP || mod->m_type == STA_LOUD_STATIC_STEREO_DP);

	CHECK_VALUE(-24*BGS <= Gstart && Gstart <= 24*BGS
			 && -24*BGS <= Gstop  && Gstop  <= 24*BGS
			 &&       1 <= Qstart && Qstart <= 100
			 &&       1 <= Qstop  && Qstop  <= 100);

	LOUD_SetAutoBassQLevels(mod, Gstart, Gstop, Qstart, Qstop);

_ret: return;
}
//----------------------------------------------------------------------------
STA_API void STA_LoudSetAutoGLevels( STAModule module, STA_LoudFilter filter, s32 Vstart, s32 Vstop, s32 Gmin, s32 Gmax )
{
	tModule* mod = CHECK_MODULE(module);

	//only for single channel Loudness
	CHECK_TYPE(mod->m_type == STA_LOUD_STATIC_DP);
	CHECK_PARAM((u32)filter <= (u32)STA_LOUD_TREB);

	CHECK_VALUE(  -24*BGS <= Gmin   && Gmin   <= 24*BGS
			 &&   -24*BGS <= Gmax   && Gmax   <= 24*BGS
			 &&     -1200 <= Vstart && Vstart <= 0
			 &&     -1200 <= Vstop  && Vstop  <= 0);

	LOUD_SetAutoGLevels(mod, filter, Vstart, Vstop, Gmin, Gmax);

_ret: return;
}
//===========================================================================
// TONE
//===========================================================================
STA_API void STA_ToneSetTypes( STAModule module, STA_ToneFilterType bassType, STA_ToneFilterType trebType )
{
	tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(mod->m_type == STA_TONE_STATIC_DP || mod->m_type == STA_TONE_STATIC_STEREO_DP);
	CHECK_PARAM((u32)bassType <= (u32)STA_TONE_DUALMODE && (u32)trebType <= (u32)STA_TONE_BANDBOOST_2 );

	TONE_SetTypes(mod, bassType, trebType);

_ret:
	STA_TRACE("%s(%s, %d, %d)\n", __FUNCTION__, MODNAME(mod), bassType, trebType);
	return;
}

//---------------------------------------------------------------------------
STA_API void STA_ToneSetAutoBassQLevels( STAModule module, s32 Gstart, s32 Gstop, s32 Qstart, s32 Qstop )
{
	tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(mod->m_type == STA_TONE_STATIC_DP || mod->m_type == STA_TONE_STATIC_STEREO_DP);

	CHECK_VALUE(-24*BGS <= Gstart && Gstart <= 24*BGS
			 && -24*BGS <= Gstop  && Gstop  <= 24*BGS
			 &&       1 <= Qstart && Qstart <= 100
			 &&       1 <= Qstop  && Qstop  <= 100);

	TONE_SetAutoBassQLevels(mod, Gstart, Gstop, Qstart, Qstop);

_ret: return;
}
//===========================================================================
// EQUALIZER
//===========================================================================
//Num bands can't be changed runtime because needs to change the filterFunc
//Instead the user should choose a new equalizer type.
/*
STA_API void STA_EqSetNumBands( STAModule module, u32 numBands )
{
	tModule* mod = CHECK_MODULE(module);

	if (numBands > STA_MAX_EQ_NUM_BANDS) {
		SetError(STA_INVALID_VALUE); return; }

	EQUA_SetNumBands(mod, numBands);
}
*/

//Replaced with generic STA_SetFilterGains()
/*
//sets new gains, AND UPDATES the DSP coefs
//Gains: array must contains all the Gains, including for the bands not to be updated.
STA_API void STA_EQSetGains( STAModule module, u32 bandmask, const s16* Gains)
{
	int i;
	tModule* mod = CHECK_MODULE(module);

//	CHECK_TYPE(STA_EQUA_STATIC_1BAND_DP <= mod->m_type && mod->m_type <= STA_EQUA_STATIC_16BANDS_SP);
	CHECK_TYPE(mod->m_pFilters); //must have filters

	FORi(mod->m_nfilters) CHECK_VALUE(-24*BGS <= Gains[i] && Gains[i] <= 24*BGS);

	EQUA_SetGains(mod, bandmask, Gains);
}
*/

//===========================================================================
// MIXER 1 & 2
//===========================================================================
#if defined(MIXER_VER_1) || defined (MIXER_VER_2)
//----------------------------------------------------------------------------
//Bind to Balance, Fader or Loudness (not dual)
//note: chb is only used for Balance and Fader
STA_API void STA_MixerBindTo(STAModule module, u32 chm, STAModule bafalo, u32 chb)
{
	tModule *mixer = (tModule*)module;
	tModule	*baf   = (tModule*)bafalo;
	tMixer  *mix;
	tAttenuator* att;

	CHECK_MODULES( IS_STAMODULE(mixer) && IS_STAMODULE(baf) );
	CHECK_TYPE(STA_MIXE_2INS_NCH <= mixer->m_type && mixer->m_type <= STA_MIXE_9INS_NCH);
	CHECK_CHANNEL(chm < mixer->m_nout && chb < baf->m_nout);
	CHECK_TYPE((baf->m_type == STA_LOUD_STATIC_DP)||(baf->m_type == STA_MIXE_BALANCE)||(baf->m_type == STA_MIXE_FADER));

	mix = (tMixer*)mixer->m_sub + chm;
	att = (tAttenuator*)baf->m_sub + chb;

	switch (baf->m_type) {
	case STA_LOUD_STATIC_DP: 	//can't bind to loudness dual
		mix->m_pLoudness = baf;
		((tLoudness*)baf->m_sub)->m_pMixer = mix;
		break;
	case STA_MIXE_BALANCE:
		mix->m_pBalance = att;
		att->m_pMixer   = mix;
		break;
	case STA_MIXE_FADER:
		mix->m_pFader   = att;
		att->m_pMixer   = mix;
		break;
	default: break;
	}

_ret: return;
}
#endif //MIXER_VER_1 || MIXER_VER_2

//===========================================================================
// MIXER 3
//===========================================================================
#ifdef MIXER_VER_3

STA_API void STA_MixerSetRelativeInGain(STAModule module, u32 outCh, u32 inRel, s32 inGain)
{
	tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(STA_MIXE_2INS_NCH <= mod->m_type && mod->m_type <= STA_MIXE_9INS_NCH);
	CHECK_CHANNEL(outCh < mod->m_nout && inRel < mod->m_ninpc);
	CHECK_VALUE(-1200 <= inGain && inGain <= 0);

	MIXE_SetInGainLinear(mod, outCh, inRel, inGain);

_ret:
	STA_TRACE("%s(%s, outch:%d, inrel:%d, %d)\n", __FUNCTION__, MODNAME(mod), outCh, inRel, inGain);
	return;
}
//----------------------------------------------------------------------------
STA_API s32 STA_MixerGetRelativeInGain(STAModule module, u32 outCh, u32 inRel)
{
	s32 ret = 0;
	const tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(STA_MIXE_2INS_NCH <= mod->m_type && mod->m_type <= STA_MIXE_9INS_NCH);
	CHECK_CHANNEL(outCh < mod->m_nout && inRel < mod->m_ninpc);

	ret = ((tMixer3*)mod->m_sub)[outCh].m_inGains[inRel].m_goalDb;

_ret:
	STA_TRACE("%s(%s, outch:%d, inrel:%d) = %d\n", __FUNCTION__, MODNAME(mod), outCh, inRel, ret);
	return ret;
}
//----------------------------------------------------------------------------
STA_API void STA_MixerSetAbsoluteInGain(STAModule module, u32 inAbs, s32 inGain)
{
	u32 outCh, inRel;
	tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(STA_MIXE_2INS_NCH <= mod->m_type && mod->m_type <= STA_MIXE_9INS_NCH);
	CHECK_CHANNEL(inAbs < mod->m_nout * mod->m_ninpc);
	CHECK_VALUE(-1200 <= inGain && inGain <= 0);

	outCh = inAbs / mod->m_ninpc;
	inRel = inAbs % mod->m_ninpc;

	MIXE_SetInGainLinear(mod, outCh, inRel, inGain);

_ret:
	STA_TRACE("%s(%s, inAbs:%d, %d)\n", __FUNCTION__, MODNAME(mod), inAbs, inGain);
	return;
}
//----------------------------------------------------------------------------
STA_API s32 STA_MixerGetAbsoluteInGain(STAModule module, u32 inAbs)
{
	s32 ret = 0;
	u32 outCh, inRel;
	const tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(STA_MIXE_2INS_NCH <= mod->m_type && mod->m_type <= STA_MIXE_9INS_NCH);
	CHECK_CHANNEL(inAbs < mod->m_nout * mod->m_ninpc);

	outCh = inAbs / mod->m_ninpc;
	inRel = inAbs % mod->m_ninpc;

	ret = ((tMixer3*)mod->m_sub)[outCh].m_inGains[inRel].m_goalDb;

_ret:
	STA_TRACE("%s(%s, inAbs:%d) = %d\n", __FUNCTION__, MODNAME(mod), inAbs, ret);
	return ret;
}
//----------------------------------------------------------------------------
STA_API void STA_MixerSetInGains(STAModule module, u32 outMask, u32 inMask, s32 inGain)
{
	u32 ch, in;
	tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(STA_MIXE_2INS_NCH <= mod->m_type && mod->m_type <= STA_MIXE_9INS_NCH);
	CHECK_VALUE(-1200 <= inGain && inGain <= 0);

	for (ch = 0; ch < mod->m_nout; ch++) {if (outMask & ((u32)1<<ch)) {
		for (in = 0; in < mod->m_ninpc; in++) {if (inMask & ((u32)1<<in)) {
			MIXE_SetInGainLinear(mod, ch, in, inGain);
		}}
	}}

_ret:
	STA_TRACE("%s(%s, outmask:0x%x, inmask:0x%x, %d)\n", __FUNCTION__, MODNAME(mod), outMask, inMask, inGain);
	return;
}
//----------------------------------------------------------------------------
STA_API void STA_MixerSetChannelInGains(STAModule module, u32 outch, const s16* inGains)
{
	u32 in;
	tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(STA_MIXE_2INS_NCH <= mod->m_type && mod->m_type <= STA_MIXE_9INS_NCH);
	CHECK_CHANNEL(outch < mod->m_nout);
	if (!inGains) {goto _ret;} //silent error

	for (in = 0; in < mod->m_ninpc; in++)
	{
		CHECK_VALUE(-1200 <= inGains[in] && inGains[in] <= 0);

		MIXE_SetInGainLinear(mod, outch, in, inGains[in]);
	}

_ret:
	STA_TRACE("%s(%s, outch:%d, [inGains])\n", __FUNCTION__, MODNAME(mod), outch);
	return;
}
//----------------------------------------------------------------------------
STA_API void STA_MixerSetOutGain(STAModule module, u32 outch, s32 Gain)
{
	tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(STA_MIXE_2INS_NCH <= mod->m_type && mod->m_type <= STA_MIXE_9INS_NCH);
	CHECK_CHANNEL(outch < mod->m_nout);
	CHECK_VALUE(-1200 <= Gain && Gain <= 240);

	MIXE_SetOutGainExponential(mod, outch, Gain);

_ret:
	STA_TRACE("%s(%s, outch:%d, %d)\n", __FUNCTION__, MODNAME(mod), outch, Gain);
	return;
}
//----------------------------------------------------------------------------
STA_API s32 STA_MixerGetOutGain(STAModule module, u32 outch)
{
	s32 ret = 0;
	const tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(STA_MIXE_2INS_NCH <= mod->m_type && mod->m_type <= STA_MIXE_9INS_NCH);
	CHECK_VALUE(outch < mod->m_nout);

	ret = ((tMixer3*)mod->m_sub)[outch].m_outGain.m_goalDb;

_ret:
	STA_TRACE("%s(%s, outch:%d) = %d\n", __FUNCTION__, MODNAME(mod), outch, ret);
	return ret;
}
//----------------------------------------------------------------------------
STA_API void STA_MixerSetOutGains(STAModule module, u32 outMask, s32 Gain)
{
	u32 ch;
	tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(STA_MIXE_2INS_NCH <= mod->m_type && mod->m_type <= STA_MIXE_9INS_NCH);
	CHECK_VALUE(-1200 <= Gain && Gain <= 240);

	for (ch = 0; ch < mod->m_nout; ch++) {if (outMask & ((u32)1<<ch)) {
		MIXE_SetOutGainExponential(mod, ch, Gain);
	}}

_ret:
	STA_TRACE("%s(%s, outmask:0x%x, %d)\n", __FUNCTION__, MODNAME(mod), outMask, Gain);
	return;
}
//----------------------------------------------------------------------------
STA_API void STA_MixerSetInRamp(STAModule module, u32 inch, u32 msUp, u32 msDown )
{
	u32 outch, inrel;
	tMixer3* mix;
	tModule* mod = CHECK_MODULE(module);
	mix = mod->m_sub;

	CHECK_TYPE(STA_MIXE_2INS_NCH <= mod->m_type && mod->m_type <= STA_MIXE_9INS_NCH);
	CHECK_CHANNEL(inch < mod->m_nout * mod->m_ninpc);
//	CHECK_VALUE();

	outch = inch / mod->m_ninpc;
	inrel = inch % mod->m_ninpc;

	GAINS_SetRampLinear(&mix[outch].m_inGains[inrel], msUp, msDown);

_ret:
	STA_TRACE("%s(%s, inch:%d, up:%d, down:%d)\n", __FUNCTION__, MODNAME(mod), inch, msUp, msDown);
	return;
}
//----------------------------------------------------------------------------
STA_API void STA_MixerSetInRamps(STAModule module, u32 outMask, u32 inMask, u32 ts)
{
	u32 ch, in;
	tMixer3* mix;
	tModule* mod = CHECK_MODULE(module);
	mix = mod->m_sub;

	CHECK_TYPE(STA_MIXE_2INS_NCH <= mod->m_type && mod->m_type <= STA_MIXE_9INS_NCH);
	//CHECK_VALUE(ts >= 30);

	for (ch = 0; ch < mod->m_nout; ch++) {if (outMask & ((u32)1<<ch)) {
		for (in = 0; in < mod->m_ninpc; in++) {if (inMask & ((u32)1<<in)) {
			GAINS_SetRampLinear(&mix[ch].m_inGains[in], ts, ts);
		}}
	}}

_ret:
	STA_TRACE("%s(%s, outmask:0x%x, inmask0x%x, %d)\n", __FUNCTION__, MODNAME(mod), outMask, inMask, ts);
	return;
}
//----------------------------------------------------------------------------
STA_API void STA_MixerSetOutRamp(STAModule module, u32 outch, u32 msUp, u32 msDown)
{
	tMixer3* mix;
	tModule* mod = CHECK_MODULE(module);
	mix = mod->m_sub;

	CHECK_TYPE(STA_MIXE_2INS_NCH <= mod->m_type && mod->m_type <= STA_MIXE_9INS_NCH);
	CHECK_CHANNEL(outch < mod->m_nout);
	//CHECK_VALUE(ts >= 30);

	GAINS_SetRampExponential(&mix[outch].m_outGain, msUp, msDown);

_ret:
	STA_TRACE("%s(%s, outch:%d, up:%d, down:%d)\n", __FUNCTION__, MODNAME(mod), outch, msUp, msDown);
	return;
}
//----------------------------------------------------------------------------
STA_API void STA_MixerSetOutRamps(STAModule module, u32 outMask, u32 ts)
{
	u32 ch;
	tMixer3* mix;
	tModule* mod = CHECK_MODULE(module);
	mix = mod->m_sub;

	CHECK_TYPE(STA_MIXE_2INS_NCH <= mod->m_type && mod->m_type <= STA_MIXE_9INS_NCH);
	//CHECK_VALUE(ts >= 30);

	for (ch = 0; ch < mod->m_nout; ch++) {if (outMask & ((u32)1<<ch)) {
		GAINS_SetRampExponential(&mix[ch].m_outGain, ts, ts);
	}}

_ret:
	STA_TRACE("%s(%s, outmask:0x%x, %d)\n", __FUNCTION__, MODNAME(mod), outMask, ts);
	return;
}
//----------------------------------------------------------------------------
STA_API void STA_MixerSetVolume(STAModule module, u32 chMask, const s16 volume)
{
	tModule* mod = CHECK_MODULE(module);

	CHECK_TYPE(STA_MIXE_2INS_NCH <= mod->m_type && mod->m_type <= STA_MIXE_9INS_NCH);
	CHECK_VALUE(-1200 <= volume && volume <= 0);

	if (DSPCOEFS(mod)) {
		MIXE_SetVolume(mod, chMask, volume);}

_ret: return;
}
//----------------------------------------------------------------------------
//DEPRECATED
STA_API void STA_MixerSetMute(STAModule module, u32 chMask, bool mute, bool hard, s32 depth)
{
	(void)module; (void)chMask; (void)mute; (void)hard; (void)depth;
}
//----------------------------------------------------------------------------
//DEPRECATED
STA_API void STA_MixerSetMuteOrVolumeTS(STAModule module, u32 chMask, bool setvol, const u16 ts)
{
	(void)module; (void)chMask; (void)setvol; (void)ts;
}
//----------------------------------------------------------------------------
//Bind to Balance, Fader or Loudness (not dual)
//note: chb is only used for Balance and Fader
STA_API void STA_MixerBindTo(STAModule module, u32 chm, STAModule bafalo, u32 chb)
{
	(void)module; (void)chm; (void)bafalo; (void)chb;
	//TODO ?
}
#endif //MIXER_VER_3

//===========================================================================
// BALANCE / FADER
//===========================================================================
STA_API void STA_MixerSetBalance(STAModule mixerBalance, s32 balance)
{
	tAttenuator* m_ch;
	tModule*     const mod = CHECK_MODULE(mixerBalance);

	CHECK_TYPE(mod->m_type == STA_MIXE_BALANCE || mod->m_type == STA_MIXE_FADER);
	CHECK_VALUE(-1200 <= balance && balance <= 1200);

	m_ch = mod->m_sub;

	//if bound to mixer...
	//(note: middle channel is optional)
	if (m_ch[0].m_pMixer && m_ch[0].m_pMixer->m_pDspCoefs
	 && m_ch[2].m_pMixer && m_ch[2].m_pMixer->m_pDspCoefs)
	{
		MIXE_SetBalance(mod, balance);
	}

_ret: return;
}
//----------------------------------------------------------------------------
STA_API void STA_MixerSetBalanceLimit(STAModule mixerBalance, s32 limit)
{
	tAttenuator* m_ch;
	tModule*     const mod = CHECK_MODULE(mixerBalance);

	CHECK_TYPE(mod->m_type == STA_MIXE_BALANCE || mod->m_type == STA_MIXE_FADER);
	CHECK_VALUE(-1200 <= limit && limit <= 0);

	m_ch = mod->m_sub;
	m_ch[0].m_limit = m_ch[1].m_limit = (s16)limit;

_ret: return;
}
//----------------------------------------------------------------------------
STA_API void STA_MixerSetBalanceCenter(STAModule mixerBalance, s32 start, s32 stop, s32 maxi)
{
	tAttenuator* m_ch;
	tModule*     const mod = CHECK_MODULE(mixerBalance);

	CHECK_TYPE(mod->m_type == STA_MIXE_BALANCE || mod->m_type == STA_MIXE_FADER);
	CHECK_VALUE(-1200 <= start && start <= 0 &&
				-1200 <= stop  && stop  <= 0 &&
				-1200 <= maxi  && maxi  <= 0 );

	m_ch = mod->m_sub;
	_STA_SetLinLimiter(&m_ch[1].m_linLimiter, start, stop, 0, maxi);

_ret: return;
}
//===========================================================================
// COMPANDER
//===========================================================================
STA_API void STA_CompanderSetParams(STAModule module, u32 paramMask, const s16 *params)
{
	STA_ErrorCode err = STA_NO_ERROR;
	tModule* const mod = CHECK_MODULE(module);

	CHECK_TYPE(STA_COMP_1CH <= mod->m_type || mod->m_type <= STA_COMP_6CH);
	if (!params || paramMask == 0) {goto _ret;} //nothing to do

	if (DSPCOEFS(mod)) {
		volatile T_ComprBaseCoeffs*    const base= &((volatile T_CompanderCoeffs*)mod->m_dspCoefs)->base_coeffs;
		volatile T_ComprSectionCoeffs* const cpr = &((volatile T_CompanderCoeffs*)mod->m_dspCoefs)->compression_coeffs;
		volatile T_ComprSectionCoeffs* const exp = &((volatile T_CompanderCoeffs*)mod->m_dspCoefs)->expansion_coeffs;
		int i;

		for (i = 0; i < (int)STA_COMP_NUMBER_OF_IDX; i++) {
			switch (paramMask & ((u32)1<<i)) {
			//base params
			case STA_COMP_MEAN_MAX_MODE:  	// 0 or 1
				DSP_WRITE(&base->mean_max, (s32) params[i]);
				break;

			case STA_COMP_AVG_FACTOR: 		// >= 1
				if (params[i] < 1 /*|| params[i] > 32767*/) {err = STA_INVALID_VALUE; break;}
				DSP_WRITE(&base->t_av, _STA_TCtoSmoothFactor((u32)(s32)params[i], LIM_OOEPS, TRUE));
				break;

			case STA_COMP_ATTENUATION:
				if (params[i] < 0 || params[i] > 240) {err = STA_INVALID_VALUE; break;}
				#ifdef COMPANDER_VER_1
				DSP_WRITE(&base->attenuation, _STA_DB2Frac(params[i], 10, FALSE));
				#else
				DSP_WRITE(&base->gain, _STA_DB2Frac(-params[i], 10, FALSE));
				#endif
				break;

			//Compressor params
			case STA_COMP_CPR_THRES:
				if (params[i] < -1920 || params[i] > 0) {err = STA_INVALID_VALUE; break;}
				DSP_WRITE(&cpr->threshold, _STA_DB2Frac(params[i], 10, FALSE));
				break;

			case STA_COMP_CPR_SLOPE:
				if (params[i] > 0) {err = STA_INVALID_VALUE; break;}
				DSP_WRITE(&cpr->slope, ((s32)params[i] << 8));   	//convert from Q15 to Q23
				break;

			case STA_COMP_CPR_HYSTERESIS:
				if (params[i] < 0 || params[i] > 19200) {err = STA_INVALID_VALUE; break;}
				DSP_WRITE(&cpr->hysteresis, _STA_DB2Frac(params[i], 100, FALSE));
				break;

			case STA_COMP_CPR_ATK_TIME:
				if (params[i] < 0 /*|| params[i] > 32767*/) {err = STA_INVALID_VALUE; break;}
				#ifdef COMPANDER_VER_1
				DSP_WRITE(&cpr->t_attack, _STA_TCtoSmoothFactor((u32)(s32)params[i], LIM_OOEPS, TRUE));
				#else
				DSP_WRITE(&cpr->t_attack, _STA_TCtoSmoothFactor((u32)(s32)params[i], LIM_OOEPS, FALSE));
				#endif
				break;

			case STA_COMP_CPR_REL_TIME:
				if (params[i] < 0 /*|| params[i] > 32767*/) {err = STA_INVALID_VALUE; break;}
				#ifdef COMPANDER_VER_1
				DSP_WRITE(&cpr->t_release, _STA_TCtoSmoothFactor((u32)(s32)params[i], LIM_OOEPS, TRUE));
				#else
				DSP_WRITE(&cpr->t_release, _STA_TCtoSmoothFactor((u32)(s32)params[i], LIM_OOEPS, FALSE));
				#endif
				break;

			//Expander params
			case STA_COMP_EXP_THRES:
				if (params[i] < -1920 || params[i] > 0) {err = STA_INVALID_VALUE; break;}
				DSP_WRITE(&exp->threshold, _STA_DB2Frac(params[i], 10, FALSE));
				break;

			case STA_COMP_EXP_SLOPE:
				if (params[i] > 0) {err = STA_INVALID_VALUE; break;}
				DSP_WRITE(&exp->slope, (s32)params[i] << 8);  	//convert from Q15 to Q23
				break;

			case STA_COMP_EXP_HYSTERESIS:
				if (params[i] < 0 || params[i] > 19200) {err = STA_INVALID_VALUE; break;}
				DSP_WRITE(&exp->hysteresis, _STA_DB2Frac(params[i], 100, FALSE));
				break;

			case STA_COMP_EXP_ATK_TIME:
				if (params[i] < 0 /*|| params[i] > 32767*/) {err = STA_INVALID_VALUE; break;}
				#ifdef COMPANDER_VER_1
				DSP_WRITE(&exp->t_attack, _STA_TCtoSmoothFactor((u32)(s32)params[i], LIM_OOEPS, TRUE));
				#else
				DSP_WRITE(&exp->t_attack, _STA_TCtoSmoothFactor((u32)(s32)params[i], LIM_OOEPS, FALSE));
				#endif
				break;

			case STA_COMP_EXP_REL_TIME:
				if (params[i] < 0 /*|| params[i] > 32767*/) {err = STA_INVALID_VALUE; break;}
				#ifdef COMPANDER_VER_1
				DSP_WRITE(&exp->t_release, _STA_TCtoSmoothFactor((u32)(s32)params[i], LIM_OOEPS, TRUE));
				#else
				DSP_WRITE(&exp->t_release, _STA_TCtoSmoothFactor((u32)(s32)params[i], LIM_OOEPS, FALSE));
				#endif
				break;

			default: break;
			}
		}
	}

	if (err != STA_NO_ERROR) {
		SetError(err);}

_ret:
	STA_TRACE("%s(%s, paramMask:0x%x, ...)\n", __FUNCTION__, MODNAME(mod), paramMask);
	return;
}
//===========================================================================
// LIMITER
//===========================================================================
//to be called AFTER built
STA_API void STA_LimiterSetParams(STAModule module, u32 paramMask, const s16 *params)
{
	STA_ErrorCode err = STA_NO_ERROR;
	s32 i;
	tModule* const mod = CHECK_MODULE(module);

	CHECK_TYPE(STA_LMTR_1CH <= mod->m_type || mod->m_type <= STA_LMTR_6CH);
	if (!params || paramMask == 0) {goto _ret;} //nothing to do

	CHECK_BUILT(mod->m_dspCoefs);


	//compute and update the DSP coefs directly here:
	{
	tLimiter* const lim = mod->m_sub;
	volatile T_Limiter2Coeffs* const coefs = mod->m_dspCoefs;
	s32 tmp;

	for (i = (int)STA_LMTR_NUMBER_OF_IDX-1; i >= 0; i--)  // i must be decremented to set peak_rt before peak_at !!!
	{
		switch (paramMask & ((u32)1 << i))
		{
		case STA_LMTR_PEAK_ATK_TIME:  // >= 0
			if (params[i] < 0 /*|| params[i] > 32767*/) {err = STA_INVALID_VALUE; break;}
			lim->m_peakAtkTime = (u16)params[i];
			tmp = DSP_READ(&coefs->peak_rt);
			DSP_WRITE(&coefs->peak_at, tmp - _STA_TCtoSmoothFactor((u32)lim->m_peakAtkTime, LIM_OOEPS, FALSE));
			break;

		case STA_LMTR_PEAK_REL_TIME: // >= 0
			if (params[i] < 0 /*|| params[i] > 32767*/) {err = STA_INVALID_VALUE; break;}
			DSP_WRITE(&coefs->peak_rt, _STA_TCtoSmoothFactor((u32)(s32)params[i], LIM_OOEPS, FALSE));
			tmp = DSP_READ(&coefs->peak_rt);
			DSP_WRITE(&coefs->peak_at, tmp - _STA_TCtoSmoothFactor((u32)lim->m_peakAtkTime, LIM_OOEPS, FALSE));
			break;

		case STA_LMTR_THRES:
			if (params[i] < -1200 || params[i] > 0) {err = STA_INVALID_VALUE; break;}
			DSP_WRITE(&coefs->threshold, _STA_db2lin(params[i], 10, NULL, DB_CLAMP));
			break;

		case STA_LMTR_HYSTERESIS:
			if (params[i] < 0 || params[i] > 600) {err = STA_INVALID_VALUE; break;}
			DSP_WRITE(&coefs->inv_hysteresis, _STA_db2lin(-params[i], 100, NULL, DB_CLAMP));
			break;

		case STA_LMTR_ATTACK_TIME:
			if (params[i] < 0 /*|| params[i] > 32767*/) {err = STA_INVALID_VALUE; break;}
			DSP_WRITE(&coefs->t_attack, _STA_TCtoSmoothFactor((u32)(s32)params[i], LIM_OOEPS, TRUE));
			break;

		case STA_LMTR_RELEASE_TIME:
			if (params[i] < 0 /*|| params[i] > 32767*/) {err = STA_INVALID_VALUE; break;}
			DSP_WRITE(&coefs->t_release, _STA_TCtoSmoothFactor((u32)(s32)params[i], LIM_OOEPS, TRUE));
			break;

		case STA_LMTR_ATTENUATION:
			if (params[i] < 0 || params[i] > 1200) {err = STA_INVALID_VALUE; break;}
			DSP_WRITE(&coefs->attenuation, _STA_db2lin(-params[i], 10, NULL, DB_CLAMP));
			break;

		default: break;
		}
	}//for
	}

	if (err != STA_NO_ERROR) {
		SetError(err);}

_ret:
	if (params) {
		const s16* p = params;
		STA_TRACE("%s(%s, msk:0x%x, pa=%d,pr=%d,th=%d,hy=%d,at=%d,rt=%d,att=%d)\n", __FUNCTION__, MODNAME(mod), paramMask, \
				p[0], p[1], p[2], p[3], p[4], p[5], p[6]);
	} else {
		STA_TRACE("%s(%s, msk:0x%x, params:NULL)\n", __FUNCTION__, MODNAME(mod), paramMask);
	}
	return;
}
//----------------------------------------------------------------------------
STA_API s32 STA_LimiterGetParam(STAModule module, u32 param)
{
	s32 ret = 0;
	tModule* const mod = CHECK_MODULE(module);

	CHECK_TYPE(STA_LMTR_1CH <= mod->m_type || mod->m_type <= STA_LMTR_6CH);
	CHECK_BUILT(mod->m_dspCoefs);


	//Get the DSP coefs directly here:
	{
	volatile T_Limiter2Coeffs* const coefs = mod->m_dspCoefs;
	volatile T_Limiter2Data* const genData = mod->m_dspData;

	switch (param) {
//	case STA_LMTR_PEAK_ATK_TIME: 	break;
//	case STA_LMTR_PEAK_REL_TIME: 	break;
	case STA_LMTR_THRES:			ret =  _STA_lin2db(DSP_READ(&coefs->threshold))/10; break;
	case STA_LMTR_HYSTERESIS:   	ret = -_STA_lin2db(DSP_READ(&coefs->inv_hysteresis)); break;// keep format in (dB/100)
//	case STA_LMTR_ATTACK_TIME:		break;
//	case STA_LMTR_RELEASE_TIME:		break;
	case STA_LMTR_ATTENUATION:		ret = -_STA_lin2db(DSP_READ(&coefs->attenuation))/10; break;
	case STA_LMTR_DYN_GAIN:			ret =  _STA_lin2db(DSP_READ(&genData->g_1))/10;	break;
	case STA_LMTR_PEAK:				ret =  _STA_lin2db(DSP_READ(&genData->peak_sig))/10; break;

	default: SetError(STA_INVALID_PARAM);
	}
	}

_ret:
	STA_TRACE("%s(%s, %d) = %d\n", __FUNCTION__, MODNAME(mod), param, ret);
	return ret;
}
//===========================================================================
// CLIPLIMITER
//===========================================================================
//to be called AFTER built

void STA_ClipLimiterSetParams(STAModule module, u32 paramMask, const s32 *params)
{
	STA_ErrorCode err = STA_NO_ERROR;
	tModule* const mod = CHECK_MODULE(module);

	CHECK_TYPE(mod->m_type == STA_CLIPLMTR_NCH);
	if (!params || paramMask == 0) {goto _ret;} //nothing to do

	CHECK_BUILT(mod->m_dspCoefs);


	//compute and update the DSP coefs directly here:
	{
	volatile T_ClipLimiterNchParams* const coefs = mod->m_dspCoefs;
	u32 i, bitmask;

	DSP_WRITE(&coefs->final_count_down,	0);
	DSP_WRITE(&coefs->gain_goal, 		FRAC( 1.0));	// linear gain = 1.0   0.0 dB, attenuation disabled
	DSP_WRITE(&coefs->tc_factor, 	0 /*FRAC( 0.0)*/);	//
	DSP_WRITE(&coefs->gain, 			FRAC( 1.0));	// initial linear gain for each channel is 1.0
	DSP_WRITE(&coefs->inc, 			0 /*FRAC( 0.0)*/);	// initial inc for each channel is zero

	for (i = 0; i < (u32)STA_CLIPLMTR_NUMBER_OF_IDX; i++)
	{
		switch (paramMask & ((u32)1 << i))
		{
		case STA_CLIPLMTR_CLIP_BITMASK:
			bitmask = (u32)params[i];
			DSP_WRITE(&coefs->bit_mask, bitmask);
			break;

		case STA_CLIPLMTR_CLIP_POLARITY:
			bitmask = (u32)DSP_READ(&coefs->bit_mask);
			DSP_WRITE(&coefs->polarity, (params[i] > 0) ? (s32)bitmask : 0);
			break;

		case STA_CLIPLMTR_MAX_ATTENUATION:
			if (params[i] < 0 || params[i] > 1200) {err = STA_INVALID_VALUE; break;}
			DSP_WRITE(&coefs->gain_goal_att, _STA_db2lin(-params[i], 10, NULL, DB_CLAMP));
			break;

		case STA_CLIPLMTR_MIN_ATTENUATION:
			if (params[i] < 0 || params[i] > 1200) {err = STA_INVALID_VALUE; break;}
			DSP_WRITE(&coefs->gain_goal_rel, _STA_db2lin(-params[i], 10, NULL, DB_CLAMP));
			break;

		case STA_CLIPLMTR_ATTACK_TIME:
			if (params[i] < 0 || params[i] > 4000000) {err = STA_INVALID_VALUE; break;}
			DSP_WRITE(&coefs->tc_factor_att, _STA_TCtoSmoothFactor((u32)params[i], LIM_OOEPS, FALSE));
			break;

		case STA_CLIPLMTR_RELEASE_TIME:
			if (params[i] < 0 || params[i] > 4000000) {err = STA_INVALID_VALUE; break;}
			DSP_WRITE(&coefs->tc_factor_rel, _STA_TCtoSmoothFactor((u32)params[i], LIM_OOEPS, FALSE));
			break;

		case STA_CLIPLMTR_ADJ_ATTENUATION:
			if (params[i] < 0 || params[i] > 1200) {err = STA_INVALID_VALUE; break;}
			DSP_WRITE(&coefs->noise_rem_gain, _STA_db2lin(-params[i], 10, NULL, DB_CLAMP));
			break;

		default: break;
		}
	}

	DSP_WRITE(&coefs->prev_clip, 0xabcdef);             // force to reconverge after new params
	}

	if (err != STA_NO_ERROR) {
		SetError(err);}

_ret:
	if (params) {
		const s32* p = params;
		STA_TRACE("%s(%s, msk:0x%x, cbmsk=%d,pol=%d,max=%d,min=%d,at=%d,rt=%d,att=%d)\n", __FUNCTION__, MODNAME(mod), paramMask, \
				p[0], p[1], p[2], p[3], p[4], p[5], p[6]);
	} else {
		STA_TRACE("%s(%s, msk:0x%x, params:NULL)\n", __FUNCTION__, MODNAME(mod), paramMask);
	}
	return;
}
//----------------------------------------------------------------------------
STA_API s32 STA_ClipLimiterGetParam(STAModule module, u32 param)
{
	s32 ret = 0;
	tModule* const mod = CHECK_MODULE(module);

	CHECK_TYPE(mod->m_type == STA_CLIPLMTR_NCH);
	CHECK_BUILT(mod->m_dspCoefs);


	//Get the DSP coefs directly here:
	{
	volatile T_ClipLimiterNchParams* const coefs = mod->m_dspCoefs;

	switch (param) {
	case STA_CLIPLMTR_CLIP_BITMASK:		ret = DSP_READ(&coefs->bit_mask); break;
	case STA_CLIPLMTR_CLIP_POLARITY:	ret = DSP_READ(&coefs->polarity); break;
	case STA_CLIPLMTR_MAX_ATTENUATION:	ret = - _STA_lin2db(DSP_READ(&coefs->gain_goal_att))/10; break;
	case STA_CLIPLMTR_MIN_ATTENUATION:	ret = - _STA_lin2db(DSP_READ(&coefs->gain_goal_rel))/10; break;
//	case STA_CLIPLMTR_ATTACK_TIME:		break;
//	case STA_CLIPLMTR_RELEASE_TIME:		break;
	case STA_CLIPLMTR_ADJ_ATTENUATION:	ret = - _STA_lin2db(DSP_READ(&coefs->noise_rem_gain))/10; break;

	default: SetError(STA_INVALID_PARAM);
	}
	}

_ret:
	STA_TRACE("%s(%s, %d) = %d\n", __FUNCTION__, MODNAME(mod), param, ret);
	return ret;
}
//===========================================================================
// DELAY LINE
//===========================================================================
STA_API void STA_DelaySetLength(STAModule module, u32 ch, u32 delayLength)
{
	tModule* const mod = CHECK_MODULE(module);

	STA_TRACE_PUSH(0);
	STA_DelaySetLengths(module, (u32)1 << ch, delayLength);
	STA_TRACE_POP();

_ret:
	STA_TRACE("%s(%s, ch%d, %d)\n", __FUNCTION__, MODNAME(mod), ch, delayLength);
	return;
}
//----------------------------------------------------------------------------
//Sets the same delay length (in number of samples) to the masked channels
//Only effective when called before STA_BuildFlow()
STA_API void STA_DelaySetLengths(STAModule module, u32 chmask, u32 delayLength)
{
	u32 i;
	tDelay*  dlay;
	u32 *m_delayLengths, *m_delays;
	tModule* const mod = CHECK_MODULE(module);

	CHECK_TYPE(STA_DLAY_Y_2CH <= mod->m_type || mod->m_type <= STA_DLAY_X_6CH);
	CHECK_BEFORE_BUILT(mod->m_dspCoefs); //must be called before BuildFlow()

	dlay 			= mod->m_sub;
	m_delayLengths	= dlay->m_delayLengths;
	m_delays 		= dlay->m_delays;

	if (delayLength < 1) {
		delayLength = 1;} //minimum length.

	//TODO: shall we check vs max length (=??), depends on available memory...

	FORMASKEDi(mod->m_nin, chmask)
	{
		m_delayLengths[i] = delayLength;
		//clamp current delay
		if (m_delays[i] > m_delayLengths[i] - 1) {
			m_delays[i] = m_delayLengths[i] - 1;}
	}

	DLAY_AdjustDspMemSize(mod);

_ret:
	STA_TRACE("%s(%s, chmask:0x%x, %d)\n", __FUNCTION__, MODNAME(mod), chmask, delayLength);
	return;
}
//----------------------------------------------------------------------------
STA_API u32 STA_DelayGetLength(STAModule module, u32 ch)
{
	u32 ret = 0;
	const tModule* const mod = CHECK_MODULE(module);

	CHECK_TYPE(STA_DLAY_Y_2CH <= mod->m_type || mod->m_type <= STA_DLAY_X_6CH);
	CHECK_CHANNEL(ch < mod->m_nin);

	ret = ((tDelay*)mod->m_sub)->m_delayLengths[ch];

_ret:
	STA_TRACE("%s(%s, ch%d) = %d\n", __FUNCTION__, MODNAME(mod), ch, ret);
	return ret;
}
//----------------------------------------------------------------------------
//set the current delay (in number of samples) for the given channel
STA_API void STA_DelaySetDelay(STAModule module, u32 ch, u32 delay)
{
	tModule* const mod = CHECK_MODULE(module);

	STA_TRACE_PUSH(0);
	STA_DelaySetDelays(module, (u32)1 << ch, delay);
	STA_TRACE_POP();

_ret:
	STA_TRACE("%s(%s, ch%d, %d)\n", __FUNCTION__, MODNAME(mod), ch, delay);
	return;
}
//----------------------------------------------------------------------------
//set the current delay (in number of samples) for the masked channels
STA_API void STA_DelaySetDelays(STAModule module, u32 chmask, u32 delay)
{
	u32 i;
	tDelay*  dlay;
	tModule* const mod = CHECK_MODULE(module);
	dlay = mod->m_sub;

	CHECK_TYPE(STA_DLAY_Y_2CH <= mod->m_type || mod->m_type <= STA_DLAY_X_6CH);

	FORMASKEDi(mod->m_nin, chmask)
	{
		CHECK_VALUE(delay <= dlay->m_delayLengths[i] - 1);
		dlay->m_delays[i] = delay;

		//update dsp (now or during buildflow)
		if (DSPCOEFS(mod)) {
			DSP_WRITE(&((volatile T_DelayYParam*)mod->m_dspCoefs)[i].readDiff, ((s32)dlay->m_delayLengths[i] - (s32)dlay->m_delays[i]) - 1); //<- the only variable after buildflow
		}
	}

_ret:
	STA_TRACE("%s(%s, chmask:0x%x, %d)\n", __FUNCTION__, MODNAME(mod), chmask, delay);
	return;
}
//----------------------------------------------------------------------------
STA_API u32 STA_DelayGetDelay(STAModule module, u32 ch)
{
	u32 ret = 0;
	const tModule* const mod = CHECK_MODULE(module);

	CHECK_TYPE(STA_DLAY_Y_2CH <= mod->m_type || mod->m_type <= STA_DLAY_X_6CH);
	CHECK_CHANNEL(ch < mod->m_nin);

	ret = ((tDelay*)mod->m_sub)->m_delays[ch];

_ret:
	STA_TRACE("%s(%s, ch%d) = %d\n", __FUNCTION__, MODNAME(mod), ch, ret);
	return ret;
}
//===========================================================================
// SINEWAVE GENERATOR
//===========================================================================
STA_API void STA_SinewaveGenSet(STAModule module, u32 freq, u32 msec, s32 gainL, s32 gainR)
{
	tSinewaveGen* sine;
	tModule* const mod = CHECK_MODULE(module);
	sine = mod->m_sub;

	CHECK_TYPE(mod->m_type == STA_SINE_2CH);

	CHECK_VALUE( -1200 <= gainL && gainL <= 0
			 &&  -1200 <= gainR && gainR <= 0
			 &&   1000 <= freq  && freq  <= 2000000); // x0.01 Hz

	sine->m_freq = freq;
	sine->m_msec = msec;

	if (sine->m_gainDb[0] != gainL) {
		sine->m_gainDb[0] = gainL;
		sine->m_gainLn[0] = _STA_db2lin(gainL, 10, NULL, DB_CLAMP);
	}

	if (sine->m_gainDb[1] != gainR) {
		sine->m_gainDb[1] = gainR;
		sine->m_gainLn[1] = (gainR == gainL) ? sine->m_gainLn[0] : _STA_db2lin(gainR, 10, NULL, DB_CLAMP);
	}

	//update dsp
	if (mod->m_dspCoefs) {
		SINE_UpdateDspCoefs(mod);}

_ret:
	STA_TRACE("%s(%s, F:%d, msec:%d, L:%d, R:%d)\n", __FUNCTION__, MODNAME(mod), freq, msec, gainL, gainR);
	return;
}
//----------------------------------------------------------------------------
STA_API void STA_SinewaveGenPlay(STAModule module, u32 freq, u32 msec, s32 ampL, s32 ampR)
{
	tSinewaveGen* sine;
	tModule* const mod = CHECK_MODULE(module);
	sine = mod->m_sub;

	CHECK_TYPE(mod->m_type == STA_SINE_2CH);

	CHECK_VALUE( -1000 <= ampL && ampL <= 1000		// x0.001
			 &&  -1000 <= ampR && ampR <= 1000
			 &&   1000 <= freq && freq <= 2000000); // x0.01 Hz

	sine->m_freq 	  = freq;
	sine->m_msec 	  = msec;
	sine->m_gainLn[0] = FRAC1000(ampL);
	sine->m_gainLn[1] = FRAC1000(ampR);

	//update dsp
	if (mod->m_dspCoefs) {
		SINE_UpdateDspCoefs(mod);}

_ret:
	STA_TRACE("%s(%s, F:%d, msec:%d, L:%d, R:%d)\n", __FUNCTION__, MODNAME(mod), freq, msec, ampL, ampR);
	return;
}
//===========================================================================
// DCO
//===========================================================================
//can be called anytime
STA_API void STA_DCOSetMode(STAModule module, u32 chmask, u32 modemask)
{
	u32 	 i, nin;
	tDCO*	 dco;
	tModule* const mod = CHECK_MODULE(module);

	dco = mod->m_sub;
	nin = mod->m_nin-1; //-1: removing last channel dedicated to DCO_REG

	CHECK_TYPE(STA_DCO_1CH <= mod->m_type && mod->m_type <= STA_DCO_4CH_MONOZCF);

	FORMASKEDi(nin, chmask) {
		dco[i].m_mode = modemask;} //bak mode

//	mod->m_dirtyDspCoefs |= chmask; //modified channels

	//update dsp (let the dsp do the inits)
	if (mod->m_dspCoefs) { // && mod->m_dsp->m_axi->isInitsDone)
		DCO_SetMode(mod, chmask);}

_ret: return;
}
//----------------------------------------------------------------------------
//Can be called anytime, except while calibrating
STA_API void STA_DCOSetParam(STAModule module, u32 chmask, STA_DCOParam param, s32 val)
{
	STA_ErrorCode err = STA_NO_ERROR;
	u32 	 i, nin;
	tDCO*	 dco;
	tModule* const mod = CHECK_MODULE(module);

	dco = mod->m_sub;
	nin = mod->m_nin-1; //-1: removing last channel dedicated to DCO_REG

	CHECK_TYPE(STA_DCO_1CH <= mod->m_type && mod->m_type <= STA_DCO_4CH_MONOZCF);

	//Can't set parameters while calibrating.
	//(report error as wrong channel.)
	FORMASKEDi(nin, chmask) {
		if (dco[i].m_mode & (u32)STA_DCO_CALIBRATE) {
			SetError(STA_INVALID_CHANNEL); goto _ret; }
	}


	switch (param) {
	/*	case STA_DCO_BULK_DELAY:
			if (val < 0 || val > STA_MAX_DCO_DELAY) {err = STA_INVALID_VALUE; break;}
			break;

		case STA_DCO_FRAC_DELAY:  //x100 (step 0.01)
			if (val < 0 || val > 100) {err = STA_INVALID_VALUE; break;}
			break;
	*/
		case STA_DCO_FRAC_DELAY:  //(bulk + frac) x100 (step 0.01)
			if (val < 0 || val > 100 * STA_MAX_DCO_DELAY) {err = STA_INVALID_VALUE; break;}
			FORMASKEDi(nin, chmask) dco[i].m_fracDelay = (u32)val;
			break;

		case STA_DCO_WIN_THRES:   //x1000 (step 0.001)
			if (val < 0 || val > 1000) {err = STA_INVALID_VALUE; break;}
			FORMASKEDi(nin, chmask) dco[i].m_winThres = (u32)val;
			break;

		case STA_DCO_DCOFFSET:   //x1000 (step 0.001)
			if (val < -100 || val > 1000) {err = STA_INVALID_VALUE; break;}
			FORMASKEDi(nin, chmask) dco[i].m_dco = val;
			break;

		case STA_DCO_SCORE_WINDOW:
			if (val < 0) {err = STA_INVALID_VALUE; break;}  //(can be as big as we want)
			if (mod->m_type == STA_DCO_4CH_MONOZCF) {chmask |= 1;} //for monozcf, set on ch0
			FORMASKEDi(nin, chmask) dco[i].m_scoreWindow = (u32)val;
			break;

		case STA_DCO_ALERT_THRES:
			if (mod->m_type == STA_DCO_4CH_MONOZCF)	{chmask |= 1;} //for monozcf, set on ch0
			FORMASKEDi(nin, chmask) {
				if (val < 0 || (u32)val > dco[i].m_scoreWindow) {err = STA_INVALID_VALUE; break;}
				dco[i].m_alertThres = (u32)val;
			}
			break;

		default: err = STA_INVALID_PARAM; break;
	}

	if (err != STA_NO_ERROR) {
		SetError(err); goto _ret;
	}

	mod->m_dirtyDspCoefs |= chmask; //modified channels

	//update dsp (params can be set before dsp init)
	if (mod->m_dspCoefs) {//mod->m_dsp->m_axi->isInitsDone
		DCO_UpdateDspCoefs(mod);}

_ret: return;
}
//----------------------------------------------------------------------------
STA_API s32 STA_DCOGetParam(STAModule module, u32 ch, STA_DCOParam param)
{
	s32 	val = 0;
	u32 	nin;
	tDCO*	dco;
	tModule* const  mod = CHECK_MODULE(module);

	dco = mod->m_sub;
	nin = mod->m_nin - 1; //(-1 because the last input channel is for the DCO_REG)

	CHECK_TYPE(STA_DCO_1CH <= mod->m_type && mod->m_type <= STA_DCO_4CH_MONOZCF);
	CHECK_CHANNEL(ch < nin);

	//If ready, get from DSP
	if (mod->m_dspCoefs) {
		val = DCO_GetParam(mod, ch, param); goto _ret;
	}

	//else, get from driver
	switch (param) {
	case STA_DCO_FRAC_DELAY:	val = (s32)dco[ch].m_fracDelay; break;
	case STA_DCO_WIN_THRES:		val = (s32)dco[ch].m_winThres; break;
	case STA_DCO_DCOFFSET:		val = (s32)dco[ch].m_dco; break;
	case STA_DCO_SCORE_WINDOW:	val = (mod->m_type == STA_DCO_4CH_MONOZCF) ? (s32)dco[0].m_scoreWindow : (s32)dco[ch].m_scoreWindow; break;
	case STA_DCO_ALERT_THRES:	val = (mod->m_type == STA_DCO_4CH_MONOZCF) ? (s32)dco[0].m_alertThres  : (s32)dco[ch].m_alertThres; break;
	case STA_DCO_STATS_MATCH11:
	case STA_DCO_STATS_MATCH00:
	case STA_DCO_STATS_MISSED:
	case STA_DCO_STATS_FALSE:
	case STA_DCO_SCORE:
	case STA_DCO_SWZCF:
	case STA_DCO_HWZCF:			val = 0; break; //no score available

	default: SetError(STA_INVALID_PARAM);
	}

_ret: return val;
}
//===========================================================================
// PCM CHIME
//===========================================================================

//must be called BEFORE buildFLow()
STA_API void STA_PCMSetMaxDataSize(STAModule module, u32 maxbytesize)
{
	tPCMChime* pcm;
	tModule* const  mod = CHECK_MODULE(module);
	pcm = mod->m_sub;

	CHECK_TYPE(STA_PCMCHIME_12BIT_Y_2CH <= mod->m_type && mod->m_type <= STA_PCMCHIME_12BIT_X_2CH);

	pcm->m_bytesizeMax = maxbytesize;

	PCM_AdjustDspMemSize(mod);

	mod->m_dsp->m_dirtyFlow = 1;

_ret:
	STA_TRACE("%s(%s, %d)\n", __FUNCTION__, MODNAME(mod), maxbytesize);
	return;
}
//----------------------------------------------------------------------------
//must be called AFTER buildFLow() (as loading directly into the DSP mem)
STA_API void STA_PCMLoadData_16bit(STAModule module, const void* data, u32 bytesize)
{
	tPCMChime* pcm;
	tModule* const  mod = CHECK_MODULE(module);
	pcm = mod->m_sub;

	CHECK_TYPE(STA_PCMCHIME_12BIT_Y_2CH <= mod->m_type && mod->m_type <= STA_PCMCHIME_12BIT_X_2CH);
	CHECK_SIZE(bytesize <= pcm->m_bytesizeMax);
	CHECK_BUILT(mod->m_dspCoefs);
	if (!data || bytesize == 0) {goto _ret;}

	if (mod->m_dspCoefs) {
		PCM_LoadData_16bit(mod, data, bytesize);}

_ret:
	STA_TRACE("%s(%s, size:%d)\n", __FUNCTION__, MODNAME(mod), bytesize);
	return;
}
//----------------------------------------------------------------------------
STA_API void STA_PCMPlay(STAModule module, const STA_PCMChimeParams* params)
{
	tModule* const  mod = CHECK_MODULE(module);

	CHECK_TYPE(STA_PCMCHIME_12BIT_Y_2CH <= mod->m_type && mod->m_type <= STA_PCMCHIME_12BIT_X_2CH);
	if (!params) {goto _ret;}

	if (mod->m_dspCoefs) {
		PCM_Play(mod, params);}

_ret:
	if (params) {
		const STA_PCMChimeParams* p = params;
		STA_TRACE("%s(%s, cnt:%d, ld:%d, rd:%d, lpd:%d, rpd:%d)\n", __FUNCTION__, MODNAME(mod), \
				p->playCount, p->leftDelay, p->rightDelay, p->leftPostDelay, p->rightPostDelay);
	} else {
		STA_TRACE("%s(%s, params:NULL)\n", __FUNCTION__, MODNAME(mod));
	}
	return;
}
//----------------------------------------------------------------------------
STA_API void STA_PCMStop(STAModule module)
{
	tModule* const  mod = CHECK_MODULE(module);

	CHECK_TYPE(STA_PCMCHIME_12BIT_Y_2CH <= mod->m_type && mod->m_type <= STA_PCMCHIME_12BIT_X_2CH);

	if (mod->m_dspCoefs) {
		PCM_Stop(mod);} //hard stop

_ret:
	STA_TRACE("%s(%s)\n", __FUNCTION__, MODNAME(mod));
	return;
}
//===========================================================================
// POLY CHIME
//===========================================================================
/*
//not going this way....
STAChime STA_ChimeCreate(const STARampParams* ramps, int numRamps)
{
	T_Ramp* dspRamps = STA_MALLOC(sizeof(T_Ramp) * numRamps);

	if (dspRamps == 0) {
		SetError(STA_OUT_OF_MEMORY); return 0; }

	CHIME_ConvertRamps(ramps, dspRamps, numRamps);
}
*/

//must be called BEFORE buildFLow()
STA_API void STA_ChimeGenSetMaxNumRamps(STAModule module, u32 maxNumRamps)
{
	tModule* const  mod = CHECK_MODULE(module);

	CHECK_TYPE(mod->m_type == STA_CHIMEGEN);

	((tChimeGen*)mod->m_sub)->m_maxNumRamps = maxNumRamps;
	//note: dsp size will be adjusted in CHIME_AdjustDspMemSize()

	CHIME_AdjustDspMemSize(mod);

	mod->m_dsp->m_dirtyFlow = 1;

_ret:
	STA_TRACE("%s(%s, nramps:%d)\n", __FUNCTION__, MODNAME(mod), maxNumRamps);
	return;
}
//----------------------------------------------------------------------------
STA_API u32 STA_ChimeGenGetMaxNumRamps(STAModule module)
{
	u32 ret = 0;
	tModule* const  mod = CHECK_MODULE(module);

	CHECK_TYPE(mod->m_type == STA_CHIMEGEN);

	ret = ((tChimeGen*)mod->m_sub)->m_maxNumRamps;

_ret:
	STA_TRACE("%s(%s) = %d\n", __FUNCTION__, MODNAME(mod), ret);
	return ret;
}
//----------------------------------------------------------------------------
STA_API void STA_ChimeGenPlay(STAModule module, const STA_ChimeParams *params)
{
	STA_ErrorCode ret = STA_NO_ERROR;
	tModule* const mod = CHECK_MODULE(module);

	CHECK_TYPE(mod->m_type == STA_CHIMEGEN);
	if (!params || !params->ramps || params->numRamps < 1) {goto _ret;}
	CHECK_SIZE(params->numRamps <= ((tChimeGen*)mod->m_sub)->m_maxNumRamps);

	//in case this API is called to stop the current chime...
	if (params->repeatCount == 0) {
		STA_ChimeGenSetParam(module, STA_CHIME_REPEAT_COUNT, 0);
		goto _ret;
	}

	//load and play the chime data
	if (mod->m_dspCoefs) {
		ret = CHIME_Play(mod, params);} //will check the ramps values

	if (ret != STA_NO_ERROR) {
		SetError(ret);}

_ret:
	if (params) {
		const STA_ChimeParams* p = params;
		STA_TRACE("%s(%s, numRamp:%d, postIdx:%d, cnt:%d)\n", __FUNCTION__, MODNAME(mod),
				p->numRamps, p->postRepeatRampIdx, p->repeatCount);
	} else {
		STA_TRACE("%s(%s, params:NULL)\n", __FUNCTION__, MODNAME(mod));
	}
	return;
}
//----------------------------------------------------------------------------
STA_API void STA_ChimeGenStop(STAModule module, STA_RampType stopRampType, u32 stopRampTime)
{
	STA_ErrorCode ret = STA_NO_ERROR;
	tModule* const mod = CHECK_MODULE(module);

	CHECK_TYPE(mod->m_type == STA_CHIMEGEN);

	if (mod->m_dspCoefs) {
		ret = CHIME_Stop(mod, stopRampType, stopRampTime);}  //will check the ramps values

	if (ret != STA_NO_ERROR) {
		SetError(ret);}

_ret:
	STA_TRACE("%s(%s, stopType:%d, stopTime:%d)\n", __FUNCTION__, MODNAME(mod), stopRampType, stopRampTime);
	return;
}
//----------------------------------------------------------------------------
STA_API void STA_ChimeGenSetParam(STAModule module, STA_ChimeGenParam param, s32 value)
{
	tModule* const mod = CHECK_MODULE(module);

	CHECK_BUILT(mod->m_dspCoefs);

	switch (mod->m_type)
	{
	//CHIME/BEEP GENERATOR
	case STA_CHIMEGEN:
	{
		switch (param) {
	//	case STA_CHIME_STOP_RAMP_TYPE:		break;
	//	case STA_CHIME_STOP_RAMP_TIME:		break;
		case STA_CHIME_REPEAT_COUNT: 		CHIME_SetRepeatCount(mod, value); break;
		case STA_CHIME_POST_REPEAT_RAMPS:	CHIME_EnablePostRepeatRamps(mod, (bool)value); break;
		case STA_CHIME_MASTER_CHIME:
		{
			const tModule* masterModule = 0;
			if (value) {masterModule = CHECK_MODULE((STAModule)value);}
			CHIME_SetMasterChime(mod, masterModule); //(masterModule can be NULL)
			break;
		}
		default: SetError(STA_INVALID_PARAM); break;
		}
		break;
	}

	//PCM CHIME
	case STA_PCMCHIME_12BIT_Y_2CH:
	case STA_PCMCHIME_12BIT_X_2CH:
	{
		switch (param) {
		case STA_CHIME_REPEAT_COUNT: PCM_SetRepeatCount(mod, value); break;
		default: SetError(STA_INVALID_PARAM); break;
		}
		break;
	}

	default: SetError(STA_INVALID_MODULE_TYPE); break;
	}

_ret:
	STA_TRACE("%s(%s, %s, %d)\n", __FUNCTION__, MODNAME(mod), g_STA_ChimeGenParamStr[param], value);
	return;
}
//----------------------------------------------------------------------------
STA_API s32 STA_ChimeGenGetParam(STAModule module, STA_ChimeGenParam param)
{
	s32 ret = 0;
	tModule* const mod = CHECK_MODULE(module);

	CHECK_BUILT(mod->m_dspCoefs);

	switch (mod->m_type)
	{
	case STA_CHIMEGEN:
	{
		volatile T_PolyChime_Params* const coefs = mod->m_dspCoefs;	//ARM addr

		switch (param) {
		case STA_CHIME_REPEAT_COUNT:	ret = CHIME_GetRepeatCount(mod); break;
		case STA_CHIME_PLAYING: 		ret = DSP_READ(&coefs->curRamp) ? 1 : 0; break;
		case STA_CHIME_POST_REPEAT_RAMPS: ret = !(((tChimeGen*)mod->m_sub)->m_disablePostRepeatRamps); break;
		default: SetError(STA_INVALID_PARAM); break;
		}
		break;
	}
	case STA_PCMCHIME_12BIT_Y_2CH:
	case STA_PCMCHIME_12BIT_X_2CH:
	{
		volatile T_PCMChime_12bit_Params* const coefs = mod->m_dspCoefs;	//ARM addr

		switch (param) {
		case STA_CHIME_REPEAT_COUNT:	ret = DSP_READ(&coefs->playCount); break;
		case STA_CHIME_PLAYING: 		ret = DSP_READ(&coefs->playCount) != 0 ? 1 : 0; break;
		default: SetError(STA_INVALID_PARAM); break;
		}
		break;
	}
	default: SetError(STA_INVALID_MODULE_TYPE); break;
	}

_ret:
	STA_TRACE("%s(%s, %s) = %d\n", __FUNCTION__, MODNAME(mod), g_STA_ChimeGenParamStr[param], ret);
	return ret;
}
//===========================================================================
// BEEP (using Polychime)  RFU
//===========================================================================
STA_API void STA_ChimeGenBeep(STAModule module, const STA_BeepParams *params)
{
	STA_RampParams  ramps[4];
	STA_ChimeParams	beep;
	tModule* const mod = CHECK_MODULE(module);

	if (params == NULL) {goto _ret;}
	//let STA_ChimeGenPlay() checks all the params.

	//attack
	ramps[0].type = params->atkType;
	ramps[0].msec = params->atkTime;
	ramps[0].ampl = params->peakAmpl;
	ramps[0].freq = params->freq;
	//sustain
	ramps[1].type = (u32)STA_RAMP_STC;
	ramps[1].msec = params->peakTime;
	ramps[1].ampl = params->peakAmpl;
	ramps[1].freq = params->freq;
	//release
	ramps[2].type = params->relType;
	ramps[2].msec = params->relTime;
	ramps[2].ampl = 0;
	ramps[2].freq = params->freq;
	//mute time
	ramps[3].type = (u32)STA_RAMP_STC;
	ramps[3].msec = params->muteTime;
	ramps[3].ampl = 0;
	ramps[3].freq = params->freq;

	beep.ramps             = ramps;
	beep.numRamps          =(params->muteTime > 0) ? 4 : 3;
	beep.repeatCount       = (s16)params->repeatCount;
	beep.postRepeatRampIdx = (u16)-1; //no post repeat ramps

	STA_TRACE_PUSH(0);
	STA_ChimeGenPlay(module, &beep);
	STA_TRACE_POP();

_ret:
	if (params) {
		const STA_BeepParams* p = params;
		STA_TRACE("%s(%s, F:%d, d:%d, a:%d, aTy:%d, at:%d, rTy:%d, rt:%d, mt:%d, rc:%d)", __FUNCTION__, MODNAME(mod),
			p->freq, p->peakTime, p->peakAmpl, p->atkType, p->atkTime, p->relType, p->relTime, p->muteTime, p->repeatCount);
	} else {
		STA_TRACE("%s(%s, params:NULL)", __FUNCTION__, MODNAME(mod));
	}
	return;
}
//===========================================================================
// SPECTRUM METER
//===========================================================================
STA_API void STA_SpectrumMeterGetPeaks(STAModule module, s32* peaks, u32 peakFlag)
{
	tModule* const mod = CHECK_MODULE(module);

	CHECK_TYPE(mod->m_type == STA_SPECMTR_NBANDS_1BIQ);
	CHECK_BUILT(mod->m_dspCoefs);
	if (!peaks) {goto _ret;}

	SPECMTR_GetPeaks(mod, peaks, peakFlag);

_ret: return;
}
//----------------------------------------------------------------------------
STA_API void STA_SpectrumMeterSetdBscale(STAModule module, s32 dBscale)
{
	tModule* const mod = CHECK_MODULE(module);

	CHECK_TYPE(mod->m_type == STA_SPECMTR_NBANDS_1BIQ);

	((tSpectrumMeter*)mod->m_sub)->m_dBscale = dBscale;

	if (mod->m_dspCoefs) {
		SPECMTR_UpdateDspCoefs(mod);}

_ret:
	STA_TRACE("%s(%s, %d)\n", __FUNCTION__, MODNAME(mod), dBscale);
	return;
}
//----------------------------------------------------------------------------
STA_API void STA_SpectrumMeterSetDecayFactor(STAModule module, u32 msec)
{
	tModule* const mod = CHECK_MODULE(module);

	CHECK_TYPE(mod->m_type == STA_SPECMTR_NBANDS_1BIQ);

	((tSpectrumMeter*)mod->m_sub)->m_decayMsec   = msec;
	((tSpectrumMeter*)mod->m_sub)->m_decayFactor = _STA_TCtoSmoothFactor(msec * 10, STA_GAIN_EXP_OOEPS, FALSE);

	if (mod->m_dspCoefs) {
		SPECMTR_UpdateDspCoefs(mod);}

_ret:
	STA_TRACE("%s(%s, %d)\n", __FUNCTION__, MODNAME(mod), msec);
	return;
}
//===========================================================================
// FREE CYCLE COUNTER
//===========================================================================
static void EnableFreeCycleCount(bool enable)
{
	u32 i;
	FORi(3) {
		if (DSP_isLoaded(&g_dsp[i])) {
			DSP_WRITE(&g_dsp[i].m_axi->free_cycle_count_enable, (u32) enable);
		}
	}
}

/*
static u32 GetFreeCycle(STA_Dsp core)
{
	u32 ret = 0;
	volatile tAXI* axi = g_dsp[core].m_axi;

	CHECK_PARAM((u32)core <= (u32)STA_DSP2);

	//read and reset the counter
	if (axi && axi->chk_xmem_loaded == DSP_XMEM_LOADED_CODE && axi->free_cycle_count_enable)
	{
		ret = axi->free_cycle_count_min * 5 + 8;	//note: formula derived from the DSP implementation overhead
		axi->free_cycle_count_min = 3200 / 5;
	}

_ret: return ret;
}
*/
//----------------------------------------------------------------------------
//notes: freeCycles[] can be NULL, the function always return the info of DSP0
STA_API u32 STA_GetFreeCycles(u32 freeCycles[3])
{
	u32 ret;
	volatile tAXI* axi;
	int i;

	//parse all the dsp finishing with DSP0 so that ret hold the free cycle count of the DSP0
	for (i = 2;  i >= 0; i--)
	{
		ret = 0;
		axi = g_dsp[i].m_axi;

		if (axi)
		{
			s32 tmp = DSP_READ(&axi->chk_xmem_loaded);
			if (DSP_READ(&axi->free_cycle_count_enable) && tmp == DSP_XMEM_LOADED_CODE)
			{
				//read from DSP
				ret = (u32)DSP_READ(&axi->free_cycle_count_min) * 5 + 8;	//note: formula derived from the DSP implementation overhead

				//reset the counter min
				DSP_WRITE(&axi->free_cycle_count_min, 3200 / 5);
			}
		}

		if (freeCycles) {
			freeCycles[i] = ret; }
	}

	if (freeCycles) {
		STA_TRACE("%s() = dsp0:%d, dsp1:%d, dsp2:%d\n", __FUNCTION__, freeCycles[0], freeCycles[1], freeCycles[2]);
	} else {
		STA_TRACE("%s(NULL)\n", __FUNCTION__);
	}
	return ret;
}
//===========================================================================
// ENABLE / DISABLE
//===========================================================================
STA_API void STA_Enable(STA_EnableParam option)
{
	switch (option)
	{
	case STA_FREE_CYCLE_COUNTER: EnableFreeCycleCount(TRUE); break;
	case STA_CALL_TRACE:		 STA_TRACE_PUSH(1); break;

	default: SetError(STA_INVALID_PARAM);
	}

	STA_TRACE("%s(%s)\n", __FUNCTION__, g_STA_EnableParamStr[option]);
}
//----------------------------------------------------------------------------
STA_API void STA_Disable(STA_EnableParam option)
{
	STA_TRACE("%s(%s)\n", __FUNCTION__, g_STA_EnableParamStr[option]);

	switch (option)
	{
	case STA_FREE_CYCLE_COUNTER: EnableFreeCycleCount(FALSE); break;
	case STA_CALL_TRACE:		 STA_TRACE_PUSH(0); break;

	default: SetError(STA_INVALID_PARAM);
	}

}
//----------------------------------------------------------------------------
STA_API void STA_Push(STA_EnableParam option, s32 val)
{
	switch (option)
	{
//	case STA_FREE_CYCLE_COUNTER: break;
	case STA_CALL_TRACE:		 STA_TRACE_PUSH(val); break;

	default: SetError(STA_INVALID_PARAM);
	}
}
//----------------------------------------------------------------------------
STA_API void STA_Pop(STA_EnableParam option)
{
	switch (option)
	{
//	case STA_FREE_CYCLE_COUNTER: break;
	case STA_CALL_TRACE:		 STA_TRACE_POP(); break;

	default: SetError(STA_INVALID_PARAM);
	}
}

//===========================================================================
// UTILITY APIS
//===========================================================================

STA_API s32 STA_DBtoLinear(s32 db, s32 dbscale, s32 *rshift, bool clamp)
{
	s32 ret = 0;

	CHECK_VALUE(1 <= dbscale || dbscale <= 10);

	ret = _STA_db2lin(db, dbscale, rshift, clamp ? DB_CLAMP : 0);

_ret: return ret;
}
//----------------------------------------------------------------------------
STA_API u32 STA_GetInfo(STA_Info info)
{
	u32 ret = 0;
	switch ((u32)info)
	{
	case STA_INFO_LIB_VERSION:	ret = STA_LIB_VERSION; break;
	case STA_INFO_DSP_VERSION:	ret = STA_DSP_VERSION; break;
	case STA_INFO_DCO_VERSION:	ret = STA_DCO_VERSION; break;
	case STA_INFO_IS_NO_FLOAT:	ret = STA_NO_FLOAT; break;
	case STA_INFO_SAMPLING_FREQ:ret = STA_SAMPLING_FREQ; break;
	case STA_INFO_ERROR:		ret = (u32)g_drv.m_error;     g_drv.m_error = STA_NO_ERROR; break;
	case STA_INFO_LAST_ERROR:	ret = (u32)g_drv.m_errorLast; g_drv.m_errorLast = STA_NO_ERROR; break;

	default: SetError(STA_INVALID_PARAM); break;
	}

	STA_TRACE("%s(%d) = %d\n", __FUNCTION__, info, ret);
	return ret;
}
//----------------------------------------------------------------------------

static bool _isNumChChangeable(STA_ModuleType type)
{
	bool ret = FALSE;

	if ((STA_EQUA_STATIC_1BAND_DP <= type && type <= STA_EQUA_STATIC_16BANDS_SP)
	||	(STA_MIXE_2INS_NCH        <= type && type <= STA_MIXE_9INS_NCH))
	{
		ret = TRUE; goto _ret;
	}

	switch (type)
	{
	case STA_GAIN_STATIC_NCH:
	case STA_GAIN_SMOOTH_NCH:
	case STA_GAIN_LINEAR_NCH:
	case STA_BITSHIFTER_NCH:
	case STA_PEAKDETECT_NCH:
	case STA_CLIPLMTR_NCH:
		ret = TRUE; break;
	default: break;
	}

_ret: return ret;

}
//----------------------------------------------------------------------------
STA_API u32 STA_GetModuleInfo(STAModule module, STA_ModuleInfo info)
{
	u32 ret = 0;
	tModule* const mod = CHECK_MODULE(module);

	switch (info)
	{
	case STA_INFO_ID:					ret = mod->m_id; break;
	case STA_INFO_TYPE: 				ret = (u32)mod->m_type; break;
	case STA_INFO_NUM_IN_CHANNELS:		ret = mod->m_nin; break;
	case STA_INFO_NUM_OUT_CHANNELS:		ret = mod->m_nout; break;
	case STA_INFO_NUM_MIXER_IN_PER_CHANNELS: ret = mod->m_ninpc; break; //TODO: check is MIXER
	case STA_INFO_NUM_BANDS:			ret = mod->m_nfilters; break;   //TODO: check is EQ
	case STA_INFO_IS_NCH_CHANGEABLE:	ret = (u32)_isNumChChangeable(mod->m_type); break;
	case STA_INFO_MODE:					ret = mod->m_mode; break;
	case STA_INFO_SIZEOF_PARAMS:		ret = mod->m_sizeofParams; break;
	case STA_INFO_WSIZEOF_XDATA:		ret = mod->m_wsizeofData; break;
	case STA_INFO_WSIZEOF_YDATA:		ret = mod->m_wsizeofCoefs; break;
	case STA_INFO_XDATA:				ret = (u32)mod->m_dspData; break;
	case STA_INFO_YDATA:				ret = (u32)mod->m_dspCoefs; break;
	case STA_INFO_MAX_DSP_CYCLES:		ret = mod->m_mainCycleCost + mod->m_updateCycleCost; break;

	default: SetError(STA_INVALID_PARAM); break;
	}

_ret:
	STA_TRACE("%s(%s, %d) = %d\n", __FUNCTION__, MODNAME(mod), info, ret);
	return ret;
}

//----------------------------------------------------------------------------
//param 'name': if not null must point to a memory owned by the caller
//returns the module name (pointing to a char[] owned by STA.
STA_API const char* STA_GetModuleName(STAModule module, char* name)
{
	static const char* const notamodule = "not_a_module";
	const char* ret = notamodule;

	tModule* const mod = CHECK_MODULE(module);

	ret = mod->m_name;

	if (name) {
		strncpy(name, ret, STA_MAX_MODULE_NAME_SIZE-1);
	}

_ret:
	STA_TRACE("%s(%s)\n", __FUNCTION__, MODNAME(mod));
	return ret;
}
