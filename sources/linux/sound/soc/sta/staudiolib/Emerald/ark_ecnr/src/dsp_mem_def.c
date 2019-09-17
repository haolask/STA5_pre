/*
 * dsp_mem_def.c
 *
 * ACCORDO2 STAudio Programmable Audio lib for EMERALD DSP
 *
 * Created on 2013/05/14 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * Memory mapping of P|X|Y RAM used for the programmable audio flow, running on Emerald DSP
 *
 * CODE FOR EMERALD DSP
 *
 */


//#include "memory.config"        // XRAM_SIZE, XIN_SIZE, XOUT_SIZE...
//#include "audio.h"              
#include "dsp_mem_def.h"

//misc module
#define DCODETECT          1

//THE ONLY FIXED PLACEMENTS
//note: disable fixed placment before to generate .msz file (project opt -> code generation -> increase mem)
#if 1
#pragma absolute     g_xmem_pool @ DSP_XMEM_POOL_OFFSET
#pragma absolute     g_ymem_pool @ DSP_YMEM_POOL_OFFSET
#pragma absolute     g_axi       @ DSP_AXI_OFFSET
#pragma absolute     g_ayi       @ DSP_AYI_OFFSET
#endif


//default filter
/*
extern T_Filter   _YMEM   g_passthrough_filters[];
extern T_Filter   _YMEM   g_passthrough_inits[];
extern T_Filter   _YMEM  *g_passthrough_updates[];
extern fraction * _YMEM   g_passthrough_transfers[];
*/
extern T_FilterFuncs _YMEM g_user_func_table[];


//----------------------------------------------------------------------
//  X RAM 
//----------------------------------------------------------------------

unsigned int         g_xmem_pool[DSP_XMEM_POOL_WSIZE] = {0};

T_ARM_XMEM_Interface g_axi = {
    DSP_XMEM_LOADED_CODE, //chk_xmem_loaded
    0,                    //chk_dsp_running = 0xF00D
	0,                    //isInitsDone
    0,                    //frame_counter
    g_xmem_pool           //pXmem_pool
};

//----------------------------------------------------------------------
//  Y RAM 
//----------------------------------------------------------------------

unsigned int _YMEM   g_ymem_pool[DSP_YMEM_POOL_WSIZE] = {0};

//##################################################################
//IMPORTANT: The list below MUST match the enum T_AudioFuncType 
//##################################################################
//Auto placement
T_FilterFuncs _YMEM g_alib_func_table[];
/* = {
    //AudioFuncs                    InitFuncs                         UpdateFuncs
    { exit_loop,                    exit_loop_incSlot,                exit_loop_resetSlot },   //keep first
    { mux_2out,                     NULL,                             NULL },
    { mux_4out,                     NULL,                             NULL },
    { mux_6out,                     NULL,                             NULL },
    { mux_8out,                     NULL,                             NULL },
    { mux_10out,                    NULL,                             NULL },
    { audio_cd_deemphasis,          audio_cd_deemphasis_init,          NULL },
    { audio_gain_simple_2Nch,       audio_gain_simple_Nch_init,        NULL },
    { audio_gain_smooth_Nch,        NULL,                             NULL },
    { loudness_static_dp,           loudness_static_dp_init,           NULL },
    { loudness_static_dual_dp,      loudness_static_dual_dp_init,      NULL },
    { tone_static_dp,               tone_static_dp_init,               NULL },
    { tone_static_dual_dp,          tone_static_dual_dp_init,          NULL },
    { eq_static_3bands_Nch_dp,      eq_static_Nbands_Nch_dp_init,      NULL }, //3-band EQ
    { eq_static_4bands_Nch_dp,      eq_static_Nbands_Nch_dp_init,      NULL }, //4-band EQ
    { eq_static_5bands_Nch_dp,      eq_static_Nbands_Nch_dp_init,      NULL }, //5-band EQ
    { eq_static_6bands_Nch_dp,      eq_static_Nbands_Nch_dp_init,      NULL }, //6-band EQ
    { eq_static_7bands_Nch_dp,      eq_static_Nbands_Nch_dp_init,      NULL }, //7-band EQ
    { eq_static_8bands_Nch_dp,      eq_static_Nbands_Nch_dp_init,      NULL }, //8-band EQ
    { eq_static_9bands_Nch_dp,      eq_static_Nbands_Nch_dp_init,      NULL }, //9-band EQ
    { eq_static_10bands_Nch_dp,     eq_static_Nbands_Nch_dp_init,      NULL }, //10-band EQ
    { audio_mixer_2ins_Nch,         audio_mixer_2ins_Nch_init,         NULL }, //audio_mixer_2ins_gain },
    { audio_mixer_3ins_Nch,         audio_mixer_3ins_Nch_init,         NULL }, //audio_mixer_3ins_gain },
    { audio_mixer_4ins_Nch,         audio_mixer_4ins_Nch_init,         NULL }, //audio_mixer_4ins_gain },
    { audio_mixer_5ins_Nch,         audio_mixer_5ins_Nch_init,         NULL }, //audio_mixer_5ins_gain },
    { audio_mixer_6ins_Nch,         audio_mixer_6ins_Nch_init,         NULL }, //audio_mixer_6ins_gain },
    { audio_mixer_7ins_Nch,         audio_mixer_7ins_Nch_init,         NULL }, //audio_mixer_7ins_gain },
    { audio_mixer_8ins_Nch,         audio_mixer_8ins_Nch_init,         NULL }, //audio_mixer_8ins_gain },    
    { audio_compander_la_mono,      compander_mono_init,               NULL },
    { audio_compander_la_stereo,    compander_stereo_init,             NULL },
    { audio_compander_la_quadro,    compander_quadro_init,             NULL },
    { audio_compander_la_6ch,       compander_6ch_init,                NULL },
    { audio_limiter_la_mono,        NULL,                             audio_limiter_update },
    { audio_limiter_la_stereo,      NULL,                             audio_limiter_update },
    { audio_limiter_la_quadro,      NULL,                             audio_limiter_update },
    { audio_limiter_la_6ch,         NULL,                             audio_limiter_update },
    { audio_delay_line_y_2ch,       NULL,                             NULL },
    { audio_delay_line_y_4ch,       NULL,                             NULL },
    { audio_delay_line_y_6ch,       NULL,                             NULL },
    { audio_delay_line_x_2ch,       NULL,                             NULL },
    { audio_delay_line_x_4ch,       NULL,                             NULL },
    { audio_delay_line_x_6ch,       NULL,                             NULL },
    { audio_sinewave,               NULL,                             NULL },
#if DCODETECT
    { dco_detect_1ch,               dco_detect_1ch_init,               NULL },
    { dco_detect_4ch_monozcf,       dco_detect_4ch_init,               NULL },
#else
    { NULL,                        NULL,                             NULL },
    { NULL,                        NULL,                             NULL },
#endif    

};
*/


T_ARM_YMEM_Interface _YMEM g_ayi = {
    DSP_ALIB_VERSION,    
    DSP_YMEM_LOADED_CODE,   //chk_ymem_loaded
    g_alib_func_table,      //pAlib_func_table
    NULL,                   //pUser_func_table
    NULL,                  //pFunc_table
    NULL,                  //pInit_table
    NULL,                  //pUpdate_table
    NULL,                 //pTransfer_table
    g_ymem_pool            //pYmem_pool
};

