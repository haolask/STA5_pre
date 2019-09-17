/*
 * STAudioLib - test_miniplayer_dsp_preset.c
 *
 * Created on 2016/04/27 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */


#if 1
#include "staudio_test.h"

#include "miniplayer.h"
#include "preset_M5.h"
#include "dsp_controls.h"



//fwd decl
static void key_print(void);
static void rtx_setSel(u32 sel);


//----------
// SONG
//----------
extern RAWINFO g_song1;
extern RAWINFO g_song2;

static RAWINFO* g_song = &g_song1;

static MSP_TypeDef *g_MSP = (MSP_TypeDef*) MSP2_BASE;

//----------
// REC
//----------
//	RAWINFO g_recInfo = {"0:/record_48k_16bit_stereo.pcm",		48000,   16, 2, (void*)0, 0};
	RAWINFO g_recInfo = {"0:/record_48k_3224bit_stereo.pcm",	48000, 3224, 2, (void*)0, 0};

//#define REC_DMA_ADDR		STA_DMA_SRC0_DO
#define REC_DMA_ADDR		STA_DMA_SAI2_RX1
#define REC_WRITE_TO_FILE	1

//-----------
// DSP PRESET
//-----------
#define BYPASS_DSP			0
#define DSP_SEL				0

#define STAMOD(modId)		STA_GetSTAModule(g_dsp_preset, modId)

static STAPreset* g_dsp_preset;

//------------
// VOLUME
//------------
static const VOLUMES g_volume_init = {
//	.dbGains	= {-120, -120, -120, -120},	 //  FL, FR, RL, RR
	.dbGains	= {  0, 0, 0, 0},	 //  FL, FR, RL, RR
	.dbStep		=   60,	 // +-6dB
	.tup		=  100,  //msec
	.tdown		=  100,  //msec
};

static VOLUMES g_volume;

//------------
// SINEWAVE
//------------
static const SINEWAVEGEN g_sine_init = {
	.ampl		= 1000,				//0.2
	.freq		= 100*100,			//Hz
	.msec		= -1,  				//ms (infinite)
	.param		= SINE_DB,
	.steps		= {100, 0, 100, 100}	//F, FLOG, A, DB/10
};

static SINEWAVEGEN g_sine;

//------------
// CHIME GEN
//------------
static const CHIMEGEN g_chimeGen_init = {
	.type		= CHIME_CHIME,
//	.staParams	= ;		//<-- set dynamically when calling chime_toggleOnOff()
};

static CHIMEGEN g_chimeGen1;


//------------------------
// CLICK-CLACK (PCMCHIMES)
//------------------------
static RAWINFO g_clickClackInfo = {"0:/A2_SONGS/click-clack_16bit_48k.wav", 48000, 16, 2, (void*)0, 0};
//static RAWINFO g_clickClackInfo = {"0:/A2_SONGS/Click_Clack_Chime_33_48kHz_D3_2x16x3288.raw", 48000, 16, 2, (void*)0, 0};

static STA_PCMChimeParams	g_clickClackParams = {
	.playCount 		= -1,		// < 0 means infinit loop
	.leftDelay 		= 0,		//msec
	.leftPostDelay	= 0,		//msec
	.rightDelay		= 500,		//msec  <-- RTX
	.rightPostDelay = 500,		//msec	<-/
};

static const CHIMEGEN g_clickClack_init = {
	.type		= CHIME_PCM,
//	.staParams	= &g_clickClackParams;
};

static CHIMEGEN g_clickClack;


//------------
// SPECMETER
//------------
#if 1
static u16 g_specmeter_freqs[] = {
//	 1,  2,  3,  4,  5,  6,   7,   8,   9,  10,   11,   12,   13,   14,    15,    16
	30, 40, 50, 60, 100, 200, 300, 400, 500, 800, 1000, 2000, 4000, 5000, 10000, 20000
};
#endif

static const SPECMETER g_specmeter_init = {
	.numBands 		= 16,
	.numBiqPerBands = 1,
//	.freqs			= g_specmeter_freqs,	//optional
	.decayMsec		= 100,			//msec
	.rateMsec		= 100,			//msec
	.dBscale		= (1<<8),		//compatible with Ryo SpeVisualizer
	.QFactor		= 100,
	.on				= 0,
	.param			= SPE_Q,
	.steps			= {10, 10, 100}		//decay, rate, Q
};

static SPECMETER g_specmeter;

//------------
// SRC_EQ
//------------
#define SRC_EQ_NUM_BANDS  7

#if 1
static u16 g_eq_freqs[] = {
//	 1,  2,  3,   4,   5,   6,    7,    8,    9,    10,    11
	20, 30, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000
};
#endif

/*
//BAND g_eq_bands[SRC_EQ_NUM_BANDS];
u16 g_eq_freqs[SRC_EQ_NUM_BANDS];
s16 g_eq_gains[SRC_EQ_NUM_BANDS];
u16 g_eq_quals[SRC_EQ_NUM_BANDS];
u16 g_eq_types[SRC_EQ_NUM_BANDS];
*/
static const EQ g_src_eq_init = {
	.numBands		= SRC_EQ_NUM_BANDS,
	.ON				= 0,
	.sel			= EQ_SELMODE,
	.mode			= EQ_ROCK,
	.param			= STA_GAIN,
	.paramSteps		= {10, 100, 10, 1}, //G/10, Q, F, T steps
	.presetGains	= (s16*)g_eq_presetGains_7bands,
//	.freqs			= g_eq_freqs,		//[optional]
	.pSpecMeter 	= &g_specmeter,		//[optional]
//	.bands			= g_eq_bands,
//	.Freqs			= g_eq_freqs,
//	.Gains			= g_eq_gains,
//	.Quals			= g_eq_quals,
//	.Types			= g_eq_types,
};

static EQ g_src_eq;

//---------------------------------------------------------
// DMABUS
//---------------------------------------------------------
void DMABUS_Set(PLY_SRC src, int nch, int dsp)
{
	u16 dmaSrc;
//	u16 dmaDst	= STA_DMA_SAI3_TX1;
	u16 dmaXIN;
	u16 dmaXOUT;

	int bypassdsp = 0; //set if dsp != 0,1,2


	//DSP in/out
	switch (dsp)
	{
	case 0: dmaXIN  = STA_DMA_DSP0_XIN;		dmaXOUT = STA_DMA_DSP0_XOUT; break;
	case 1: dmaXIN  = STA_DMA_DSP1_XIN;		dmaXOUT = STA_DMA_DSP1_XOUT; break;
	case 2: dmaXIN  = STA_DMA_DSP2_XIN;		dmaXOUT = STA_DMA_DSP2_XOUT; break;

	default: bypassdsp = 1; break;
	}

	//SRC
	switch (src)
	{
	case PLY_MSP1:
	case PLY_MSP2:
		dmaSrc = STA_DMA_SRC0_DO; break;

	case PLY_ADCMIC_BUILTIN:
	case PLY_ADCMIC_EXT:
		dmaSrc = STA_DMA_SAI3_RX1; break;

	case PLY_ADCAUX_IN1:
	case PLY_ADCAUX_IN2:
		dmaSrc = STA_DMA_ADC_DATA; break;

	case PLY_FIFO2:
		dmaSrc = STA_DMA_FIFO_OUT; break;
	}

	STA_DMAReset();

	if (bypassdsp)
	{
		STA_DMAAddTransfer(dmaSrc,		STA_DMA_SAI3_TX1,  2); //DACO (Front)
		STA_DMAAddTransfer(dmaSrc,		STA_DMA_SAI3_TX2,  2); //DAC1 (Rear)
		STA_DMAAddTransfer(dmaSrc,		STA_DMA_SAI3_TX3,  2); //DAC2 (Central, SubW)
	}
	else
	{
		//DSP IO
		STA_DMAAddTransfer(dmaSrc,		dmaXIN,  nch);
		STA_DMAAddTransfer(dmaXOUT,		STA_DMA_SAI3_TX1,  6); //DACO (Front)
//		STA_DMAAddTransfer(dmaXOUT,		STA_DMA_SAI3_TX2,  2); //DAC1 (Rear)
	//	STA_DMAAddTransfer(dmaXOUT,		STA_DMA_SAI3_TX3,  2); //DAC2 (Central, SubW)
		//DSP IO swap
		STA_DMAAddTransfer(dmaSrc, 		dmaXIN+127, 1);
	}

	//CLICK-CLACK (DSP1 -> DSP0)
	STA_DMAAddTransfer(STA_DMA_DSP1_XOUT+0,	STA_DMA_DSP0_XIN+4,  2);

	//DSP1 swap
	STA_DMAAddTransfer(dmaSrc,			STA_DMA_DSP1_XIN+127,  1);

	//FIFO1 REC
	STA_DMAAddTransfer(REC_DMA_ADDR,	STA_DMA_FIFO_IN,  nch); //dump after DSP

	STA_DMAStart();
}

//---------------------------------------------------------
// FIFO1 Recording
//---------------------------------------------------------
static void toggleRecord(void)
{
	static int	s_record = 0;
	RAWINFO* rec 	= &g_recInfo;
	u32 numSamples	= -1; //max

	s_record ^= 1;

	if (s_record)
	{
		PRINTF("Recording...\n");
		PLY_start_dump_FIFO1(rec->data, numSamples, rec->nch); //max numsamples
	}
	else
	{
		PRINTF("Stop recording\n");
		PLY_stop_dump_FIFO1();

		rec->byteSize = g_player.fifo1.m_numBytes;

		switch (rec->bits) {
		case 16:
			PRINTF("converting to 16 bits... ");
			Convert_32to16(rec->data, rec->byteSize, rec->data, &rec->byteSize);
			PRINTF("done\n");
			break;
		case 24:
			PRINTF("converting to 24 bits... ");
			Convert_32to24(rec->data, rec->byteSize, rec->data, &rec->byteSize);
			PRINTF("done\n");
			break;
		default:
			rec->bits = 3224;
			PRINTF("(recorded as 32bit_24valid)\n");
		}
#if REC_WRITE_TO_FILE
		PRINTF("Copying to SD: %s... ", rec->filename);
		WriteFile(rec->filename, &rec->byteSize, (u32*)rec->data);
		PRINTF("done\n");
#endif
	}

	//then retrieve the dump with J-Link Commander:
	//J-Link> savebin <filename>,  <addr>     <numBytes>
	//J-Link> savebin savebin.raw, 0x61E00010, 0x5000
}

//---------------------------------------------------------
// play MSP
//---------------------------------------------------------
static void MSP_play(MSP_TypeDef *MSP, RAWINFO* song)
{
	PLY_SetState_MSP(MSP, PLY_STOP);

	PRINTF("Playing %s\n", song->filename);

	PRS_SetInMuxOutChannel(0, 0);
	PRS_SetInMuxOutChannel(1, 1);

	DMABUS_Set(MSP == MSP1 ? PLY_MSP1 : PLY_MSP2, song->nch, DSP_SEL);

	PLY_Set_pcm_source_MSP(MSP, song, PLY_NOFLAG);
	PLY_SetState_MSP(MSP, PLY_PLAY);
}

//---------------------------------------------------------
// toggle MSP: MSP2, MSP2
//---------------------------------------------------------
static void toggleMSP(void)
{
	PLY_SetState_MSP(g_MSP, PLY_STOP);

	g_MSP = (g_MSP == MSP1) ? MSP2 : MSP1;

	PRINTF((g_MSP == MSP1) ? "MSP1\n" : "MSP2\n");

	MSP_play(g_MSP, g_song);
}
//---------------------------------------------------------
// toggle Song: g_song1, g_song2
//---------------------------------------------------------
static void toggleSong(void)
{
	PLY_SetState_MSP(g_MSP, PLY_STOP);

	g_song = (g_song == &g_song1) ? &g_song2 : &g_song1;

	MSP_play(g_MSP, g_song);
}

//---------------------------------------------------------
// toggle Rec Source: FILE(MSP2), REC(MSP2)
//---------------------------------------------------------
static void toggleSourceRec(void)
{
	static int s_play_rec = 0;

	PLY_SetState_MSP(g_MSP, PLY_STOP);

	s_play_rec ^= 1;

	//play REC
	if (s_play_rec)
	{
		PRINTF("Playing REC\n");

		DMABUS_Set(g_MSP == MSP1 ? PLY_MSP1 : PLY_MSP2, g_recInfo.nch, DSP_SEL);

		PLY_Set_pcm_source_MSP(g_MSP, &g_recInfo, PLY_NOFLAG);
		PLY_SetState_MSP(g_MSP, PLY_PLAY);

	}
	//play SONG
	else
	{
		MSP_play(g_MSP, g_song);
	}
}

//---------------------------------------------------------
// ADC AUX
//---------------------------------------------------------
static void toggleADCAUX(int chsel)
{
	PLY_SetState_MSP(g_MSP, PLY_STOP);

	PRINTF("ADCAUX_IN%d\n", chsel+1);

	ADCAUX_Init(chsel);

	DMABUS_Set((chsel == ADCAUX_IN1) ? PLY_ADCAUX_IN1 : PLY_ADCAUX_IN2, 2, DSP_SEL);
}

//---------------------------------------------------------
// ADC MIC
//---------------------------------------------------------
static void toggleADCMIC(int chsel)
{
	PLY_SetState_MSP(g_MSP, PLY_STOP);

	PRINTF("ADCMIC %s\n", (chsel == ADCMIC_EXTMIC) ? "(ext)" : "(builtin)");

	DMABUS_Set((chsel == ADCMIC_EXTMIC) ? PLY_ADCMIC_EXT : PLY_ADCMIC_BUILTIN, 2, DSP_SEL);
}

//---------------------------------------------------------
// KEY
//---------------------------------------------------------
enum {
	KEY_SEL_TOP,	//VOL
	KEY_SEL_SRC,
	KEY_SEL_CHIME,
	KEY_SEL_SINE,
	KEY_SEL_SRC_EQ,
	KEY_SEL_SPECMETER,
	KEY_SEL_LAST
};

enum {
	RTX_SEL_VOL,
	RTX_SEL_PARAM,	//PARAM depends on the selected effect
};

const char* g_key_sel_names[] = {"TOP", "SRC", "CHIME", "SINE", "SRC_EQ", "SPECMTR"};

static u32 g_key_sel     = KEY_SEL_TOP;
static int g_key_pressed = 0;

static u32 g_rtx_sel     = RTX_SEL_VOL;
static int g_rtx_steps   = 0;

static int g_exit		 = 0;


static void key_setSel(u32 sel)
{
	g_key_sel = sel;

	key_print();

	//print additional current selection info (and adapt RTX_SEL)
	switch (g_key_sel)
	{
	case  KEY_SEL_TOP:
		g_rtx_sel = RTX_SEL_VOL;
		vol_print(&g_volume);
		break;

	case  KEY_SEL_SINE:
		g_rtx_sel = RTX_SEL_PARAM;
		sine_print(&g_sine);
		break;

	case  KEY_SEL_SRC_EQ:
		g_rtx_sel = RTX_SEL_PARAM;
		eq_print(&g_src_eq);
		eq_showInSpecVisualizer(&g_src_eq, ON);
	 	 break;

	case  KEY_SEL_SPECMETER:
		g_rtx_sel = RTX_SEL_PARAM;
		eq_showInSpecVisualizer(&g_src_eq, OFF);
		specmeter_print(&g_specmeter);
		break;
	}

}

static void key_toggleSel()
{
	key_setSel((g_key_sel < KEY_SEL_LAST-1) ? g_key_sel + 1 : KEY_SEL_TOP);
}

static void key_callback(int key, bool down)
{
	g_key_pressed = key;
}

//hack for correct display on ST SpeViz's console
#define CR	PRINTF("\n");

static void key_print(void)
{
	PRINTF("\n___ %s _______________________________________________\n", g_key_sel_names[g_key_sel]); CR

	switch (g_key_sel)
	{
	case KEY_SEL_TOP:
	PRINTF(" [MUTE] [    ]          [KEYS]  [    ]  [SPEC]  [EXIT]\n"); CR
	PRINTF("                (VOL)   [VOL ]  [    ]  [REC ]  [STA ]\n"); CR
	PRINTF(" [PLAY] [STOP]          [SEL ]  [    ]  [SRC ]  [CYCL]\n\n");
	break;

	case KEY_SEL_SRC:
	PRINTF(" [MUTE] [    ]          [KEYS]  [    ]  [    ]  [    ]\n"); CR
	PRINTF("                (VOL)   [VOL ]  [    ]  [SINE]  [    ]\n"); CR
	PRINTF(" [PLAY] [STOP]          [SEL ]  [    ]  [MSP2]  [    ]\n\n");
	break;

	case KEY_SEL_CHIME:
	PRINTF(" [MUTE] [    ]          [KEYS]  [    ]  [    ]  [    ]\n"); CR
	PRINTF("                (VOL)   [VOL ]  [    ]  [CLCK]  [    ]\n"); CR
	PRINTF(" [PLAY] [STOP]          [SEL ]  [    ]  [GONG]  [BELT]\n\n");
	break;

	case KEY_SEL_SRC_EQ:
	PRINTF(" [MUTE] [    ]          [KEYS]  [    ]  [SPEC]  [ONOF]\n"); CR
	PRINTF("                (VOL)   [VOL ]  [    ]  [STP-]  [STP+]\n"); CR
	PRINTF(" [PLAY] [STOP]          [SEL ]  [    ]  [PAR-]  [PAR+]\n\n");
	break;

	case KEY_SEL_SINE:
	case KEY_SEL_SPECMETER:
	PRINTF(" [MUTE] [    ]          [KEYS]  [    ]  [    ]  [ONOF]\n"); CR
	PRINTF("                (VOL)   [VOL ]  [    ]  [STP-]  [STP+]\n"); CR
	PRINTF(" [PLAY] [STOP]          [SEL ]  [    ]  [PAR-]  [PAR+]\n\n");
	break;
	}

//	if (g_rtx_sel == RTX_SEL_VOL)
//		vol_print(&g_volume);
}

#undef CR

//Process the key/rtx in main task instead of callbacks
static void key_process(void)
{
	/*
	key mapping:
	[MUTE]       [xxxx]	    [GONG  ]  [BELT]  [    ]  [FreeCyCount]
							[SRC   ]  [    ]  [REC ]  [PlyRec]
	[PLAY/PAUS]  [STOP] 	[MSP1/2]  [    ]  [AUX2]  [MIC   ]
	*/

	int key = g_key_pressed;
	g_key_pressed = 0;  //key processed

	specmeter_toggleTimer(&g_specmeter, 0);

	//--------- always active keys ---------
	switch (key)
	{
	//MUTE
	case 0x11: vol_toggleMute(&g_volume); break;

	//PLAY/PAUSE
	case 0x13: PLY_TogglePlayPause_MSP(g_MSP); break;

	//STOP
	case 0x14: PLY_SetState_MSP(g_MSP, PLY_STOP); break;

	//PRINT KEYS
	case 0x21: key_print(); break;

	//VOLUME
	case 0x31: g_rtx_sel = RTX_SEL_VOL; vol_print(&g_volume); break;

	//SEL/TOP
	case 0x41: key_toggleSel(); break;
	}

	switch (g_key_sel) {
	//--------------------------------------------
	case KEY_SEL_TOP:

		switch (key)
		{
		//EXIT
		case 0x24: g_exit = 1; break;

		//FREE CYLE COUNT
		case 0x44: toggleFreeCycleCount(); break;

		//STA TRACE
		case 0x34: toggleSTATrace(); break;

		//SPECTRUM METER
		case 0x23: specmeter_toggleOnOff(&g_specmeter); break;

		//SRC
		case 0x43: key_setSel(KEY_SEL_SRC); break;

		//REC
		case 0x33: toggleRecord(); break;
		}
		break;
	//--------------------------------------------
	case KEY_SEL_SRC:

		switch (key)
		{
		//SRC
	//	case 0x31: toggleSong(); break;

	//	case 0x34: toggleSourceRec(); break;

		//MSP
	//	case 0x41: toggleMSP(); break;
		case 0x43: MSP_play(g_MSP, g_song); break;

		//ADCAUX
	//	case 0x43: toggleADCAUX(ADCAUX_IN2); break;

		//ADCMIC
		//case 0x44: toggleADCMIC(ADCMIC_EXTMIC); break;

		//SINEGEN
		case 0x33: sine_toggleOnOff(&g_sine); break;
		}
		break;
	//--------------------------------------------
	case KEY_SEL_CHIME:

		switch (key) {
		case 0x43: chime_toggleOnOff(&g_chimeGen1, &g_gong); break;
		case 0x44: chime_toggleOnOff(&g_chimeGen1, &g_beltminderB); break;
		case 0x33: chime_toggleOnOff(&g_clickClack, &g_clickClackParams); break;
		}
		break;
	//--------------------------------------------
	case KEY_SEL_SINE:

		g_rtx_sel = sine_key(&g_sine, key);
		break;
	//--------------------------------------------
	case KEY_SEL_SRC_EQ:

		g_rtx_sel = eq_key(&g_src_eq, key);
		break;
	//--------------------------------------------
	case KEY_SEL_SPECMETER:

		g_rtx_sel = specmeter_key(&g_specmeter, key);
		break;
	//--------------------------------------------
	} //end switch(g_key_sel)

	if (g_specmeter.on && !g_src_eq.showInSpecVisualizer)
		specmeter_toggleTimer(&g_specmeter, 1);

}

//---------------------------------------------------------
// ROTARY
//---------------------------------------------------------

static void rtx_setSel(u32 sel)
{
	g_rtx_sel = sel;
}

static void rtx_callback(int steps)
{
	g_rtx_steps = steps;
}

static void rtx_process(void)
{
	int processed = 0;
	s32 rtxSteps = g_rtx_steps;
	g_rtx_steps  = 0; //rtx processed

//	PRS_rtxCallback(steps);

	specmeter_toggleTimer(&g_specmeter, 0);

	if (g_rtx_sel == RTX_SEL_VOL) {
		vol_rotary(&g_volume, rtxSteps);
		processed = 1;
	}

	else switch (g_key_sel)
	{
	//VOLUME
//	default:
//	case KEY_SEL_TOP:		processed = vol_rotary(&g_volume, rtxSteps); break;

	//SINE FREQ
	case KEY_SEL_SINE:		processed = sine_rotary(&g_sine, rtxSteps); break;

	//SRC_EQ
	case KEY_SEL_SRC_EQ:	processed = eq_rotary(&g_src_eq, rtxSteps); break;

	//SPEC METER
	case KEY_SEL_SPECMETER:	processed = specmeter_rotary(&g_specmeter, rtxSteps); break;
	}

	//VOLUME
	if (!processed)
		vol_rotary(&g_volume, rtxSteps);

	if (g_specmeter.on && !g_src_eq.showInSpecVisualizer)
		specmeter_toggleTimer(&g_specmeter, 1);

}

//---------------------------------------------------------
// Main
//---------------------------------------------------------

void TestMiniplayer(void)
{
	PRINTF("\nTestMiniplayer... \n");

	//---- click-clack data ------
	load_song(&g_clickClackInfo, 0, 0);

	//---- rec data ------
	g_recInfo.data = (void*)ALIGN_ADDR_TO_0((u32)g_clickClackInfo.data + g_clickClackInfo.byteSize + 32);


	//---- user control ----
	RTX_Init(rtx_callback);
	KPD_Init(key_callback);

	//---- mini player ----
	PLY_Init();

	//---- DSP -------
#if !BYPASS_DSP
	//PRESET 5
//	DSP_init();
	PRINTF("Init DSP Preset M5... ");
	g_dsp_preset = PRS_Init_preset_M5();
	vol_init		(&g_volume, 	&g_volume_init, 	STAMOD(M5_VOLUME));
	sine_init		(&g_sine, 		&g_sine_init, 		STAMOD(M5_SINE));
	chime_init		(&g_chimeGen1,	&g_chimeGen_init,	STAMOD(M5_BEEP));
	chime_init		(&g_clickClack,	&g_clickClack_init,	STAMOD(M5_CLICKCLACK_X));
	eq_init			(&g_src_eq,		&g_src_eq_init,		STAMOD(M5_SRC_EQ));
	specmeter_init	(&g_specmeter,	&g_specmeter_init,	STAMOD(M5_SPECMETER));

	//load the clicclac song
	STA_PCMLoadData_16bit(g_clickClack.mod, g_clickClackInfo.data, g_clickClackInfo.byteSize);

	PRINTF("OK\n");
#else
	PRINTF("DSP bypassed\n");
#endif

	//---- play ---------
	MSP_play(g_MSP, g_song);

	key_setSel(KEY_SEL_TOP);

	//---- MAIN LOOP ----
	while (!g_exit)
	{
		if (g_key_pressed)
			key_process();

		if (g_rtx_steps)
			rtx_process();

		SLEEP(100);	//avoid sticky loop
	}

	PRINTF("Exiting miniplayer\n");

	//stop and clean
	STA_Reset();
}

#endif //0
