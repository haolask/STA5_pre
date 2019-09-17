/***********************************************************************************/
/*!
*
*  \file      dsp.c
*
*  \brief     <i><b> STAudioLib DSP manager </b></i>
*
*  \details   Build the audio flow on the Emerad DSPs on Accordo2
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

#ifdef LINUX_OS
#include "trace.h"
#endif


//////////////////////////////////////////////////////
//local config
#define HAS_LOAD_DEFAULT_FW			0
#define CHECK_DSP					1
//////////////////////////////////////////////////////


//IRQ stuff...
#ifdef FREE_RTOS
//decl local FSYNC IRQ handler, registered in sta10xx_m3_irq.c
void FSYNC_IRQHandler(void);
//decl local DSP IRQ handler
void DSP_IRQHandler_(DSP_Core core);
#endif


static STA_ErrorCode DSP_CheckBinaries(const tDSP* dsp);



#define XIN_DSP_OFFSET(core)		(((u32)(core) == 2) ? 0x3000 : 0x1000)
#define XOUT_DSP_OFFSET(core)		(((u32)(core) == 2) ? 0x3080 : 0x1080)

#define XIN_DMABUS_ADDR(core)		(((u32)(core) == 0) ? DMABUS_XIN0_BASE : \
									 ((u32)(core) == 1) ? DMABUS_XIN1_BASE : \
														  DMABUS_XIN2_BASE)

#define XOUT_DMABUS_ADDR(core)		(((u32)(core) == 0) ? DMABUS_XOUT0_BASE : \
									 ((u32)(core) == 1) ? DMABUS_XOUT1_BASE : \
														  DMABUS_XOUT2_BASE)



//lint -esym(960, FOR_EACH_MODULE, FOR_EACH_CONNECTION)		inhibits misra2004 19.4 "disallowed definition of macro" (can't add parenthesis or bracket here or it won't compile)
//lint -esym(9022, FOR_EACH_MODULE, FOR_EACH_CONNECTION)	inhibits misra2004 19.10 "unparenthesized parameters"
#define FOR_EACH_MODULE(it, list)  		LIST_FOR_EACH(it, list, tModule*)
#define FOR_EACH_CONNECTION(it, list) 	LIST_FOR_EACH(it, list, tConnection*)

//----------------------------------------------------------------------------
#ifdef LINUX_OS

static int g_skip_dsp_write = 0;

u32 _DSP_readl(const volatile void *addr)
{
	return *(const vu32*)addr;
}

//write a word (u32) to DSP followed by a read so to "ensure" a valid write.
int _DSP_writel(volatile void *addr, u32 value)
{
	TRACE_PRINT((u32)addr, value);

	if (!g_skip_dsp_write)
		*(vu32*)addr = value;
	return *(const vu32*)addr;
}

void _DSP_skip_write(int onoff)
{
	g_skip_dsp_write = onoff;
}
#endif //LINUX_OS

//--------------------------------------------------------------------------------
//DSP access must be 32bit word, word aligned
void _DSP_memset(volatile void* ptr, int value, unsigned int num)
{
	vu32 *dst = ptr;
	u32 i;

	sta_assert(num % 4 == 0);
	num >>= 2;	//now num Words
	//we should also check that 'ptr' is word aligned...

	if (!dst) {goto _ret;}

	for (i = 0; i < num; i++) {
		DSP_WRITE(dst++, value);}

_ret: return;
}

//DSP access must be 32bit word, word aligned
void _DSP_memcpy(volatile void* dst, const void* src, unsigned int num)
{
	const vu32 *_src = src;
	      vu32 *_dst = dst;
	u32 i;

	if (!_dst || !_src) {goto _ret;}

	sta_assert(num % 4 == 0);
	num >>= 2;	//now num Words
	//we should also check that 'dst' is word aligned...

	for (i = 0; i < num; i++) {
		DSP_WRITE(_dst++, *_src++);}

_ret: return;
}

//----------------------------------------------------------------------------
void DSP_Init(tDSP* dsp, STA_Dsp core, u32 baseAddress)
{
	const char* xinName[3]  = {"XIN0",  "XIN1",  "XIN2"};
	const char* xoutName[3] = {"XOUT0", "XOUT1", "XOUT2"};

	if (!dsp || baseAddress == 0) {goto _ret;}

	memset(dsp, 0, sizeof(tDSP));
	dsp->m_core 			= core;
	dsp->m_pram				= (vu32*) baseAddress;
	dsp->m_xram				= (vu32*)(baseAddress + DSP_XRAM_OFFSET(core));
	dsp->m_yram	 			= (vu32*)(baseAddress + DSP_YRAM_OFFSET(core));
	dsp->m_axi  			= (volatile tAXI*)(volatile void*)(dsp->m_xram + DSP_AXI_OFFSET); //lint !e960 misra2004 11.5 cast away volatile warning: DSP_AXI_OFFSET is in dsp word!
	dsp->m_ayi  			= (volatile tAYI*)(volatile void*)(dsp->m_yram + DSP_AYI_OFFSET); //lint !e960 misra2004 11.5 cast away volatile warning: DSP_AYI_OFFSET is in dsp word!
	dsp->m_numOfUpdateSlots = 6;
	dsp->m_dspISR			= 0;

	//xin module
	MOD_SetModuleInfo(&dsp->m_xin, &g_modulesInfo[STA_XIN]);
	dsp->m_xin.m_type		= STA_XIN;
	dsp->m_xin.m_dsp	 	= dsp;
	dsp->m_xin.m_refCode 	= STA_MODULE_REF_CODE;
	dsp->m_xin.m_dspData	= (volatile void*)(baseAddress + DSP_XIN_OFFSET(core));
	dsp->m_xin.m_xdata		= XIN_DSP_OFFSET(core);
	strncpy(dsp->m_xin.m_name, xinName[core], STA_MAX_MODULE_NAME_SIZE-1);

	//xout module
	MOD_SetModuleInfo(&dsp->m_xout, &g_modulesInfo[STA_XOUT]);
	dsp->m_xout.m_type		= STA_XOUT;
	dsp->m_xout.m_dsp	 	= dsp;
	dsp->m_xout.m_refCode	= STA_MODULE_REF_CODE;
	dsp->m_xout.m_dspData	= (volatile void*)(baseAddress + DSP_XOUT_OFFSET(core));
	dsp->m_xout.m_xdata		= XOUT_DSP_OFFSET(core);
	strncpy(dsp->m_xout.m_name, xoutName[core], STA_MAX_MODULE_NAME_SIZE-1);

	//reset main cycle cost
	dsp->m_mainCycleCost	= MAIN_LOOP_DEFAULT_CYCLES; // dsp->m_updateCycleCost[] was yet set to 0 by memset


	//enable DSP clk
	//IMPORTANT: Before accessing the DSP mem, we must enable its clock.
	DSP_ENABLE_CLOCK(core);

//	dsp->m_isInitialised = 1;

_ret: return;
}
//----------------------------------------------------------------------------
void DSP_Clean(tDSP* dsp)
{
	vu32 *xmem, *ymem;
	u32 i;

	if (!dsp) {goto _ret;}

	sta_assert(DSP_CLOCK_IS_ENABLED(dsp->m_core));	//DSP clk must be on to access its mem
	sta_assert(DSP_isLoaded(dsp));	//so to have a valid 'pXmem_pool'
	sta_assert(!dsp->m_isRunning);	//ensured from calling DSP_Stop_() just before

	//delete modules and connections
	LIST_DelAll(&dsp->m_modules);
	LIST_DelAll(&dsp->m_connections);

	//reset the DSP side of XIN+XOUT dual port (from AHB2DSP bridge)
    DSP_MEMSET(dsp->m_xin.m_dspData, 0, (STA_XIN_SIZE + STA_XOUT_SIZE)*4);

	//reset the dsp's xmem/ymem pools
	xmem = (vu32 *)DSP2ARM(dsp->m_xram, dsp->m_axi->pXmem_pool);	//ARM offset
	ymem = (vu32 *)DSP2ARM(dsp->m_yram, dsp->m_ayi->pYmem_pool);	//ARM offset
	DSP_MEMSET(xmem, 0, (u32)DSP_READ(&dsp->m_axi->xmem_pool_size) * 4);
	DSP_MEMSET(ymem, 0, (u32)DSP_READ(&dsp->m_ayi->ymem_pool_size) * 4);
	dsp->m_xmemPoolUsedSize = 0;
	dsp->m_ymemPoolUsedSize = 0;
	dsp->m_xmemPoolEstimatedSize = 0;

	dsp->m_mainCycleCost = MAIN_LOOP_DEFAULT_CYCLES;
	FORi(STA_MAX_UPDATE_SLOTS) {
		dsp->m_updateCycleCost[i] = 0;
	}

	//now, we can build a new flow ;-)
	dsp->m_isFlowBuilt = 0;

_ret: return;
}
//----------------------------------------------------------------------------
int DSP_isLoaded(const tDSP* dsp)
{
	int ret = 0;

	if (dsp && dsp->m_baseAddr && dsp->m_axi && dsp->m_ayi) {
		if (DSP_READ(&dsp->m_axi->chk_xmem_loaded) == DSP_XMEM_LOADED_CODE) {
			if (DSP_READ(&dsp->m_ayi->chk_ymem_loaded) == DSP_YMEM_LOADED_CODE) {
				ret = 1; goto _ret;
			}
		}
	}
_ret: return ret;
}

//----------------------------------------------------------------------------
void DSP_Load_ext(tDSP* dsp, const char *Paddr, u32 Psize, const char *Xaddr, u32 Xsize, const char *Yaddr, u32 Ysize)
{
    u32 PMaxSize, XMaxSize, YMaxSize;
	if (!dsp) {goto _ret;}

	//Make sure the dsp clk is on, but dsp is stopped
	DSP_ENABLE_CLOCK(dsp->m_core);
	DSP_Stop_(dsp);

	//remove the headers
	//note: the binaries have a header of 0x10 bytes to remove.
	if (Paddr && Paddr[0] == 'I' && Paddr[1] == 'M' && Paddr[2] == 'G') {
		Paddr += 0x10; Psize -= 0x10;}
	if (Xaddr && Xaddr[0] == 'I' && Xaddr[1] == 'M' && Xaddr[2] == 'G') {
		Xaddr += 0x10; Xsize -= 0x10;}
	if (Yaddr && Yaddr[0] == 'I' && Yaddr[1] == 'M' && Yaddr[2] == 'G') {
		Yaddr += 0x10; Ysize -= 0x10;}

    PMaxSize = 4*(DSP_PRAM_SIZE(dsp->m_core)-2); // limit to PRAM size minus 2 last words
    XMaxSize = 4*DSP_XRAM_SIZE(dsp->m_core); // limit to XRAM size
    YMaxSize = 4*DSP_YRAM_SIZE(dsp->m_core); // limit to YRAM size

	if (Psize > PMaxSize) { sta_assert(0); }
	if (Xsize > XMaxSize) { sta_assert(0); }
	if (Ysize > YMaxSize) { sta_assert(0); }

	if (Paddr)
	{
        DSP_MEMCPY(dsp->m_pram, Paddr, Psize);

        // write 0x0 in 2 last instruction words to avoid bug after RSR
        DSP_MEMSET(dsp->m_pram+Psize, 0x0, 2*4);
    }
	if (Xaddr){DSP_MEMCPY(dsp->m_xram, Xaddr, Xsize);}
	if (Yaddr){DSP_MEMCPY(dsp->m_yram, Yaddr, Ysize);}

_ret: return;
}
//----------------------------------------------------------------------------
#if defined(FREE_RTOS)

//load the dsp firmware linked in the OS image.
void DSP_Load(tDSP* dsp)
{
#if HAS_LOAD_DEFAULT_FW
	char *Paddr = 0, *Xaddr = 0, *Yaddr = 0;
	u32   Psize = 0,  Xsize = 0,  Ysize = 0;

	if (!dsp) {goto _ret;}

	//get the source addresses of the dsp binaries
	switch ((u32)dsp->m_core)
	{
	case 0:
	case 1:
		Paddr = (char*)(void*)&address_P0;  Psize = (u32)&size_P0 - (u32)Paddr;
		Xaddr = (char*)(void*)&address_X0;  Xsize = (u32)&size_X0 - (u32)Xaddr;
		Yaddr = (char*)(void*)&address_Y0;  Ysize = (u32)&size_Y0 - (u32)Yaddr;
		break;
	case 2:
		Paddr = (char*)(void*)&address_P2;  Psize = (u32)&size_P2 - (u32)Paddr;
		Xaddr = (char*)(void*)&address_X2;  Xsize = (u32)&size_X2 - (u32)Xaddr;
		Yaddr = (char*)(void*)&address_Y2;  Ysize = (u32)&size_Y2 - (u32)Yaddr;
		break;

	default: sta_assert(0); break;
	}

	if (Paddr) {
		DSP_Load_ext(dsp, Paddr, Psize, Xaddr, Xsize, Yaddr, Ysize);
	}

_ret: return;

#else
	(void)(u32)(void*) dsp;
#endif
}

#else
void DSP_Load(tDSP* dsp)
{
	(void)(u32)(void*) dsp;
}
#endif // FREE_RTOS

//----------------------------------------------------------------------------
void DSP_Start_(tDSP* dsp)
{
	if (!dsp) {goto _ret;}

    //TODO: DSP_Start() activate IRQ/NVIC


    //Start the DSP
#ifndef WIN32
    DSP_ENABLE_CLOCK(dsp->m_core);
    DSP_START(dsp->m_core);
#else //WIN32
	//for PC simulation (these flags are normally set by the dsp)
	dsp->m_axi->chk_dsp_running = DSP_RUNNING_CODE;
	dsp->m_axi->isInitsDone = 1;
#endif

_ret: return;
}
//----------------------------------------------------------------------------
void DSP_Stop_(tDSP* dsp)
{
	if (!dsp) {goto _ret;}

    //Stop the DSP
    //Note: For now we don't disable the clk, otherwise we won't be able to access the DSP mems.
#ifndef WIN32
//	DSP_DISABLE_CLOCK(core);
	DSP_STOP(dsp->m_core);
#endif

	if (dsp->m_axi) {
		DSP_WRITE(&dsp->m_axi->chk_dsp_running, 0);
		DSP_WRITE(&dsp->m_axi->isInitsDone,     0);
	}
	dsp->m_isRunning = 0;

_ret: return;
}
//----------------------------------------------------------------------------
// additional params: id, name, tags
// assumes that API has checked that 'id' is unique
tModule* DSP_AddModule2(tDSP* dsp, STA_ModuleType type, const STA_UserModuleInfo *info, u32 id, const char* name, u32 tags)
{
	tModule* mod =  0;

	if (!dsp) {goto _ret;} {

	const u32 sizeofSub = (info) ? info->m_sizeofParams : g_modulesInfo[type].sizeofParams;

	//create and add a new module to the list
	mod = (tModule*) LIST_AddNew(&dsp->m_modules, sizeof(tModule) + sizeofSub);
	if (!mod) {goto _ret;} //out of memory

	//set the sub-params ptr
	mod->m_sub = (sizeofSub > 0) ? (void*)((u32)mod + sizeof(tModule)) : NULL;

	//init the sub-params
	MOD_Init(mod, type, id, dsp, info, name, tags);

	//compute the module cycles and update the dsp stats
	MOD_ComputeModuleCycles(mod);
	dsp->m_mainCycleCost += mod->m_mainCycleCost;
	dsp->m_updateCycleCost[mod->m_updateSlot] += mod->m_updateCycleCost;

	//compute the module dsp mem and update the dsp stats
	if (mod->AdjustDspMemSize) {mod->AdjustDspMemSize(mod);}
	dsp->m_xmemPoolEstimatedSize += mod->m_wsizeofData;
	dsp->m_ymemPoolEstimatedSize += mod->m_wsizeofCoefs;


	dsp->m_dirtyFlow = 1;

	}
_ret: return mod;
}
//----------------------------------------------------------------------------
void DSP_DelModule(tDSP* dsp, tModule* mod)
{
	if (!dsp || !mod) {goto _ret;}

	//update dsp cycles
	dsp->m_mainCycleCost -= mod->m_mainCycleCost;
	dsp->m_updateCycleCost[mod->m_updateSlot] -= mod->m_updateCycleCost;

	//update dsp memory
	dsp->m_xmemPoolEstimatedSize -= mod->m_wsizeofData;
	dsp->m_ymemPoolEstimatedSize -= mod->m_wsizeofCoefs;

	//remove and free up
	LIST_DelElmt(&dsp->m_modules, mod);

	dsp->m_dirtyFlow = 1;

_ret: return;
}

//----------------------------------------------------------------------------
tModule* DSP_FindModuleByID(const tDSP* dsp, u32 id)
{
	tModule* ret = 0;
	LIST_ITERATOR(tModule*, mod);

	if (!dsp) {goto _ret;}{

	FOR_EACH_MODULE(mod, &dsp->m_modules) {
		if (mod && mod->m_id == id) {
			ret = mod; break;
		}
	}

	}
_ret: return ret;
}
//----------------------------------------------------------------------------
tModule* DSP_FindModuleByName(const tDSP* dsp, const char* name)
{
	tModule* ret = 0;
	LIST_ITERATOR(tModule*, mod);

	if (!dsp) {goto _ret;}{

	FOR_EACH_MODULE(mod, &dsp->m_modules) {
		if (mod && name) {
			if (strncmp(mod->m_name, name, STA_MAX_MODULE_NAME_SIZE) == 0) {
				ret = mod; break;
			}
		}
	}

	}
_ret: return ret;
}
//----------------------------------------------------------------------------
tConnection* DSP_AddConnection(tDSP* dsp, tModule* from, u32 chout, tModule* to, u32 chin)
{
	tConnection* con = 0;

	if (!dsp) {goto _ret;}{

	con = (tConnection*) LIST_AddNew(&dsp->m_connections, sizeof(tConnection));
	if (!con) {goto _ret;} //out of memory

	con->from  = from;
	con->to    = to;
	con->chout = (u16)chout;
	con->chin  = (u16)chin;

	dsp->m_mainCycleCost += PER_MONO_CONNECTION_CYCLES;
	dsp->m_dirtyFlow = 1;

	}
_ret: return con;
}
//----------------------------------------------------------------------------
void DSP_ReconnectFrom(tConnection* con, tModule* newfrom, u32 newchout)
{
	if (!con) {goto _ret;}

	con->from  = newfrom;
	con->chout = (u16)newchout;

	if (con->armAddr) {
		DSP_WRITE(con->armAddr, MOD_GetDspOutAddr(con->from, con->chout));	//src
	}

_ret: return;
}
//----------------------------------------------------------------------------
void DSP_ReconnectTo(tConnection* con, tModule* newto, u32 newchin)
{
	if (!con) {goto _ret;}

	con->to    = newto;
	con->chin  = (u16)newchin;

	if (con->armAddr) {
		DSP_WRITE(con->armAddr+4, MOD_GetDspInAddr(con->to, con->chin));	//dst
	}

_ret: return;
}
//----------------------------------------------------------------------------
tConnection* DSP_GetConnection(tDSP* dsp, tModule* from, u32 chout, tModule* to, u32 chin)
{
	tConnection* ret = 0;

	if (!dsp) {goto _ret;}{

	LIST_ITERATOR(tConnection*, con);

	FOR_EACH_CONNECTION(con, &dsp->m_connections)
	{
		if (   con
			&& con->from == from
			&& con->to   == to
			&& con->chout== chout
			&& con->chin == chin)
		{
			ret = con; break;
		}
	}

	}
_ret: return ret;
}
//----------------------------------------------------------------------------
//Note: Not used because, since the connections may belong to different dsps, this is handled
//      directly within the API call STA_Connectv().
/*
void DSP_AddConnections(tDSP* dsp, const STAConnector* ctors, u32 num, STAConnection* cons )
{
	tListElmt* oldLast = dsp->m_connections.m_last;
	int i;

	for ( i = 0; i < num; i++ )
	{
		tConnection* con = (tConnection*) LIST_AddNew(&dsp->m_connections, sizeof(tConnection));
		if (!con) goto error; //out of memory

//		con->from  = (tModule*)ctors[i].from;
//		con->to    = (tModule*)ctors[i].to;
//		con->chout = ctors[i].chout;
//		con->chin  = ctors[i].chin;
		*con = *(tConnection*) &ctors[i]; 		//copy

		if (cons)
			cons[i] = (STAConnection) con;
	}

	dsp->m_dirtyFlow = 1;
	return;
error:
	//delete all the connections that just have been added
	if ( i > 0 ) {
		oldLast = (oldLast) ? oldLast->m_next : dsp->m_connections.m_first;
		sta_assert(oldLast);
		LIST_DelElmts(&dsp->m_connections, oldLast, NULL);
	}
}
*/
//----------------------------------------------------------------------------
void DSP_DelConnection(tDSP* dsp, tConnection* con)
{
	if (!dsp) {goto _ret;} {

	unsigned int oldsize = dsp->m_connections.m_size;

	LIST_DelElmt(&dsp->m_connections, con);

	if (oldsize != dsp->m_connections.m_size) {
		dsp->m_mainCycleCost -= PER_MONO_CONNECTION_CYCLES;
	}

	dsp->m_dirtyFlow = 1;

	}
_ret: return;
}
//----------------------------------------------------------------------------
//return error
static STA_ErrorCode DSP_CheckBinaries(const tDSP* dsp)
{
	STA_ErrorCode ret = STA_NO_ERROR;

	if (!dsp) {goto _ret;}

	//TODO(optm) DSP_FillTables: check if we really have XMEM data to load, (apart from g_xmem_pool, which could be moved in AYI)
	//check that the XRAM is properly loaded
	if (DSP_READ(&dsp->m_axi->chk_xmem_loaded) != DSP_XMEM_LOADED_CODE) {
		ret = (STA_ErrorCode)((u32)STA_DSP0_XMEM_NOT_LOADED + (u32)dsp->m_core); goto _ret;}

	//check that the YRAM is properly loaded, mainly for alibFuncTable
	if (DSP_READ(&dsp->m_ayi->chk_ymem_loaded) != DSP_YMEM_LOADED_CODE) {
		ret = (STA_ErrorCode)((u32)STA_DSP0_YMEM_NOT_LOADED + (u32)dsp->m_core); goto _ret;}

	//check that DSP lib matches STAudioLib.
	if (DSP_READ(&dsp->m_ayi->alib_version) != STA_DSP_VERSION) {
		ret = (STA_ErrorCode)((u32)STA_DSP0_LIB_NOT_MATCHING + (u32)dsp->m_core); goto _ret;}

_ret: return ret;
}

//----------------------------------------------------------------------------
STA_ErrorCode DSP_UpdateEstimatedSizes(tDSP* dsp)
{
	STA_ErrorCode ret = STA_NO_ERROR;

	if (!dsp || !dsp->m_ayi) {goto _ret;} {

	const T_FilterFuncs* dsp_alibFuncTable = (T_FilterFuncs*)(void*)DSP2ARM(dsp->m_yram, DSP_READ(&dsp->m_ayi->pAlib_func_table));
	const T_FilterFuncs* dsp_userFuncTable = (T_FilterFuncs*)(void*)DSP2ARM(dsp->m_yram, DSP_READ(&dsp->m_ayi->pUser_func_table));

	u32 estimated_xsize = 0, estimated_ysize = 0;		//in dsp Words!
	u32 numtransf;

	LIST_ITERATOR(tModule*, mod);


	//--- checks -----------------------------------

	//Check for correct DSP fw version
#if CHECK_DSP
	ret = DSP_CheckBinaries(dsp);
	if (ret != STA_NO_ERROR) {goto _ret;}
#endif


//	//TODO(low) DSP_FillTables: check that all channels of all modules are connected


	//--- XMEM and YMEM size estimation ------------------
	//Estimate the total XMEM and YMEM allocation (in Word size)

	//minimum for init() and audio() tables
	estimated_ysize += 2 * WSIZEOF(T_Filter); 				//exit_loop() x 2


	//minimum for update() table
	//need at minimum:
	//T_Filter   _YMEM g_updates_slot0[] = {{ exit_loop_incSlot, NULL, NULL }}
	//T_Filter   _YMEM g_updates_slot1[] = {{ exit_loop_incSlot, NULL, NULL }}
	// ...
	//T_Filter   _YMEM g_updates_slot5[] = {{ exit_loop_resetSlot, NULL, NULL }}
	//T_Filter   _YMEM *g_updates[] = { g_updates_slot0, ... g_updates_slot5}
	estimated_ysize += dsp->m_numOfUpdateSlots * (WSIZEOF(T_Filter) + 1);

	//minimum for transfers()
	//The DSP firmware requires an even number of transfers (DSP firmware optimization)
	numtransf = (dsp->m_connections.m_size + 1) & 0xFFFFFFFE; //force even
	estimated_ysize += numtransf * 2 + 1/*numtransf*/;		//transfers

//	printf("Start memory allocation on DSP%d\r\n", dsp->m_core);

	//+ XMEM and YMEM allocated per module
	FOR_EACH_MODULE(mod, &dsp->m_modules) { if (mod)
	{
		if (mod->AdjustDspMemSize) {
			mod->AdjustDspMemSize(mod);}

		if (mod->m_wsizeofData) {
			const T_FilterFuncs* dsp_funcTable = (mod->m_type < STA_MOD_LAST) ? dsp_alibFuncTable : dsp_userFuncTable;  //ST or USER effects

			//data and coefs
			estimated_xsize += mod->m_wsizeofData;			//all data
			estimated_ysize += mod->m_wsizeofCoefs;			//all coefs

			//audio() entry
			estimated_ysize += WSIZEOF(T_Filter);			//audio()

//			printf("total xsize = %-6d module:x:%-6d    |", estimated_xsize, mod->m_wsizeofData);
//			printf("total ysize = %-6d module:y:%d +  %d\r\n", estimated_ysize, mod->m_wsizeofCoefs, WSIZEOF(T_Filter));

			//init() and update() entries
			if (dsp_funcTable) {

				//init() entry
				if (DSP_READ(&dsp_funcTable[mod->m_dspType].initFunc)) {
					estimated_ysize += WSIZEOF(T_Filter);}	//init()

				//updates() entry
				if (DSP_READ(&dsp_funcTable[mod->m_dspType].updateFunc)) {
					estimated_ysize += WSIZEOF(T_Filter);}	//update()
			}
		}
	}}

	dsp->m_xmemPoolEstimatedSize = estimated_xsize;
	dsp->m_ymemPoolEstimatedSize = estimated_ysize;
	}
_ret: return ret;
}

//----------------------------------------------------------------------------
STA_ErrorCode DSP_FillTables(tDSP* dsp)
{
	STA_ErrorCode ret = STA_NO_ERROR;

	if (!dsp) {goto _ret;} {

	volatile tAXI* const m_axi = dsp->m_axi;
	volatile tAYI* const m_ayi = dsp->m_ayi;
	int		const m_core 	   = (int)dsp->m_core;
	tList*  const m_modules    = &dsp->m_modules;
	tList*  const m_connections= &dsp->m_connections;

    //DSP offset of x|ymem pools
	tDspPtr  xmem_do = (tDspPtr)DSP_READ(&m_axi->pXmem_pool);	//DSP offset
	tDspPtr  ymem_do = (tDspPtr)DSP_READ(&m_ayi->pYmem_pool);	//DSP offset

    //ARM offset of x|ymem pools
	vu32* xmem  = (vu32 *)DSP2ARM(dsp->m_xram, xmem_do);	//ARM offset
	vu32* ymem  = (vu32 *)DSP2ARM(dsp->m_yram, ymem_do);	//ARM offset

	//DSP function tables
	const T_FilterFuncs* m_alibTable = (T_FilterFuncs*)(void*)DSP2ARM(dsp->m_yram, DSP_READ(&m_ayi->pAlib_func_table));
	const T_FilterFuncs* m_userTable = (T_FilterFuncs*)(void*)DSP2ARM(dsp->m_yram, DSP_READ(&m_ayi->pUser_func_table));

	u32 xsize = 0, ysize = 0;		//in Words!
	u32 numtransf, i;

	LIST_ITERATOR(tModule*, mod);
	LIST_ITERATOR(tConnection*, con);

	//reset the DSP side of XIN+XOUT dual port (from AHB2DSP bridge)
	DSP_MEMSET(dsp->m_xin.m_dspData, 0, (STA_XIN_SIZE + STA_XOUT_SIZE)*4);

	//reset the DMABUS side of XIN+XOUT dual port (from AHB2DMA bridge).
	//note: On Acccordo2, the DSP cannot work asynchronously when triggering the DSP DualPort,
	//      (writing at XIN[127]), for e.g. ECNR on DSP2. Therefore, do not reset XIN[127]
	if (AUDIO_P4_BASE) {
		XIN_DMABUS_CLEAR(dsp->m_core);
		XOUT_DMABUS_CLEAR(dsp->m_core);
	}

	//--- XMEM and YMEM sizes checks ------------------

	ret = DSP_UpdateEstimatedSizes(dsp);
	if (ret != STA_NO_ERROR) { goto _ret;}

//	printf("DSP%d total xsize = %d / %d\r\n", m_core, dsp->m_xmemPoolEstimatedSize, DSP_XMEM_POOL_WSIZE);
//	printf("DSP%d total ysize = %d / %d\r\n", m_core, dsp->m_ymemPoolEstimatedSize, DSP_YMEM_POOL_WSIZE);


	//Check that ymemPool is big enough (in WordSize).
	if (dsp->m_ymemPoolEstimatedSize > (u32)DSP_READ(&m_ayi->ymem_pool_size)) {
		STA_PRINTF("STA: not enough space in YMEM_POOL of DSP%d to build the audio flow\n", m_core);
		ret = (STA_ErrorCode)((u32)STA_DSP0_OUT_OF_YMEM + (u32)m_core);
		goto _ret;
	}

	//Check that xmemPool is big enough (in WordSize).
	if (dsp->m_xmemPoolEstimatedSize > (u32)DSP_READ(&m_axi->xmem_pool_size)) {
		STA_PRINTF("STA: not enough space in XMEM_POOL of DSP%d to build the audio flow\n", m_core);
		ret = (STA_ErrorCode)((u32)STA_DSP0_OUT_OF_XMEM + (u32)m_core);
		goto _ret;
	}


	//--- XMEM and YMEM reset -------------------------

	//reset the flag first
	dsp->m_isFlowBuilt = 0;

	//reset the xmem/ymem pool usage
	//To optimize time, let's just clean the estimated sizes instead of the whole dsp mem pools.
	DSP_MEMSET(xmem, 0, dsp->m_xmemPoolEstimatedSize * 4);
	DSP_MEMSET(ymem, 0, dsp->m_ymemPoolEstimatedSize * 4);

	dsp->m_xmemPoolUsedSize = 0;
	dsp->m_ymemPoolUsedSize = 0;
	//xsize = ysize = 0;


	//--- MODULES ------------------------------------------------------

	//1. Fill the YMEM coefs and XMEM data (simultaneously)
	FOR_EACH_MODULE(mod, m_modules) {
		if (mod && mod->m_wsizeofData) {
			mod->m_dspData  = &xmem[xsize]; xsize += mod->m_wsizeofData;
			mod->m_dspCoefs = &ymem[ysize]; ysize += mod->m_wsizeofCoefs;
			mod->m_xdata	= (u32) ARM2DSP(dsp->m_xram, mod->m_dspData);
			mod->m_ycoef	= (u32) ARM2DSP(dsp->m_yram, mod->m_dspCoefs);

			//dsp coefs shadow buffers: 'Front' = dsp side, 'Back' = arm side
			mod->m_pDspCoefFront = mod->m_dspCoefs;
			mod->m_pDspCoefBack  = (mod->m_coefsShadowing) ? (volatile void*)((vu32*)mod->m_dspCoefs + (mod->m_wsizeofCoefs/2)) : mod->m_dspCoefs; //lint !e960 misra2004 11.5 cast away volatile

			if (mod->InitDspData) {
				mod->InitDspData(mod);}
		}
	}


	//2. Fill the AUDIO func table (YMEM)
	DSP_WRITE(&m_ayi->pFunc_table, (T_Filter*)(ymem_do + ysize));
	FOR_EACH_MODULE(mod, m_modules) {
		const T_FilterFuncs* m_funcTable = (mod && mod->m_type < STA_MOD_LAST) ? m_alibTable : m_userTable;  //ST or USER effects
		if (mod && mod->m_dspType != (u32)NO_DSP_TYPE && m_funcTable) {
			if(DSP_READ(&m_funcTable[mod->m_dspType].audioFunc)) {
				sta_assert((u32)mod->m_dspType < NUM_DSP_TYPE);
				DSP_WRITE(&ymem[ysize++], DSP_READ(&m_funcTable[mod->m_dspType].audioFunc));
				DSP_WRITE(&ymem[ysize++], mod->m_xdata);
				DSP_WRITE(&ymem[ysize++], (mod->m_pDspCoefFront == mod->m_dspCoefs) ? (s32*)mod->m_ycoef : (s32*)(mod->m_ycoef + mod->m_wsizeofCoefs/2));

				//HACK for modules supporting dsp coef update shadowing (Front/Back coefs)
				mod->m_pFuncTableYcoef = &ymem[ysize-1]; //backup this address
			}
		}
	}
	DSP_WRITE(&ymem[ysize++], DSP_READ(&m_alibTable[EXIT_LOOP].audioFunc)); //exit_loop();
	DSP_WRITE(&ymem[ysize++], NULL);
	DSP_WRITE(&ymem[ysize++], NULL);


	//3. Fill the INIT func table (YMEM)
	DSP_WRITE(&m_ayi->pInit_table, (T_Filter*)(ymem_do + ysize));
	FOR_EACH_MODULE(mod, m_modules) {
		const T_FilterFuncs* m_funcTable = (mod && mod->m_type < STA_MOD_LAST) ? m_alibTable : m_userTable;  //ST or USER effects
		if (mod && mod->m_dspType != (u32)NO_DSP_TYPE && m_funcTable) {
			if (DSP_READ(&m_funcTable[mod->m_dspType].initFunc)) {
				sta_assert((u32)mod->m_dspType < NUM_DSP_TYPE);
				DSP_WRITE(&ymem[ysize++], DSP_READ(&m_funcTable[mod->m_dspType].initFunc));
				DSP_WRITE(&ymem[ysize++], mod->m_xdata);
				DSP_WRITE(&ymem[ysize++], mod->m_ycoef);
			}
		}
	}
	DSP_WRITE(&ymem[ysize++], DSP_READ(&m_alibTable[EXIT_LOOP].audioFunc)); //exit_loop();
	DSP_WRITE(&ymem[ysize++], NULL);
	DSP_WRITE(&ymem[ysize++], NULL);


	//4. Fill the updates func table (YMEM)
	//TODO DSP_FillTables: fill the update_slots sub-tables !!!
	//		meantime keep to the default passthrough_updates
	{
	u32  numPerSlots[STA_MAX_UPDATE_SLOTS] = {0};
	u32  offsets[STA_MAX_UPDATE_SLOTS+1]; //+1 for final update[] table
	vu32* armAddr[STA_MAX_UPDATE_SLOTS+1];

	const u32 numSlots = dsp->m_numOfUpdateSlots;

	//count how many updates per slots
	FOR_EACH_MODULE(mod, m_modules) {
		const T_FilterFuncs* m_funcTable = (mod && mod->m_type < STA_MOD_LAST) ? m_alibTable : m_userTable;  //ST or USER effects
		if (mod && mod->m_dspType != (u32)NO_DSP_TYPE && m_funcTable) {
			if (DSP_READ(&m_funcTable[mod->m_dspType].updateFunc)) {
				numPerSlots[mod->m_updateSlot]++;
			}
		}
	}

	//precompute the offset for each update_slot[] tables
	offsets[0] = ysize;
	armAddr[0] = ymem + offsets[0];
	for (i = 1; i <= numSlots; i++) {
		offsets[i] = offsets[i-1] + (numPerSlots[i-1] + 1) * WSIZEOF(T_Filter); //+1 for the exit_loop_incSlot()
		armAddr[i] = ymem + offsets[i];
	}

	//fill all the update_slot[] tables simultaneously
	FOR_EACH_MODULE(mod, m_modules) {
		const T_FilterFuncs* m_funcTable = (mod && mod->m_type < STA_MOD_LAST) ? m_alibTable : m_userTable;  //ST or USER effects
		if (mod && mod->m_dspType != (u32)NO_DSP_TYPE && m_funcTable) {
			if (DSP_READ(&m_funcTable[mod->m_dspType].updateFunc)) {
				DSP_WRITE(armAddr[mod->m_updateSlot]++, DSP_READ(&m_funcTable[mod->m_dspType].updateFunc));
				DSP_WRITE(armAddr[mod->m_updateSlot]++, mod->m_xdata);
				DSP_WRITE(armAddr[mod->m_updateSlot]++, mod->m_ycoef);
			}
		}
	}

	//add the exit_loop_xxxx() functions
	FORi(numSlots-1) {
		DSP_WRITE(armAddr[i]++, DSP_READ(&m_alibTable[EXIT_LOOP].initFunc)); //exit_loop_incSlot()
		DSP_WRITE(armAddr[i]++, NULL);
		DSP_WRITE(armAddr[i]++, NULL);
	}
	DSP_WRITE(armAddr[i]++, DSP_READ(&m_alibTable[EXIT_LOOP].updateFunc)); //exit_loop_resetSlot()
	DSP_WRITE(armAddr[i]++, NULL);
	DSP_WRITE(armAddr[i]++, NULL);

	//add the final pUpdate_table[]
	ysize = offsets[numSlots];
	DSP_WRITE(&m_ayi->pUpdate_table, (T_Filter**)(ymem_do + ysize));
	FORi(numSlots) {
		DSP_WRITE(&ymem[ysize++], (s32*)(ymem_do + offsets[i]));
	}

	}

	//--- CONNECTIONS ---------------------------------------------

	//5. Fill the transfers table (YMEM)

	//The DSP firmware requires an even number of transfers (DSP firmware optimization)
	numtransf = (dsp->m_connections.m_size + 1) & 0xFFFFFFFE; //force even

	DSP_WRITE(&m_ayi->pTransfer_table, (fraction**)(ymem_do + ysize));
	DSP_WRITE(&ymem[ysize++], numtransf); //the first elmt is the num of transfers

	FOR_EACH_CONNECTION(con, m_connections) { if (con) {
		con->armAddr = (u32)&ymem[ysize];
		DSP_WRITE(&ymem[ysize++], MOD_GetDspOutAddr(con->from, con->chout));	//src
		DSP_WRITE(&ymem[ysize++], MOD_GetDspInAddr(con->to, con->chin));		//dst
	}}
	//if the num transfer was not even, duplicate the last transfer.
	if (m_connections->m_size & 1) {
		DSP_WRITE(&ymem[ysize], DSP_READ(&ymem[ysize-2])); ysize++;
		DSP_WRITE(&ymem[ysize], DSP_READ(&ymem[ysize-2])); ysize++;
	}


	//--- Checks -----------------------------------------------------

	//check versus estimations
	if (xsize != dsp->m_xmemPoolEstimatedSize) {
		ret = (STA_ErrorCode)((u32)STA_DSP0_XMEM_SIZE_ERROR + (u32)m_core);	goto _ret; }
	if (ysize != dsp->m_ymemPoolEstimatedSize) {
		ret = (STA_ErrorCode)((u32)STA_DSP0_YMEM_SIZE_ERROR + (u32)m_core);	goto _ret; }

	sta_assert(xsize < (u32)DSP_READ(&m_axi->xmem_pool_size));
	sta_assert(ysize < (u32)DSP_READ(&m_ayi->ymem_pool_size));

	dsp->m_xmemPoolUsedSize = xsize;
	dsp->m_ymemPoolUsedSize = ysize;

	dsp->m_isFlowBuilt = 1;
//	dsp->m_isInitsDone = 0;
	dsp->m_dirtyFlow   = 0;

	}
_ret: return ret;
}

//----------------------------------------------------------------------------
// DSP IRQ
//----------------------------------------------------------------------------
#ifdef FREE_RTOS

//IRQ handler dispatcher
void DSP_IRQHandler_(DSP_Core core)
{
	if (g_dsp[core].m_dspISR) {
		g_dsp[core].m_dspISR((STA_Dsp)core);
	} else {
		STA_PRINTF("STA: DSP_IRQHandler_(%d)\n", core);
	}

	//clear the IRQ (done by DSP_IRQHandler())
	//DSP0->IRQ_D2A_CL = 1;
}
//----------------------------------------------------------------------------
void FSYNC_IRQHandler(void)
{
	if (g_drv.m_fsyncISR) {
		g_drv.m_fsyncISR(); }

//	else
//		STA_PRINTF("STA: FSYNC_IRQHandler()%s\n", "");

	//clear the IRQ
	AUDCR->ACO_ICR.BIT.FSYNC_ICR = 1;
}
//----------------------------------------------------------------------------

#if 0
#ifdef CORTEX_M3
static void DSP_InitIRQ(tDSP* dsp)
{
	//Route DSP0 interrupt from R4 to M3 (EXT0_IRQChannel)
    //see A2 DS: M3 interrupt request interface (ch.7.5)
	//
	//DSP0 IRQ is hardwired to BLOCK0_INTIN_10:
	//
    //       wired               (1)    wired               wired         (2)   (3)
    //DSP0 IRQ -> BLOCK0_INTIN_10 -> IRQ0 -> EXT0_IRQChannel -> NVIC_IRQ16 -> M3 -> EXT0_IRQHandler()
	//
    //Then need to:
    //1) set M3IRQ so to contribute to BLOCK0_IRQ0 (unmask). -> done in M3IRQ_Init()
    //3) replace EXT0_IRQHandler() with DSP0_IRQHandler()    -> done in M3IRQ_Init()
    //2) set NVIC to enable EXT0_IRQChannel

	/*
	//this is done in M3IRQ_Init()
	//M3IRQ (0x40038000)
    //DSP0 IRQ is mapped to INTIN_10 of block0, make it contribute to BLOCK0_IRQ0
    *(u32 *)(0x40038004) |= (1 < 10);	//M3IRQ_MASKCLR0		unmask DSP0 IRQ on M3IRQ
    *(u32 *)(0x40038010) &= ~0xC00;		//M3IRQ_IRQSEL_LOWER0	map MTU0  IRQ to BLOCK0_IRQ0 why ???
    *(u32 *)(0x400380C0) &= ~0x3;		//M3IRQ_INTIN16_RSEL0	map GPIO0 IRQ to BLOCK0_IRQ0 why ???
	*/

	//NVIC(0xE000E100):	Enable IRQChannel
	/*
	switch (dsp->m_core) {
	case 0:	NVIC_EnableIRQChannel(EXT0_IRQChannel, 0, 0); break;
	case 1:	NVIC_EnableIRQChannel(EXT4_IRQChannel, 0, 0); break;
	case 2:	NVIC_EnableIRQChannel(EXT1_IRQChannel, 0, 0); break;
	}
	*/
}

#else // !CORTEX_M3
static void DSP_InitIRQ(tDSP* dsp)
{
	/*
	//VIC
	//example from RTX...
	//ROTARY IRQ -> S_GPIO4 IRQ -> combined S_GPIO IRQ (31) ->R4 VIC -> ...
	VIC_ISEL0  &= ~VIC_MASK_SGPIO_31;					//SELECT: IRQ (default)
	VIC_IENS0  |=  VIC_MASK_SGPIO_31;					//ENABLE: IRQ 31
	VIC_VCRx[0] =  VIC_VCR_EN | 31;						//set Handler
	VIC_VARx[0] =  (u32)GPIO_S_IRQHandler;
	*/
}
#endif // CORTEX_M3
#endif // 0

#endif //FREE_RTOS


