/*
 * STAudioLib - test_equalizer.c
 *
 * Created on 2016/06/28 by Olivier Douvenot
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	Testing EQUALIZER module (static stereo)

	        13-band EQ
	+----+  +-------+
	|SINE|--|  EQ0  |--XOUT[0]
	|GEN |--|  EQ1  |--XOUT[1]
	+----+  +-------+


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

static STAModule g_peak_detector;

//-------------
// SINEWAVE GEN
//-------------
enum SINE_PARAM {SINE_FREQ, SINE_VOL, SINE_BAL};

typedef struct {
	STAModule 	mod;
	s16			gains[2];		//user
	u32			freq;			//user
	u32			msec;
	enum SINE_PARAM param;		//selected param to update
} SINEWAVEGEN;

static const SINEWAVEGEN g_sineInit = {
	.mod 		= 0,
	.gains 		= {-20,-20},	//-2 dB
	.freq		=  20*(33*3+1), 		//20 Hz
	.msec		=  1000, 		// 1000 msec
	.param		= SINE_FREQ,
};

//----------
// EQUALIZER
//----------

#define EQ_NUM_BANDS  5

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

static int g_band_index = 0;

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
//	.type		= STA_EQUA_STATIC_7BANDS_DP, //7-bands: 63, 150, 400, 1000, 2500, 6300, 16000 Hz
	.type		= STA_EQUA_STATIC_7BANDS_SP,
	.bandGQF	= {{0, 30, 63},{0, 30, 150},{0, 30, 400},{0, 30, 1000},{0, 30, 2500},{0, 30, 6300},{0, 30, 16000}}, //7-bands FLAT
#elif (EQ_NUM_BANDS == 13)
	.type		= STA_EQUA_STATIC_13BANDS_DP, //19, 34, 60, 105, 185, 327, 577, 1017, 1794, 3165, 5582, 9846, 17367
	.bandGQF	= {
	{12, 30, 20},
	{12, 30, 32},
	{13, 30, 63},
	{14, 30, 125},
	{5, 30, 500},
	{-9, 90, 1000},
	{-4, 90, 2000},
	{6, 90, 4000},
	{11, 90, 5000},
	{14, 90, 6300},
	{18, 90, 10000},
	{18, 90, 16000},
	{18, 90, 20000}}, //13-bands FLAT
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
static SINEWAVEGEN 	g_sine;
static STAModule	g_selMod;
static EQUA			g_EQ;

static void ResetSinewaveGen(SINEWAVEGEN* sine)
{
	STAModule mod = sine->mod; 				//bak handle
	*sine = g_sineInit; sine->mod = mod;	//reset

	STA_SetMode(mod, STA_SINE_ON);
	STA_SinewaveGenSet(mod, sine->freq, sine->msec, sine->gains[0], sine->gains[1]);
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
	STA_SetFilterGains(EQ->mod, 0xFF, g_eq_preset_G[mode]);

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

	STA_SetFilterGains(EQ->mod, 0xFF, g_eq_preset_G[mode]);
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

// 5 bands EQ, mask 0x1F
static q23 g_WB5_coefs[EQ_NUM_BANDS*6]={
//band0 mask 0x1
936312,
-926669,
936312,
-4853518,
4085714,
0,
//band1 mask 0x2
936312,
-833289,
936312,
-4958199,
3797000,
0,
//band2 mask 0x4
936312,
-528275,
936312,
-5300021,
3295612,
0,
//band3 mask 0x8
936312,
425477,
936312,
-5817625,
2605422,
0,
//band4 mask 0x10
936312,
4194304,
0,
-3054371,
0,
0
};

static q23 g_WB5_45k_coefs[EQ_NUM_BANDS*6]={
703739,
-610280,
703739,
-4179118,
4074062,
0,
1808887,
-1399294,
1808887,
-4243580,
3736875,
0,
2149866,
-991390,
2149866,
-4497800,
3095683,
0,
1557441,
815987,
1557441,
-4912797,
2139978,
0,
2102132,
2102132,
0,
-2578695,
0,
0
};

static q23 g_WB4_45k_coefs[EQ_NUM_BANDS*6]={
831916,
-717489,
831916,
-4299234,
4006768,
0,
1883994,
-1352663,
1883994,
-4320135,
3485746,
0,
1862428,
-369688,
1862428,
-4502064,
2565328,
0,
1135218,
1562034,
1135218,
-4732995,
1541249,
0,
0x400000,
0,
0,
0,
0,
0,
};

static q23 g_NB4_45k_coefs[EQ_NUM_BANDS*6]={
759694,
-1285005,
759694,
-7252908,
4103959,
0,
1760510,
-2891498,
1760510,
-7123429,
3840283,
0,
1569056,
-2238863,
1569056,
-6928657,
3343045,
0,
501925,
40307,
501925,
-6708971,
2758036,
0,
0x400000,
0,
0,
0,
0,
0,
};

static q23 g_NB5_45k_coefs[EQ_NUM_BANDS*6]={
809338,
-1370719,
809338,
-7209708,
4129805,
0,
1941466,
-3241701,
1941466,
-7105728,
3953343,
0,
2203585,
-3503934,
2203585,
-6940477,
3621362,
0,
1507058,
-1953737,
1507058,
-6673935,
3064787,
0,
476058,
188980,
476058,
-6409701,
2508661,
0
};

static q23 g_coefs[EQ_NUM_BANDS*6];

static void SetEqualizerMode(EQUA* EQ, enum EQ_MODE mode)
{
	const char* modeNames[] = {"FLAT", "CLASSIC", "POP", "ROCK", "JAZZ"};

	EQ->mode = mode;

	PRINTF("EQ MODE: %s\n", modeNames[mode]);

	//1. compute the coef
	//PRINTF("STA_SetFilterGains()\n");
	//STA_SetFilterGains(EQ->mod, 0xFF, g_eq_preset_G[mode]);
	//SLEEP(1000);

	//2. get the coefs
	//PRINTF("STA_GetFilterHalvedCoefsAll()\n");
	//STA_GetFilterHalvedCoefsAll(EQ->mod, 0xFF, g_coefs);
//	SLEEP(10);

	//3. re-set the coefs
	//PRINTF("STA_SetFilterHalvedCoefsAll()\n");
	//STA_SetFilterHalvedCoefsAll(EQ->mod, 0xFF, g_coefs);

	//STA_SetFilterType(EQ->mod, 0, STA_BIQUAD_AP2_CB);
	//STA_SetFilter(EQ->mod, 0, 0, 100, 7000);


	//STA_SetFilterType(EQ->mod, 0, STA_BIQUAD_HIGP_BUTTW_2);
	//STA_SetFilter(EQ->mod, 0, -240, 100, 7000);

	//STA_GetFilterHalvedCoefsAll(EQ->mod, 0x1F, g_simple_coefs);

	/*STA_SetFilterType(EQ->mod, 1, STA_BIQUAD_BYPASS);
	STA_SetFilterType(EQ->mod, 2, STA_BIQUAD_BYPASS);
	STA_SetFilterType(EQ->mod, 3, STA_BIQUAD_BYPASS);
	STA_SetFilterType(EQ->mod, 4, STA_BIQUAD_BYPASS);*/

	STA_SetFilterHalvedCoefsAll(EQ->mod, 0x1F, g_WB5_45k_coefs);

}

//---------------------------------------------------------
// BUILD FLOW
//---------------------------------------------------------

static void DSP_Init_(void)
{
	STAModule EQ;
	int i;

	STA_Reset();

	//SINEWAVE
	g_sine.mod = STA_AddModule(DSP_SEL, STA_SINE_2CH);
	ResetSinewaveGen(&g_sine);

	//EQ
	g_EQ = g_EQInit;
	g_EQ.mod = EQ = STA_AddModule(STA_DSP0, g_EQ.type);  //(2 ch by default)
	STA_SetNumChannels(EQ, 1);

	g_peak_detector = STA_AddModule(STA_DSP0, STA_PEAKDETECT_NCH); //2 gains by default
	STA_SetNumChannels(g_peak_detector, 1);  						//add more channels
	STA_SetMode(g_peak_detector, 1);

	//CONNECTIONS
	FORi(1) {
		STA_Connect(g_sine.mod, i,  EQ,     i);
		STA_Connect(EQ,       i,  g_peak_detector, i);
		STA_Connect(EQ,       i,  STA_XOUT0, i);
		STA_Connect(EQ,       i,  STA_XOUT0, i+1);

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
// SEL (effects)
//---------------------------------------------------------
static void toggleSEL(void)
{
	g_selMod = (g_selMod == g_sine.mod) ? g_EQ.mod : g_sine.mod;

	if (g_selMod == g_sine.mod)
		PRINTF("SINE = %d Hz\n", g_sine.freq/100);
	else
		PRINTF("EQ\n");
}

//---------------------------------------------------------
// KEY / ROTARY
//---------------------------------------------------------
static int g_key_pressed = 0;
static int g_rtx_steps   = 0;

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


	//SEL
	case 0x13:	toggleSEL(); return;
	}


	//EQ
	if (g_selMod == g_EQ.mod) {
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

//u32 freqs[22] = {20,200,1000,2000,3000,4000,5000,6000,7000,8000,9000,10000,11000,12000,13000,14000,15000,16000,17000,18000,19000,20000};
u32 freqs[39] = {20,200,500,1000,1500,2000,2500,3000,3500,4000,4500,5000,5500,6000,6500,7000,7500,8000,8500,9000,9500,10000,10500,11000,11500,12000,12500,13000,13500,14000,14500,15000,15500,16000,16500,17000,17500,18000,18500};
//u32 freqs[14] = {20,200,2000,4000,6000,7000,8000,9000,10000,12000,14000,16000,18000,20000};
//u32 freqs[11] = {7000,7100,7200,7300,7400,7500,7600,7700,7800,7900,8000};
//u32 freqs[11] = {3000,3100,3200,3300,3400,3500,3600,3700,3800,3900,4000};

if (steps < 0)
{
if (g_band_index > 0) g_band_index--;
}
else
{
if (g_band_index < 39) g_band_index++;
}

g_sine.freq = 100*freqs[g_band_index];

						PRINTF("SINE: Frq = %d Hz\n", g_sine.freq/100);


}

static void keyCallback(int key, bool down)
{
	g_key_pressed = key;
	process_key();
}

static void rtxCallback(int steps)
{
	g_rtx_steps = steps;
	process_rtx();
}


//---------------------------------------------------------
// Main
//---------------------------------------------------------

void TestSineEqualizer(void)
{
	int nch = 2;
	s32 level;
	u32 cycles;
	u32 freq;

	PRINTF("TestSineEqualizer... \n");
	PRINTF("EQ %d-BANDS\n", g_EQInit.numBands);

	//user control
	RTX_Init(rtxCallback);
	KPD_Init(keyCallback);

	//audio init
	DSP_Init_();
//	AUD_Routing(nch);
	DMABUS_Set(PLY_MSP2, g_song1.nch, DSP_SEL);

	//mini player
	//PLY_Init();

	//play
	//PLY_Set_pcm_source_MSP(MSP2, &g_song1, PLY_NOFLAG);
	//PLY_SetState_MSP(MSP2, PLY_PLAY);
    AIF->SAI3_CR.REG = SAI_I2S | SAI_MST | SAI_32_24VAL | SAI_RL | SAI_EN;


    cycles = STA_GetMaxDspCycleCost(DSP_SEL);
    PRINTF("global cycles in DSP=%d\n", cycles);

	while (1)
	{
        freq = g_sine.freq;
        STA_SinewaveGenSet(g_sine.mod, (48*(freq+300))/48, g_sine.msec, g_sine.gains[0], g_sine.gains[1]);
        SLEEP(800);
        STA_PeakDetectorResetLevel(g_peak_detector, 0);
        SLEEP(100);
        //level = STA_PeakDetectorGetLevel(g_peak_detector, 0, STA_PEAK_LINEAR);
        //PRINTF("%d DSP cycles, linear level at %d Hz = %d (s24)\n", cycles, freq/100, level);
        level = STA_PeakDetectorGetLevel(g_peak_detector, 0, STA_PEAK_DB);
        PRINTF("%d DSP cycles, log level at %d Hz = %d (dB/10)\n", cycles, freq/100, level);
	}

	//stop and clean
	STA_Reset();
}


#endif //0
