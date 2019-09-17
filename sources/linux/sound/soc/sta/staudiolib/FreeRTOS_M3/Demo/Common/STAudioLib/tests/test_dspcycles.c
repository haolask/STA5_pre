/*
 * STAudioLib - test_dspcycles.c
 *
 * Created on 2017/06/20 by Olivier Douvenot
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	Testing cycles
 */

#if 1
#include "staudio_test.h"
#include "dsp_cycles_def.h"


struct ModuleInfo {
	STAModule			stamod;
	STA_ModuleType		mod_type;
	u32					mod_cycles;
	u32					mod_xmem;
	u32					mod_ymem;
};


static STAModule g_gain, g_limiter6ch, g_compander6ch;

static u32 g_dsp_estimated_cycles;
static u32 g_dsp_estimated_xmem;
static u32 g_dsp_estimated_ymem;


static void check_stats(STA_Dsp core, struct ModuleInfo *mod)
{
	u32 mod_cycles, dsp_cycles;
	u32 mod_xmem, dsp_xmem;
	u32 mod_ymem, dsp_ymem;


//	if (0 <= core && core <= 2)
	{
		dsp_cycles = STA_GetMaxDspCycleCost(core);
		CHECK(dsp_cycles == g_dsp_estimated_cycles);

		STA_GetDspMemCost(core, &dsp_xmem, &dsp_ymem);
		CHECK(dsp_xmem == g_dsp_estimated_xmem);
		CHECK(dsp_ymem == g_dsp_estimated_ymem);
	}

	if (mod)
	{
		mod_cycles = STA_GetModuleInfo(mod->stamod, STA_INFO_MAX_DSP_CYCLES);
		CHECK(mod_cycles == mod->mod_cycles);

		mod_xmem = STA_GetModuleInfo(mod->stamod, STA_INFO_WSIZEOF_XDATA);
		CHECK(mod_xmem == mod->mod_xmem);

		mod_ymem = STA_GetModuleInfo(mod->stamod, STA_INFO_WSIZEOF_YDATA);
		CHECK(mod_ymem == mod->mod_ymem);
		(void) mod_ymem;
	}
}

static void check_add(STA_Dsp core, struct ModuleInfo *mod)
{
	//add module
	mod->stamod = STA_AddModule(core, mod->mod_type);

	//update stats
	g_dsp_estimated_cycles += mod->mod_cycles;
	g_dsp_estimated_xmem   += mod->mod_xmem;
	g_dsp_estimated_ymem   += mod->mod_ymem;

	g_dsp_estimated_ymem   += 3;// audio func
	switch (mod->mod_type)
	{
	case STA_GAIN_STATIC_NCH:
	case STA_LOUD_STATIC_DP:
	case STA_LOUD_STATIC_STEREO_DP:
	case STA_TONE_STATIC_DP:
	case STA_TONE_STATIC_STEREO_DP:
	case STA_DCO_1CH:
	case STA_DCO_4CH_MONOZCF:
		g_dsp_estimated_ymem   += 3;// init func
        break;

    case STA_COMP_1CH:
	case STA_COMP_2CH:
	case STA_COMP_4CH:
	case STA_COMP_6CH:
	case STA_LMTR_1CH:
	case STA_LMTR_2CH:
	case STA_LMTR_4CH:
	case STA_LMTR_6CH:
		g_dsp_estimated_ymem   += 3;// update func
        break;

    default:
        break;
    }

	check_stats(core, mod);
}

static void check_adduser(STA_Dsp core, struct ModuleInfo *mod, STA_UserModuleInfo *userInfo)
{
	//add user module
	mod->stamod		= STA_AddUserModule(core, userInfo->m_type, userInfo);
	mod->mod_type   = userInfo->m_type;
	mod->mod_cycles = userInfo->m_mainCycleCost + userInfo->m_updateCycleCost + MAIN_LOOP_PER_MODULE_CYCLES;
	mod->mod_xmem   = userInfo->m_wsizeofData;
	mod->mod_ymem   = userInfo->m_wsizeofCoefs;

	//update stats
	g_dsp_estimated_cycles += mod->mod_cycles;
	g_dsp_estimated_xmem   += mod->mod_xmem;
	g_dsp_estimated_ymem   += mod->mod_ymem;
	g_dsp_estimated_ymem   += 3;// audio func

	check_stats(core, mod);
}

static void check_update(STA_Dsp core, struct ModuleInfo *mod, struct ModuleInfo *upd)
{
	//update stats
	g_dsp_estimated_cycles += upd->mod_cycles - mod->mod_cycles;
	g_dsp_estimated_xmem   += upd->mod_xmem - mod->mod_xmem;
	g_dsp_estimated_ymem   += upd->mod_ymem - mod->mod_ymem;

	check_stats(core, upd);
}

static void check_delete(STA_Dsp core, struct ModuleInfo *mod)
{
	//del module
	STA_DeleteModule(&mod->stamod);

	//update stats
	g_dsp_estimated_cycles -= mod->mod_cycles;
	g_dsp_estimated_xmem   -= mod->mod_xmem;
	g_dsp_estimated_ymem   -= mod->mod_ymem;

	g_dsp_estimated_ymem   -= 3;// audio func
	switch (mod->mod_type)
	{
	case STA_GAIN_STATIC_NCH:
	case STA_LOUD_STATIC_DP:
	case STA_LOUD_STATIC_STEREO_DP:
	case STA_TONE_STATIC_DP:
	case STA_TONE_STATIC_STEREO_DP:
	case STA_DCO_1CH:
	case STA_DCO_4CH_MONOZCF:
		g_dsp_estimated_ymem   -= 3;// init func
        break;

    case STA_COMP_1CH:
	case STA_COMP_2CH:
	case STA_COMP_4CH:
	case STA_COMP_6CH:
	case STA_LMTR_1CH:
	case STA_LMTR_2CH:
	case STA_LMTR_4CH:
	case STA_LMTR_6CH:
		g_dsp_estimated_ymem   -= 3;// update func
        break;

    default:
        break;
    }

	check_stats(core, NULL);
}


//---------------------------------------------------------
static void BuildFlow(STA_Dsp core)
{
	STA_ErrorCode sta_err;
	STA_DspIO xin, xout;
	STA_UserModuleInfo user_info = {0};
	struct ModuleInfo mod_info, mod_upd;
	u32 xmem, mod_xmem, cycles, mod_cycles;
	int i, j;


	if (core == STA_DSP0)
	{
	    xin  = STA_XIN0;
	    xout = STA_XOUT0;
	}
	else if (core == STA_DSP1)
	{
	    xin  = STA_XIN1;
	    xout = STA_XOUT1;
	}
	else //(core == STA_DSP2)
	{
	    xin  = STA_XIN2;
	    xout = STA_XOUT2;
	}

	//---- main loop stats  -----------------------------------------

	g_dsp_estimated_cycles = MAIN_LOOP_DEFAULT_CYCLES;
	g_dsp_estimated_xmem   = 0;
	g_dsp_estimated_ymem   = 31;// exit loop(6)+update slots(6*4)+transfers(1)
	FORi(3) {
		check_stats(i, NULL);
	}

	//---- STA_USER_0+1 --------------------------------------------

	//add User module
	user_info.m_type = STA_USER_0+1;
	user_info.m_dspType = 0;
	user_info.m_mainCycleCost = 99;
	user_info.m_updateCycleCost = 1;
	user_info.m_wsizeofData = 421;
	user_info.m_wsizeofCoefs = 0;
	check_adduser(core, &mod_info, &user_info);
	check_delete(core, &mod_info);

	//---- STA_USER_0+3 --------------------------------------------

	//add User module
	user_info.m_type = STA_USER_0+3;
	user_info.m_dspType = 0;
	user_info.m_mainCycleCost = 11;
	user_info.m_updateCycleCost = 9;
	user_info.m_wsizeofData = 123;
	user_info.m_wsizeofCoefs = 0;
	check_adduser(core, &mod_info, &user_info);
	check_delete(core, &mod_info);

	//---- STA_PCMCHIME_12BIT_X_2CH --------------------------------------------

	//add module
	mod_info.mod_type   = STA_PCMCHIME_12BIT_X_2CH;
	mod_info.mod_cycles = PCMCHIME_12BIT_X_2CH_MAIN_CYCLES + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = sizeof(T_PCMChime_12bit_Data)>>2;
	mod_info.mod_ymem   = sizeof(T_PCMChime_12bit_Params)>>2;
	check_add(core, &mod_info);

	//STA_PCMSetMaxDataSize
	STA_PCMSetMaxDataSize(mod_info.stamod, 400);
	mod_upd = mod_info;
	mod_upd.mod_xmem = (sizeof(T_PCMChime_12bit_Data)>>2)+100;
	check_update(core, &mod_info, &mod_upd);

	//del module
	check_delete(core, &mod_upd);

	//---- STA_PCMCHIME_12BIT_Y_2CH --------------------------------------------

	//add module
	mod_info.mod_type   = STA_PCMCHIME_12BIT_Y_2CH;
	mod_info.mod_cycles = PCMCHIME_12BIT_Y_2CH_MAIN_CYCLES + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = sizeof(T_PCMChime_12bit_Data)>>2;
	mod_info.mod_ymem   = sizeof(T_PCMChime_12bit_Params)>>2;
	check_add(core, &mod_info);

	//STA_PCMSetMaxDataSize
	STA_PCMSetMaxDataSize(mod_info.stamod, 400);
	mod_upd = mod_info;
	mod_upd.mod_ymem = (sizeof(T_PCMChime_12bit_Params)>>2)+100;
	check_update(core, &mod_info, &mod_upd);

	//del module
	check_delete(core, &mod_upd);

	//---- STA_CHIMEGEN --------------------------------------------

	//add module
	mod_info.mod_type   = STA_CHIMEGEN;
	mod_info.mod_cycles = CHIMEGEN_MAIN_CYCLES + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = sizeof(T_PolyChime_Data)>>2;
	mod_info.mod_ymem   = sizeof(T_PolyChime_Params)>>2;
	check_add(core, &mod_info);

	//STA_ChimeGenSetMaxNumRamps
	STA_ChimeGenSetMaxNumRamps(mod_info.stamod, 40);
	mod_upd = mod_info;
	mod_upd.mod_ymem   = (sizeof(T_PolyChime_Params)>>2)+(sizeof(T_Ramp)>>2)*40;
	check_update(core, &mod_info, &mod_upd);

	//del module
	check_delete(core, &mod_upd);

	//---- STA_SINE_2CH --------------------------------------------

	mod_info.mod_type   = STA_SINE_2CH;
	mod_info.mod_cycles = SINE_2CH_MAIN_CYCLES + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = (sizeof(T_SinewaveData)>>2) + 2;
	mod_info.mod_ymem   = (sizeof(T_SinewaveCoeffs)>>2);
	check_add(core, &mod_info);
	check_delete(core, &mod_info);

	//---- STA_DLAY_Y_2CH --------------------------------------------

	//add module
	mod_info.mod_type   = STA_DLAY_Y_2CH;
	mod_info.mod_cycles = DELAY_Y_2CH_MAIN_CYCLES + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = (sizeof(T_DelayData)>>2)*2+2*2;
	mod_info.mod_ymem   = (sizeof(T_DelayYParam)>>2)*2+2*1;
	check_add(core, &mod_info);

	//STA_DelaySetLengths
	STA_DelaySetLengths(mod_info.stamod, 0x3, 7);
	mod_upd = mod_info;
	mod_upd.mod_ymem   = (sizeof(T_DelayYParam)>>2)*2+2*7;
	check_update(core, &mod_info, &mod_upd);

	//del module
	check_delete(core, &mod_upd);

	//---- STA_DLAY_Y_4CH --------------------------------------------

	//add module
	mod_info.mod_type   = STA_DLAY_Y_4CH;
	mod_info.mod_cycles = DELAY_Y_4CH_MAIN_CYCLES + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = (sizeof(T_DelayData)>>2)*4+2*4;
	mod_info.mod_ymem   = (sizeof(T_DelayYParam)>>2)*4+4*1;
	check_add(core, &mod_info);

	//STA_DelaySetLengths
	STA_DelaySetLengths(mod_info.stamod, 0xF, 7);
	mod_upd = mod_info;
	mod_upd.mod_ymem   = (sizeof(T_DelayYParam)>>2)*4+4*7;
	check_update(core, &mod_info, &mod_upd);

	//del module
	check_delete(core, &mod_upd);

	//---- STA_DLAY_Y_6CH --------------------------------------------

	//add module
	mod_info.mod_type   = STA_DLAY_Y_6CH;
	mod_info.mod_cycles = DELAY_Y_6CH_MAIN_CYCLES + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = (sizeof(T_DelayData)>>2)*6+2*6;
	mod_info.mod_ymem   = (sizeof(T_DelayYParam)>>2)*6+6*1;
	check_add(core, &mod_info);

	//STA_DelaySetLengths
	STA_DelaySetLengths(mod_info.stamod, 0x3F, 7);
	mod_upd = mod_info;
	mod_upd.mod_ymem   = (sizeof(T_DelayYParam)>>2)*6+6*7;
	check_update(core, &mod_info, &mod_upd);

	//del module
	check_delete(core, &mod_upd);

	//---- STA_DLAY_X_2CH --------------------------------------------

	//add module
	mod_info.mod_type   = STA_DLAY_X_2CH;
	mod_info.mod_cycles = DELAY_X_2CH_MAIN_CYCLES + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = (sizeof(T_DelayData)>>2)*2+2*2+2;
	mod_info.mod_ymem   = (sizeof(T_DelayXParam)>>2)*2;
	check_add(core, &mod_info);

	//STA_DelaySetLengths
	STA_DelaySetLengths(mod_info.stamod, 0x3, 7);
	mod_upd = mod_info;
	mod_upd.mod_xmem = (sizeof(T_DelayData)>>2)*2+2*2+2*7;
	check_update(core, &mod_info, &mod_upd);

	//del module
	check_delete(core, &mod_upd);

	//---- STA_DLAY_X_4CH --------------------------------------------

	//add module
	mod_info.mod_type   = STA_DLAY_X_4CH;
	mod_info.mod_cycles = DELAY_X_4CH_MAIN_CYCLES + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = (sizeof(T_DelayData)>>2)*4+2*4+4;
	mod_info.mod_ymem   = (sizeof(T_DelayXParam)>>2)*4;
	check_add(core, &mod_info);

	//STA_DelaySetLengths
	STA_DelaySetLengths(mod_info.stamod, 0xF, 7);
	mod_upd = mod_info;
	mod_upd.mod_xmem = (sizeof(T_DelayData)>>2)*4+2*4+4*7;
	check_update(core, &mod_info, &mod_upd);

	//del module
	check_delete(core, &mod_upd);

	//---- STA_DLAY_X_6CH --------------------------------------------

	//add module
	mod_info.mod_type   = STA_DLAY_X_6CH;
	mod_info.mod_cycles = DELAY_X_6CH_MAIN_CYCLES + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = (sizeof(T_DelayData)>>2)*6+2*6+6;
	mod_info.mod_ymem   = (sizeof(T_DelayXParam)>>2)*6;
	check_add(core, &mod_info);

	//STA_DelaySetLengths
	STA_DelaySetLengths(mod_info.stamod, 0x3F, 7);
	mod_upd = mod_info;
	mod_upd.mod_xmem = (sizeof(T_DelayData)>>2)*6+2*6+6*7;
	check_update(core, &mod_info, &mod_upd);

	//del module
	check_delete(core, &mod_upd);

	//---- STA_TONE_STATIC_STEREO_DP --------------------------------------------

	mod_info.mod_type   = STA_TONE_STATIC_STEREO_DP; //2 ch by default
	mod_info.mod_cycles = TONE_DP_2CH_MAIN_CYCLES + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = sizeof(T_ToneDual_Data)>>2;
	mod_info.mod_ymem   = (sizeof(T_Tone_Coefs)>>2)*2;
	check_add(core, &mod_info);
	check_delete(core, &mod_info);

	//---- STA_TONE_STATIC_DP --------------------------------------------

	mod_info.mod_type   = STA_TONE_STATIC_DP; //1 ch by default
	mod_info.mod_cycles = TONE_DP_1CH_MAIN_CYCLES + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = sizeof(T_Tone_Data)>>2;
	mod_info.mod_ymem   = (sizeof(T_Tone_Coefs)>>2)*2;
	check_add(core, &mod_info);
	check_delete(core, &mod_info);

	//---- STA_LOUD_STATIC_STEREO_DP --------------------------------------------

	mod_info.mod_type   = STA_LOUD_STATIC_STEREO_DP; //2 ch by default
	mod_info.mod_cycles = LOUDNESS_DP_2CH_MAIN_CYCLES + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = sizeof(T_LoudnessDual_Data)>>2;
	mod_info.mod_ymem   = (sizeof(T_Loudness_Coefs)>>2)*2;
	check_add(core, &mod_info);
	check_delete(core, &mod_info);

	//---- STA_LOUD_STATIC_DP --------------------------------------------

	mod_info.mod_type   = STA_LOUD_STATIC_DP; //1 ch by default
	mod_info.mod_cycles = LOUDNESS_DP_1CH_MAIN_CYCLES + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = sizeof(T_Loudness_Data)>>2;
	mod_info.mod_ymem   = (sizeof(T_Loudness_Coefs)>>2)*2;
	check_add(core, &mod_info);
	check_delete(core, &mod_info);

	//---- STA_CD_DEEMPH --------------------------------------------

	mod_info.mod_type   = STA_CD_DEEMPH; //2 ch by default
	mod_info.mod_cycles = CD_DEEMPH_2CH_MAIN_CYCLES + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = (sizeof(T_CDDeemphasisData)>>2)+4;
	mod_info.mod_ymem   = (sizeof(T_CDDeemphasisParam)>>2);
	check_add(core, &mod_info);
	check_delete(core, &mod_info);

	//---- STA_MUX_2OUT --------------------------------------------

	mod_info.mod_type   = STA_MUX_2OUT;
	mod_info.mod_cycles = MUX_2OUT_MAIN_CYCLES + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = sizeof(T_Mux2outData)>>2;
	mod_info.mod_ymem   = sizeof(T_Mux2outParam)>>2;
	check_add(core, &mod_info);
	check_delete(core, &mod_info);

	//---- STA_MUX_4OUT --------------------------------------------

	mod_info.mod_type   = STA_MUX_4OUT;
	mod_info.mod_cycles = MUX_4OUT_MAIN_CYCLES + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = sizeof(T_Mux4outData)>>2;
	mod_info.mod_ymem   = sizeof(T_Mux4outParam)>>2;
	check_add(core, &mod_info);
	check_delete(core, &mod_info);

	//---- STA_MUX_6OUT --------------------------------------------

	mod_info.mod_type   = STA_MUX_6OUT;
	mod_info.mod_cycles = MUX_6OUT_MAIN_CYCLES + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = sizeof(T_Mux6outData)>>2;
	mod_info.mod_ymem   = sizeof(T_Mux6outParam)>>2;
	check_add(core, &mod_info);
	check_delete(core, &mod_info);

	//---- STA_MUX_8OUT --------------------------------------------

	mod_info.mod_type   = STA_MUX_8OUT;
	mod_info.mod_cycles = MUX_8OUT_MAIN_CYCLES + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = sizeof(T_Mux8outData)>>2;
	mod_info.mod_ymem   = sizeof(T_Mux8outParam)>>2;
	check_add(core, &mod_info);
	check_delete(core, &mod_info);

	//---- STA_MUX_10OUT --------------------------------------------

	mod_info.mod_type   = STA_MUX_10OUT;
	mod_info.mod_cycles = MUX_10OUT_MAIN_CYCLES + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = sizeof(T_Mux10outData)>>2;
	mod_info.mod_ymem   = sizeof(T_Mux10outParam)>>2;
	check_add(core, &mod_info);
	check_delete(core, &mod_info);

	//---- STA_SPECMTR_NBANDS_1BIQ --------------------------------------------

	//add module
	mod_info.mod_type   = STA_SPECMTR_NBANDS_1BIQ;  //9 bands by default
	mod_info.mod_cycles = SPECMTR_NBANDS_1BIQ_MAIN_CYCLES + SPECMTR_NBANDS_1BIQ_PER_BAND_CYCLES*9 + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = ((sizeof(T_Biquad_Optim_SP_Chained_Data)>>2)+3)*9;
	mod_info.mod_ymem   = (sizeof(T_SpectrumMeter_Coefs)>>2)+(sizeof(T_Biquad_Optim_SP_Coefs)>>2)*9;
	check_add(core, &mod_info);

	// STA_SetFilterNumBands
	mod_info.stamod = STA_SetFilterNumBands(mod_info.stamod, 19);
	mod_upd = mod_info;
	mod_upd.mod_cycles = SPECMTR_NBANDS_1BIQ_MAIN_CYCLES + SPECMTR_NBANDS_1BIQ_PER_BAND_CYCLES*19 + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_upd.mod_xmem   = ((sizeof(T_Biquad_Optim_SP_Chained_Data)>>2)+3)*19;
	mod_upd.mod_ymem   = (sizeof(T_SpectrumMeter_Coefs)>>2)+(sizeof(T_Biquad_Optim_SP_Coefs)>>2)*19;
	check_update(core, &mod_info, &mod_upd);

	//del module
	check_delete(core, &mod_upd);

	//---- STA_MIXE_7INS_NCH --------------------------------------------

	//add module
	mod_info.mod_type   = STA_MIXE_7INS_NCH;  //2 ch by default
	mod_info.mod_cycles = MIXER3_MAIN_CYCLES + MIXER3_PER_CH_CYCLES*2 + MIXER3_PER_INPUT_PER_CH_CYCLES*7*2 + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = (sizeof(T_Mixer3_NchData)>>2)+2*7;
	mod_info.mod_ymem   = (sizeof(T_Mixer3_7inNchParams)>>2);
	check_add(core, &mod_info);

	// STA_SetNumChannels
	mod_info.stamod = STA_SetNumChannels(mod_info.stamod, 3);
	mod_upd = mod_info;
	mod_upd.mod_cycles = MIXER3_MAIN_CYCLES + MIXER3_PER_CH_CYCLES*3 + MIXER3_PER_INPUT_PER_CH_CYCLES*7*3 + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_upd.mod_xmem   = (sizeof(T_Mixer3_NchData)>>2)+2*7;
	check_update(core, &mod_info, &mod_upd);

	//del module
	check_delete(core, &mod_upd);

	//---- STA_CLIPLMTR_NCH --------------------------------------------

	//add module
	mod_info.mod_type   = STA_CLIPLMTR_NCH;  //2 ch by default
	mod_info.mod_cycles = CLIP_LIMITER_NCH_MAIN_CYCLES + CLIP_LIMITER_NCH_PER_CH_CYCLES*2 + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = (sizeof(T_ClipLimiterData)>>2)+2*2+1;
	mod_info.mod_ymem   = (sizeof(T_ClipLimiterNchParams)>>2);
	check_add(core, &mod_info);

	// STA_SetNumChannels
	mod_info.stamod = STA_SetNumChannels(mod_info.stamod, 5);
	mod_upd = mod_info;
	mod_upd.mod_cycles = CLIP_LIMITER_NCH_MAIN_CYCLES + CLIP_LIMITER_NCH_PER_CH_CYCLES*5 + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_upd.mod_xmem   = (sizeof(T_ClipLimiterData)>>2)+2*5+1;
	check_update(core, &mod_info, &mod_upd);

	//del module
	check_delete(core, &mod_upd);

	//---- STA_EQUA_STATIC_12BANDS_DP --------------------------------------------

	//add module
	mod_info.mod_type   = STA_EQUA_STATIC_12BANDS_DP;  //2 ch by default
	mod_info.mod_cycles = EQ_DP_MAIN_CYCLES + EQ_DP_PER_CH_CYCLES*2 + EQ_DP_PER_BAND_PER_CH_CYCLES*2*12 + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = (sizeof(T_Eq_12bands_DP_Data)>>2)*2;
	mod_info.mod_ymem   = (sizeof(T_Eq_12bands_Coefs)>>2)*2;
	check_add(core, &mod_info);

	// STA_SetNumChannels
	mod_info.stamod = STA_SetNumChannels(mod_info.stamod, 4);
	mod_upd = mod_info;
	mod_upd.mod_cycles = EQ_DP_MAIN_CYCLES + EQ_DP_PER_CH_CYCLES*4 + EQ_DP_PER_BAND_PER_CH_CYCLES*4*12 + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_upd.mod_xmem   = (sizeof(T_Eq_12bands_DP_Data)>>2)*4;
	check_update(core, &mod_info, &mod_upd);

	//del module
	check_delete(core, &mod_upd);

	//---- STA_EQUA_STATIC_16BANDS_SP --------------------------------------------

	//add module
	mod_info.mod_type   = STA_EQUA_STATIC_16BANDS_SP;  //2 ch by default
	mod_info.mod_cycles = EQ_SP_MAIN_CYCLES + EQ_SP_PER_CH_CYCLES*2 + EQ_SP_PER_BAND_PER_CH_CYCLES*2*16 + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = (sizeof(T_Eq_16bands_SP_Data)>>2)*2;
	mod_info.mod_ymem   = (sizeof(T_Eq_16bands_SP_Coefs)>>2)*2;
	check_add(core, &mod_info);

	// STA_SetNumChannels
	mod_info.stamod = STA_SetNumChannels(mod_info.stamod, 4);
	mod_upd = mod_info;
	mod_upd.mod_cycles = EQ_SP_MAIN_CYCLES + EQ_SP_PER_CH_CYCLES*4 + EQ_SP_PER_BAND_PER_CH_CYCLES*4*16 + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_upd.mod_xmem   = (sizeof(T_Eq_16bands_SP_Data)>>2)*4;
	check_update(core, &mod_info, &mod_upd);

	//del module
	check_delete(core, &mod_upd);

	//---- STA_PEAKDETECT_NCH --------------------------------------------

	//add module
	mod_info.mod_type   = STA_PEAKDETECT_NCH;  //2 ch by default
	mod_info.mod_cycles = PEAKDETECT_NCH_MAIN_CYCLES + PEAKDETECT_NCH_PER_CH_CYCLES*2 + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = (sizeof(T_PeakDetectData)>>2)+2;
	mod_info.mod_ymem   = (sizeof(T_PeakDetectNchParams)>>2)+2+1;
	check_add(core, &mod_info);

	// STA_SetNumChannels
	mod_info.stamod = STA_SetNumChannels(mod_info.stamod, 6);
	mod_upd = mod_info;
	mod_upd.mod_cycles = PEAKDETECT_NCH_MAIN_CYCLES + PEAKDETECT_NCH_PER_CH_CYCLES*6 + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_upd.mod_xmem   = (sizeof(T_PeakDetectData)>>2)+6;
	mod_upd.mod_ymem   = (sizeof(T_PeakDetectNchParams)>>2)+6+1;
	check_update(core, &mod_info, &mod_upd);

	//del module
	check_delete(core, &mod_upd);

	//---- STA_BITSHIFTER_NCH --------------------------------------------

	//add module
	mod_info.mod_type   = STA_BITSHIFTER_NCH;  //2 ch by default
	mod_info.mod_cycles = BITSHIFTER_NCH_MAIN_CYCLES + BITSHIFTER_NCH_PER_CH_CYCLES*2 + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = (sizeof(T_BitShiftData)>>2)+2*2;
	mod_info.mod_ymem   = (sizeof(T_BitShiftNchParams)>>2)+1+2;
	check_add(core, &mod_info);

	// STA_SetNumChannels
	mod_info.stamod = STA_SetNumChannels(mod_info.stamod, 4);
	mod_upd = mod_info;
	mod_upd.mod_cycles = BITSHIFTER_NCH_MAIN_CYCLES + BITSHIFTER_NCH_PER_CH_CYCLES*4 + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_upd.mod_xmem   = (sizeof(T_BitShiftData)>>2)+2*4;
	mod_upd.mod_ymem   = (sizeof(T_BitShiftNchParams)>>2)+1+4;
	check_update(core, &mod_info, &mod_upd);

	//del module
	check_delete(core, &mod_upd);

	//---- STA_GAIN_STATIC_NCH --------------------------------------------

	//add module
	mod_info.mod_type   = STA_GAIN_STATIC_NCH;  //2 ch by default
	mod_info.mod_cycles = GAIN_STATIC_NCH_MAIN_CYCLES + GAIN_STATIC_NCH_PER_STEREO_CH_CYCLES*1 + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = (sizeof(T_GainData)>>2)+2*2;
	mod_info.mod_ymem   = (sizeof(T_GainSimpleNchParams)>>2)+2*2;
	check_add(core, &mod_info);

	// STA_SetNumChannels
	mod_info.stamod = STA_SetNumChannels(mod_info.stamod, 4);
	mod_upd = mod_info;
	mod_upd.mod_cycles = GAIN_STATIC_NCH_MAIN_CYCLES + GAIN_STATIC_NCH_PER_STEREO_CH_CYCLES*2 + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_upd.mod_xmem   = (sizeof(T_GainData)>>2)+2*4;
	mod_upd.mod_ymem   = (sizeof(T_GainSimpleNchParams)>>2)+2*4;
	check_update(core, &mod_info, &mod_upd);

	//del module
	check_delete(core, &mod_upd);

	//---- STA_GAIN_SMOOTH_NCH --------------------------------------------

	//add module
	mod_info.mod_type   = STA_GAIN_SMOOTH_NCH;  //2 ch by default
	mod_info.mod_cycles = GAIN_SMOOTH_NCH_MAIN_CYCLES + GAIN_SMOOTH_NCH_PER_CH_CYCLES*2 + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = (sizeof(T_GainData)>>2)+2*2;
	mod_info.mod_ymem   = (sizeof(T_GainSmoothNchParams)>>2)+(sizeof(T_GainSmooth1chParams)>>2)*2;
	check_add(core, &mod_info);

	// STA_SetNumChannels
	mod_info.stamod = STA_SetNumChannels(mod_info.stamod, 6);
	mod_upd = mod_info;
	mod_upd.mod_cycles = GAIN_SMOOTH_NCH_MAIN_CYCLES + GAIN_SMOOTH_NCH_PER_CH_CYCLES*6 + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_upd.mod_xmem   = (sizeof(T_GainData)>>2)+2*6;
	mod_upd.mod_ymem   = (sizeof(T_GainSmoothNchParams)>>2)+(sizeof(T_GainSmooth1chParams)>>2)*6;
	check_update(core, &mod_info, &mod_upd);

	//del module
	check_delete(core, &mod_upd);

	//---- STA_LMTR_1CH --------------------------------------------

	mod_info.mod_type   = STA_LMTR_1CH;
	mod_info.mod_cycles = LIMITER_1CH_MAIN_CYCLES + LIMITER_UPDATE_CYCLES + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = (sizeof(T_Limiter2MonoData)>>2)+2;
	mod_info.mod_ymem   = (sizeof(T_Limiter2Coeffs)>>2)+1*80;
	check_add(core, &mod_info);
	check_delete(core, &mod_info);

	//---- STA_LMTR_2CH --------------------------------------------

	mod_info.mod_type   = STA_LMTR_2CH;
	mod_info.mod_cycles = LIMITER_2CH_MAIN_CYCLES + LIMITER_UPDATE_CYCLES + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = (sizeof(T_Limiter2StereoData)>>2)+4;
	mod_info.mod_ymem   = (sizeof(T_Limiter2Coeffs)>>2)+2*80;
	check_add(core, &mod_info);
	check_delete(core, &mod_info);

	//---- STA_LMTR_4CH --------------------------------------------

	mod_info.mod_type   = STA_LMTR_4CH;
	mod_info.mod_cycles = LIMITER_4CH_MAIN_CYCLES + LIMITER_UPDATE_CYCLES + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = (sizeof(T_Limiter2QuadroData)>>2)+8;
	mod_info.mod_ymem   = (sizeof(T_Limiter2Coeffs)>>2)+4*80;
	check_add(core, &mod_info);
	check_delete(core, &mod_info);

	//---- STA_LMTR_6CH --------------------------------------------

	mod_info.mod_type   = STA_LMTR_6CH;
	mod_info.mod_cycles = LIMITER_6CH_MAIN_CYCLES + LIMITER_UPDATE_CYCLES + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = (sizeof(T_Limiter2SixChData)>>2)+12;
	mod_info.mod_ymem   = (sizeof(T_Limiter2Coeffs)>>2)+6*80;
	check_add(core, &mod_info);
//	check_delete(core, &mod_info);
	//note: keep the limiter 6ch in the buildflow to test the STA_SetUpdateSlot() later....
	g_limiter6ch = mod_info.stamod;

	//---- STA_COMP_1CH --------------------------------------------

	mod_info.mod_type   = STA_COMP_1CH;
	mod_info.mod_cycles = COMPANDER_1CH_MAIN_CYCLES + COMPANDER_UPDATE_CYCLES + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = (sizeof(T_CompanderMonoData)>>2)+2;
	mod_info.mod_ymem   = (sizeof(T_CompanderCoeffs)>>2)+1*80;
	check_add(core, &mod_info);
	check_delete(core, &mod_info);

	//---- STA_COMP_2CH --------------------------------------------

	mod_info.mod_type   = STA_COMP_2CH;
	mod_info.mod_cycles = COMPANDER_2CH_MAIN_CYCLES + COMPANDER_UPDATE_CYCLES + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = (sizeof(T_CompanderStereoData)>>2)+4;
	mod_info.mod_ymem   = (sizeof(T_CompanderCoeffs)>>2)+2*80;
	check_add(core, &mod_info);
	check_delete(core, &mod_info);

	//---- STA_COMP_4CH --------------------------------------------

	mod_info.mod_type   = STA_COMP_4CH;
	mod_info.mod_cycles = COMPANDER_4CH_MAIN_CYCLES + COMPANDER_UPDATE_CYCLES + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = (sizeof(T_CompanderQuadroData)>>2)+8;
	mod_info.mod_ymem   = (sizeof(T_CompanderCoeffs)>>2)+4*80;
	check_add(core, &mod_info);
	check_delete(core, &mod_info);

	//---- STA_COMP_6CH --------------------------------------------

	mod_info.mod_type   = STA_COMP_6CH;
	mod_info.mod_cycles = COMPANDER_6CH_MAIN_CYCLES + COMPANDER_UPDATE_CYCLES + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = (sizeof(T_Compander6ChData)>>2)+12;
	mod_info.mod_ymem   = (sizeof(T_CompanderCoeffs)>>2)+6*80;
	check_add(core, &mod_info);
//	check_delete(core, &mod_info);
	//note: keep the compander 6ch in the buildflow to test the STA_SetUpdateSlot() later....
	g_compander6ch = mod_info.stamod;



	//Test set update slots
	//assuming 1 limiter6ch + 1 compander6ch in the buildflow
	STA_SetNumOfUpdateSlots(core, 6);

	STA_SetUpdateSlot(g_limiter6ch, 3);
	g_dsp_estimated_cycles -= COMPANDER_UPDATE_CYCLES;
	check_stats(core, NULL);

	STA_SetUpdateSlot(g_limiter6ch, 0);
	g_dsp_estimated_cycles += COMPANDER_UPDATE_CYCLES;
	check_stats(core, NULL);

	STA_SetUpdateSlot(g_compander6ch, 3);
	g_dsp_estimated_cycles -= COMPANDER_UPDATE_CYCLES;
	check_stats(core, NULL);



	//---- STA_GAIN_LINEAR_NCH --------------------------------------------

	//add module
	mod_info.mod_type   = STA_GAIN_LINEAR_NCH;  //2 ch by default
	mod_info.mod_cycles = GAIN_LINEAR_NCH_MAIN_CYCLES + GAIN_LINEAR_NCH_PER_CH_CYCLES*2 + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_info.mod_xmem   = (sizeof(T_GainData)>>2)+2*2;
	mod_info.mod_ymem   = (sizeof(T_GainSmoothNchParams)>>2)+(sizeof(T_GainSmooth1chParams)>>2)*2;
	check_add(core, &mod_info);

	// STA_SetNumChannels
	mod_info.stamod = STA_SetNumChannels(mod_info.stamod, 100);
	mod_upd = mod_info;
	mod_upd.mod_cycles = GAIN_LINEAR_NCH_MAIN_CYCLES + GAIN_LINEAR_NCH_PER_CH_CYCLES*100 + MAIN_LOOP_PER_MODULE_CYCLES;
	mod_upd.mod_xmem   = (sizeof(T_GainData)>>2)+2*100;
	mod_upd.mod_ymem   = (sizeof(T_GainSmoothNchParams)>>2)+(sizeof(T_GainSmooth1chParams)>>2)*100;
	check_update(core, &mod_info, &mod_upd);

	//del module
//	check_delete(core, &mod_upd);

	//keep using the gain module to test the cycles and dsp mem cost for connections
	g_gain = mod_upd.stamod;

#if 0
    //connexion speed
   	s64 time0 = xTaskGetTickCount();
    FORj(100)
    {
        FORi(50)
        {
            STA_Connect(xin, i, g_gain, i);
            STA_Connect(g_gain, i, xout, i);
        }
        FORi(50)
        {
            STA_Disconnect(xin, i, g_gain, i);
            STA_Disconnect(g_gain, i, xout, i);
        }
    }
   	s64 time1 = xTaskGetTickCount();
	time1 -= time0;
	PRINTF("100 connect+disconnect time=%d ms (~280) ", (int)time1);
#endif

	//add connections
	STA_Connect(xin, 0, g_gain, 0);
	STA_Connect(xin, 1, g_gain, 1);
	STA_Connect(g_gain, 0, xout, 0);
	STA_Connect(g_gain, 1, xout, 1);
	g_dsp_estimated_cycles += PER_MONO_CONNECTION_CYCLES*4;
	g_dsp_estimated_ymem += 2*4;
	check_stats(core, NULL);

	//del connections
	STA_Disconnect(xin, 1, g_gain, 1);
	g_dsp_estimated_cycles -= PER_MONO_CONNECTION_CYCLES;
	check_stats(core, NULL);

	//add connections
	STA_Connect(xin, 1, g_gain, 1);
	g_dsp_estimated_cycles += PER_MONO_CONNECTION_CYCLES;
	check_stats(core, NULL);

	//build
	STA_BuildFlow();

	sta_err = STA_GetError();
	CHECK(sta_err==STA_NO_ERROR);

	//post build check
	check_stats(core, NULL);
}
//---------------------------------------------------------
void TestDspCycles(void)
{
	u32 cycles, i;
	STA_ErrorCode sta_err;

	PRINTF("TestDspCycles... ");


	//test with buildflow consecutively on the 3 dsps
	FORi(3)
	{
		STA_Reset();

		BuildFlow(i);

		//start
		STA_StartDSP(i);
		//STA_Play();				//DSPStart + DMABStart
		SLEEP(1);					//wait 1ms for DSP initialisation


		//stop and clean
		STA_StopDSP(i);
		STA_Reset();
	}

	PRINT_OK_FAILED;
}


#endif //0
