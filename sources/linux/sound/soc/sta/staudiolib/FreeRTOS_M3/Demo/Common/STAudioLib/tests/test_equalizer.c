/*
 * STAudioLib - test_equalizer.c
 *
 * Created on 2013/04/22 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	Testing EQUALIZER module (static stereo)

	            GAINS    7-band EQ
	          +-------+  +-------+
	L XIN[0]--| Gain0 |--|  EQ0  |--XOUT[0]
	R XIN[1]--| Gain1 |--|  EQ1  |--XOUT[1]
	          +-------+  +-------+


 7-Band EQUALIZER Presets:
 =========================
 +/-10 unit   scale -> 0    1    2    3    4   5     6     7     8     9   10
 +/-24 dB     scale -> 0  2.4  4.8  7.2  9.6  12  14.4  16.8  19.2  21.6   24  dB
 +/-24 dB(x4) scale -> 0   10   19   29 ....

		FLAT			CLASSIC			POP				ROCK			JAZZ
64    ---o----------  ---o--|-------  -|-o----------  ---o----|-----  ---o|---------
150   ---o----------  ---o-|--------  ---|----------  ---o-|--------  ---|----------
400   ---o----------  ---o|---------  ---o--|-------  ---|----------  ---|----------
1000  ---o----------  ---|----------  ---o----|-----  ---o--|-------  ---o-|--------
2500  ---o----------  --|o----------  ---o-|--------  ---|----------  ---o---|------
6300  ---o----------  ---o-|--------  ---|----------  ---o-|--------  ---o----|-----
16000 ---o----------  ---o----|-----  --|o----------  ---o---|------  ---o-|--------

*/

#if 1
#include "staudio_test.h"

#define DSP_SEL				0

//---------
// GAINS
//---------
#define GAIN_L	0x1
#define GAIN_R	0x2

typedef struct {
	STA_ModuleType	type;
	STAModule 		mod;
	u32				LRmask;
	s16				L, R;		//controlable
	u32				tup, tdown;
	bool			mute;
} GAIN;

static const GAIN g_gainsInit = {
//	.type		= STA_GAIN_STATIC_NCH,
	.type		= STA_GAIN_SMOOTH_NCH,
	.mod 		= 0,
	.LRmask		= GAIN_L|GAIN_R,
	.L 			= -120,		//-12 dB  <-- - max EQ gain
	.R			= -120,		//-12 dB
	.tup		=  50,		//100 msec
	.tdown		=  20,		//100 msec
};

//----------
// EQUALIZER
//----------

#define EQ_NUM_BANDS  7

enum EQ_MODE {EQ_FLAT, EQ_CLASSIC, EQ_POP, EQ_ROCK, EQ_JAZZ};

typedef struct {
	STAModule 	 mod;
	STA_ModuleType type;
	enum EQ_MODE mode;
	u8			 ON;
	u8			 numBands;
	u8			 selBand;
	u8			 selParam;
	s16			 bandGQF[EQ_NUM_BANDS][3]; //controlable
} EQUA;


static const EQUA g_EQInit = {
	.mod 		= 0,
	.mode		= EQ_FLAT,
	.ON			= 0,
	.numBands	= EQ_NUM_BANDS,
	.selBand	= 0,
	.selParam	= 0,
#if (EQ_NUM_BANDS == 5)
	.type		= STA_EQUA_STATIC_5BANDS_DP,
	.bandGQF	= {{0, 30, 150},{0, 30, 400},{0, 30, 1000},{0, 30, 2500},{0, 30, 6300}}, //5-bands FLAT
#elif (EQ_NUM_BANDS == 7)
	.type		= STA_EQUA_STATIC_7BANDS_SP, //7-bands: 63, 150, 400, 1000, 2500, 6300, 16000 Hz
	.bandGQF	= {{0, 30, 63},{0, 30, 150},{0, 30, 400},{0, 30, 1000},{0, 30, 2500},{0, 30, 6300},{0, 30, 16000}}, //7-bands FLAT
#elif (EQ_NUM_BANDS == 13)
	.type		= STA_EQUA_STATIC_13BANDS_DP, //19, 34, 60, 105, 185, 327, 577, 1017, 1794, 3165, 5582, 9846, 17367
	.bandGQF	= {{0, 30, 20},{0, 30, 34},{0, 30, 60},{0, 30, 105},{0, 30, 185},{0, 30, 327},{0, 30, 577},{0, 30, 1017},{0, 30, 1794},{0, 30, 3165},{0, 30, 5582},{0, 30, 9846},{0, 30, 17367}}, //7-bands FLAT
#endif
};

//convert from graphical EQ unit (max +/-10 unit) to rescaled dbGain (max +/-24*BGS db)
#define G(u) 	(u * 24 * STA_BIQ_GAIN_SCALE / 10)

#if (EQ_NUM_BANDS == 5)
static const s16 g_eq_preset_G[][5] = {
	//150   400     1K    2.5    6.3
	{G(0),  G(0),  G(0),  G(0),  G(0)  },  //FLAT
	{G(2),  G(1),  G(0),  G(-1), G(2)  },  //CLASSIC
	{G(0),  G(3),  G(5),  G(2),  G(0)  },  //POP
	{G(2),  G(0),  G(3),  G(0),  G(2)  },  //ROCK    <--- need more bass boost (hence a lower freq band)
	{G(0),  G(0),  G(2),  G(4),  G(5)  }}; //JAZZstatic
#elif (EQ_NUM_BANDS == 7)
const s16 g_eq_preset_G[][7] = {
	// 63   150    400     1K    2.5    6.3    16K
	{G(0),  G(0),  G(0),  G(0),  G(0),  G(0),  G(0)},  //FLAT
	{G(3),  G(2),  G(1),  G(0),  G(-1), G(2),  G(5)},  //CLASSIC
	{G(-2), G(0),  G(3),  G(5),  G(2),  G(0),  G(-1)}, //POP
	{G(5),  G(2),  G(0),  G(3),  G(0),  G(2),  G(4)},  //ROCK
	{G(1),  G(0),  G(0),  G(2),  G(4),  G(5),  G(2)}}; //JAZZ
#elif (EQ_NUM_BANDS == 13)
//extrapolated from EQ 7 bands (just to test against pop/noise)
static const s16 g_eq_preset_G[][13] = {
	// 1      2      3      4      5      6      7      8      9     10     11     12     13
	{G(0),  G(0),  G(0),  G(0),  G(0),  G(0),  G(0),  G(0),  G(0),  G(0),  G(0),  G(0),  G(0)},  //FLAT
	{G(3),  G(3),  G(2),  G(2),  G(1),  G(1),  G(0),  G(-1), G(-1), G(0),  G(2),  G(3),  G(5)},  //CLASSIC
	{G(-2), G(-1), G(0),  G(2),  G(3),  G(4),  G(5),  G(4),  G(2),  G(1),  G(0),  G(-1), G(-1)}, //POP
	{G(5),  G(4),  G(2),  G(1),  G(0),  G(2),  G(3),  G(2),  G(0),  G(1),  G(2),  G(3),  G(4)},  //ROCK
	{G(1),  G(1),  G(0),  G(0),  G(0),  G(1),  G(2),  G(3),  G(4),  G(5),  G(5),  G(4),  G(2)}}; //JAZZ
#endif

#undef G

//----------
// MODULES
//----------

//for user interaction
static STAModule	g_selMod;
static EQUA			g_EQ;
static GAIN 		g_gains;

//---------------------------------------------------------
// GAIN
//---------------------------------------------------------

static void ResetGains(GAIN* gains)
{
	STAModule mod = gains->mod; 			//bak handle
	*gains = g_gainsInit; gains->mod = mod;	//reset

	STA_GainSetGains(gains->mod, GAIN_L, gains->L);
	STA_GainSetGains(gains->mod, GAIN_R, gains->R);

	if (gains->type != STA_GAIN_STATIC_NCH) {
		STA_SmoothGainSetTC(gains->mod, 0, gains->tup, gains->tdown);
		STA_SmoothGainSetTC(gains->mod, 1, gains->tup, gains->tdown);
	}

}

//---------------------------------------------------------
// EQ
//---------------------------------------------------------

static void ResetEqualizer(EQUA* EQ)
{
	STAModule mod = EQ->mod;
	int i;

	*EQ = g_EQInit; EQ->mod = mod;			//reset values

	STA_SetMode(mod, STA_EQ_OFF);

	FORi(EQ->numBands) {
		STA_SetFilterType(mod, i, STA_BIQUAD_BAND_BOOST_2);
		STA_SetFilterv(mod, i, EQ->bandGQF[i]); //FLAT mode
	}

	//FOR DEBUG
//	STA_SetFilterType(mod, 0, STA_BIQUAD_BYPASS);
//	STA_SetFilterType(mod, 1, STA_BIQUAD_BYPASS);
//	STA_SetFilterType(mod, 2, STA_BIQUAD_BYPASS);
//	STA_SetFilterType(mod, 3, STA_BIQUAD_BYPASS);
//	STA_SetFilterType(mod, 4, STA_BIQUAD_BYPASS);
//	STA_SetFilterType(mod, 5, STA_BIQUAD_BYPASS);
//	STA_SetFilterType(mod, 6, STA_BIQUAD_BYPASS);
}


//USE smooth mute/unmute trick
#if 0
static void SetEqualizerMode(EQUA* EQ, enum EQ_MODE mode)
{
	const char* modeNames[] = {"FLAT", "CLASSIC", "POP", "ROCK", "JAZZ"};

	EQ->mode = mode;

	PRINTF("EQ MODE: %s\n", modeNames[mode]);

	//1. mute:
	STA_GainSetGains(g_gains.mod, GAIN_L|GAIN_R, -1200);
	SLEEP(g_gains.tdown * 2);

	//2. Change EQ
	STA_SetFilterGains(EQ->mod, 0xFFFF, g_eq_preset_G[mode]);

	//3. unmute
	STA_GainSetGains(g_gains.mod, GAIN_L, g_gains.L);
	STA_GainSetGains(g_gains.mod, GAIN_R, g_gains.R);
}

static void toggleEqualizerOnOff(EQUA* EQ)
{
	EQ->ON ^= 1;

	PRINTF("EQ %s\n",  EQ->ON ? "ON":"OFF");

	//1. mute:
	STA_GainSetGains(g_gains.mod, GAIN_L|GAIN_R, -1200);
	SLEEP(g_gains.tdown * 2);

	//2. Change EQ
	STA_SetMode(EQ->mod, EQ->ON ? STA_EQ_ON : STA_EQ_OFF);

	//3. unmute
	STA_GainSetGains(g_gains.mod, GAIN_L, g_gains.L);
	STA_GainSetGains(g_gains.mod, GAIN_R, g_gains.R);
}
#else
static void SetEqualizerMode_(EQUA* EQ, enum EQ_MODE mode)
{
	const char* modeNames[] = {"FLAT", "CLASSIC", "POP", "ROCK", "JAZZ"};

	EQ->mode = mode;

	PRINTF("EQ MODE: %s\n", modeNames[mode]);

	STA_SetFilterGains(EQ->mod, 0xFFFF, g_eq_preset_G[mode]);
}

static void toggleEqualizerOnOff(EQUA* EQ)
{
	EQ->ON ^= 1;

	PRINTF("EQ %s\n",  EQ->ON ? "ON":"OFF");

	STA_SetMode(EQ->mod, EQ->ON ? STA_EQ_ON : STA_EQ_OFF);
}
#endif

//---------------------------------------------------------
// testing STA_SetFilterHalvedCoefsAll / STA_GetFilterHalvedCoefsAll
//---------------------------------------------------------

static q23 g_coefs[EQ_NUM_BANDS*6];

static void SetEqualizerMode(EQUA* EQ, enum EQ_MODE mode)
{
	const char* modeNames[] = {"FLAT", "CLASSIC", "POP", "ROCK", "JAZZ"};

	EQ->mode = mode;

	PRINTF("EQ MODE: %s\n", modeNames[mode]);

	//1. compute the coef
	PRINTF("STA_SetFilterGains()\n");
	STA_SetFilterGains(EQ->mod, 0xFFFF, g_eq_preset_G[mode]);
	SLEEP(1000);

	//2. get the coefs
	PRINTF("STA_GetFilterHalvedCoefsAll()\n");
	STA_GetFilterHalvedCoefsAll(EQ->mod, 0xFFFF, g_coefs);
//	SLEEP(10);

	//3. re-set the coefs
	PRINTF("STA_SetFilterHalvedCoefsAll()\n");
	STA_SetFilterHalvedCoefsAll(EQ->mod, 0xFFFF, g_coefs);

}

//---------------------------------------------------------
// BUILD FLOW
//---------------------------------------------------------

static void DSP_Init_(void)
{
	STAModule gains, EQ;
	int i;

	STA_Reset();

	//GAINS
	g_gains = g_gainsInit;
//	g_gains.mod = gains = STA_AddModule(STA_DSP0, STA_GAIN_STATIC_NCH);  //(2 gains by default)
	g_gains.mod = gains = STA_AddModule(STA_DSP0, g_gains.type);  //(2 gains by default)

	if (g_gains.type != STA_GAIN_STATIC_NCH)
		ResetGains(&g_gains); //smooth gain can be set before BuildFlow!

	//EQ
	g_EQ = g_EQInit;
	g_EQ.mod = EQ = STA_AddModule(STA_DSP0, g_EQ.type);  //(2 ch by default)

	//CONNECTIONS
	FORi(2) {
		STA_Connect(STA_XIN0, i,  gains,     i);
		STA_Connect(gains,    i,  EQ,        i);
		STA_Connect(EQ,       i,  STA_XOUT0, i);

/*		STA_Connect(STA_XIN0, i,  gains,     i);
		STA_Connect(gains,    i,  STA_XOUT0, i);
*/
	}

	//BUILD
	STA_BuildFlow();

	ASSERT(STA_GetError() == STA_NO_ERROR);


	//START dsp
	STA_Play();
	SLEEP(10);					//wait 10ms for DSP initialisation

	//POST INIT
	ResetGains(&g_gains);
	ResetEqualizer(&g_EQ);

	g_selMod = g_EQ.mod;
}

/*
static void AUD_Routing(int nch)
{
	//DMABUS
	STA_DMAReset();

	STA_DMAAddTransfer(STA_DMA_SRC0_DO,		STA_DMA_DSP0_XIN, nch);		//SAI2 to DSP
//	STA_DMAAddTransfer(STA_DMA_SAI2_RX1,	STA_DMA_DSP0_XIN, nch);		//SAI2 to DSP
	if (nch == 1)
		STA_DMAAddTransfer(STA_DMA_DSP0_XIN, STA_DMA_DSP0_XIN+1, 1); 	//R (copy from L)

	STA_DMAAddTransfer(STA_DMA_DSP0_XOUT,  	STA_DMA_SAI3_TX1,    2);	//DSP to SAI3

	STA_DMAAddTransfer(STA_DMA_SAI2_RX1, 	STA_DMA_DSP0_XIN+127, 1);	//swap trigger
}


static void AUD_Routing_FIFO2(int nch)
{
	//DMABUS
	STA_DMAReset();
	STA_DMAAddTransfer(STA_DMA_FIFO_OUT,	STA_DMA_DSP0_XIN, nch);		//FIFO2 to DSP
	if (nch == 1)
		STA_DMAAddTransfer(STA_DMA_DSP0_XIN, STA_DMA_DSP0_XIN+1, 1); 	//R (copy from L)
	STA_DMAAddTransfer(STA_DMA_DSP0_XOUT,  	STA_DMA_SAI3_TX1,    2);	//DSP to SAI3
	STA_DMAAddTransfer(STA_DMA_SAI2_RX1, 	STA_DMA_DSP0_XIN+127, 1);	//swap trigger
}
*/


//---------------------------------------------------------
// MUTE
//---------------------------------------------------------
static void toggleMute(void)
{
	g_gains.mute ^= 1;

	PRINTF(g_gains.mute ? "GAIN: mute\n" : "GAIN: unmute\n");

	if (g_gains.mute)
		STA_GainSetGains(g_gains.mod, GAIN_L|GAIN_R, -1200);
	else {
		STA_GainSetGains(g_gains.mod, GAIN_L, g_gains.L);
		STA_GainSetGains(g_gains.mod, GAIN_R, g_gains.R);

		//replay sine
//		STA_SinewaveGenSet(g_sine.mod, g_sine.freq, g_sine.msec, g_sine.gains[0], g_sine.gains[1]);
	}
}
//---------------------------------------------------------
// SEL (effects)
//---------------------------------------------------------
static void toggleSEL(void)
{
	g_selMod = (g_selMod == g_gains.mod) ? g_EQ.mod : g_gains.mod;

	if (g_selMod == g_gains.mod)
		PRINTF("GAIN L,R = %d, %d dB\n", g_gains.L/10, g_gains.R/10);
	else
		PRINTF("EQ\n");
}
//---------------------------------------------------------
// DSP CYCLE FREE COUNT
//---------------------------------------------------------
static void togglePrintFreeCycleCount(void)
{
	static int s_enable = 0;

	s_enable ^= 1;

	PRINTF("FREE cycle Counter %s\n", s_enable ? "ON" : "OFF");
	STA_PrintFreeCycles(s_enable ? 2 : 0); //2 sec or stop
}
//---------------------------------------------------------
// KEY / ROTARY
//---------------------------------------------------------
static int g_key_pressed = 0;
static int g_rtx_steps   = 0;

static void keyCallback(int key, bool down)
{
	g_key_pressed = key;
}

static void rtxCallback(int steps)
{
	g_rtx_steps = steps;
}

//Process the key/rtx in main task instead of callbacks
static void process_key(void)
{
	/* OLD
	key mapping SEL=EQ:
	[    ]	[  ]		[ONOFF]  [FLAT]  [CLSC]  [POP]
						[     ]  [ROCK]  [JAZZ]  [   ]
	[GAIN]	[EQ]		[RST  ]  [    ]  [    ]  [   ]
	*/

	/* NEW
	key mapping SEL=GAIN:
	[MUTE]	[  ]		[   ]  [   ]  [   ]   [   ]
						[   ]  [   ]  [   ]   [   ]
	[SEL]	[  ]		[RST]  [   ]  [Lmask] [Rmask]


	key mapping SEL=EQ:
	[MUTE]	[  ]		[    ]  [  ]  [    ]  [    ]
						[FLAT]  [  ]  [ROCK]  [JAZZ]
	[SEL]	[ONOFF]		[RST ]  [  ]  [CLSC]  [POP ]
	*/


	int key = g_key_pressed;
	g_key_pressed = 0;  //key processed

	//module selection
	//row1
	switch(key) {

	//MUTE
//	case 0x11: 	toggleMute(); return;

	//SEL
	case 0x13:	toggleSEL(); return;

	//FREE CYLE COUNT
//	case 0x41: printDSPFreeCycleCount(); return;
	case 0x11: togglePrintFreeCycleCount(); return;
	}

	//GAIN
	if (g_selMod == g_gains.mod)
	{
		switch(key) {
		case 0x41:  ResetGains(&g_gains); PRINTF("GAIN L,R = %d, %d dB\n", g_gains.L/10, g_gains.R/10); return;
		default: return;
		}
		return;
	}

	//EQ
	if (g_selMod == g_EQ.mod)
	{
		switch(key) {
		case 0x14: toggleEqualizerOnOff(&g_EQ); 		break; //toggle on/off
		case 0x31: SetEqualizerMode(&g_EQ, EQ_FLAT); 	break;
		case 0x43: SetEqualizerMode(&g_EQ, EQ_CLASSIC); break;
		case 0x44: SetEqualizerMode(&g_EQ, EQ_POP); 	break;
		case 0x33: SetEqualizerMode(&g_EQ, EQ_ROCK); 	break;
		case 0x34: SetEqualizerMode(&g_EQ, EQ_JAZZ); 	break;
		case 0x41: ResetEqualizer(&g_EQ); PRINTF("EQ RESETED\n"); break;
		default: return;
		}
		return;
	}
}

static void process_rtx(void)
{
	int steps = g_rtx_steps;
	g_rtx_steps = 0; //rtx processed


	//GAIN
//	if (g_selMod == g_gains.mod)
	{
		if (g_gains.LRmask & GAIN_L) {
			g_gains.L += steps * 60; //gain step = +-6 dB
			STA_GainSetGains(g_gains.mod, GAIN_L, g_gains.L);
		}
		if (g_gains.LRmask & GAIN_R) {
			g_gains.R += steps * 60; //gain step = +-6 dB
			STA_GainSetGains(g_gains.mod, GAIN_R, g_gains.R);
		}
		PRINTF("GAIN L,R = %d, %d dB\n", g_gains.L/10, g_gains.R/10);
		return;
	}

	//EQ
/*	if (g_selMod == g_EQ.mod) {
		return;
	}
*/
}


//---------------------------------------------------------
// Main
//---------------------------------------------------------

void TestEqualizer(void)
{
	int nch = 2;

	PRINTF("TestEqualizer... \n");
	PRINTF("EQ %d-BANDS\n", g_EQInit.numBands);

	//user control
	RTX_Init(rtxCallback);
	KPD_Init(keyCallback);

	//audio init
	DSP_Init_();
//	AUD_Routing(nch);
	DMABUS_Set(PLY_MSP2, g_song1.nch, DSP_SEL);

	//mini player
	PLY_Init();

	//play
	PLY_Set_pcm_source_MSP(MSP2, &g_song1, PLY_NOFLAG);
	PLY_SetState_MSP(MSP2, PLY_PLAY);


	while (1)
	{
		if (g_key_pressed)
			process_key();

		if (g_rtx_steps)
			process_rtx();

		SLEEP(100);	//avoid sticky loop
	}

	//stop and clean
	STA_Reset();
}

/*
void TestEqualizer_FIFO2(void)
{
	int nch = 2;

	PRINTF("TestEqualizer_FIFO2... \n");
	PRINTF("EQ %d-BANDS\n", g_EQInit.numBands);

	//user control
	RTX_Init(rtxCallback);
	KPD_Init(keyCallback);

	//audio init
	DSP_Flow();
	AUD_Routing_FIFO2(nch);

	//start dsp
	STA_Play();
	SLEEP(5);					//wait 5ms for DSP initialisation

	//post initialisation
	ResetGains(&g_gains);
	ResetEqualizer(&g_EQ);
	g_selMod = g_gains.mod;

	//play (in loop)
	STA_DMAStart();

	//mini player
	PLY_Init();
	PLY_Set_pcm_source_MSP(MSP2, &g_song1, PLY_NOFLAG);
	PLY_SetState_MSP(MSP2, PLY_PLAY);


	while (1) {
		SLEEP(100);	//avoid sticky loop
	}

	//stop and clean
	STA_Reset();
}
*/

#endif //0
