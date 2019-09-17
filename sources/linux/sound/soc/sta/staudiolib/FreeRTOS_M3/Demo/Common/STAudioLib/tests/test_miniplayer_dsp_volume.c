/*
 * STAudioLib - test_miniplayer_dsp_volume.c
 *
 * Created on 2013/04/22 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	Testing SMOOTH GAIN module

			  S.GAIN
	+-----+  +------+
	|SINE |--|gain0 |--XOUT[0] L
	|GEN  |--|gain1 |--XOUT[1] R
	+-----+  +------+

 */

#if 0
#include "staudio_test.h"


#define DSP_SEL				0

#if (DSP_SEL == 0)
#define DMA_XIN 			STA_DMA_DSP0_XIN
#define DMA_XOUT 			STA_DMA_DSP0_XOUT
#define MOD_XIN				STA_XIN0
#define MOD_XOUT			STA_XOUT0
#elif (DSP_SEL == 1)
#define DMA_XIN 			STA_DMA_DSP1_XIN
#define DMA_XOUT 			STA_DMA_DSP1_XOUT
#define MOD_XIN				STA_XIN1
#define MOD_XOUT			STA_XOUT1
#else
#define DMA_XIN 			STA_DMA_DSP2_XIN
#define DMA_XOUT 			STA_DMA_DSP2_XOUT
#define MOD_XIN				STA_XIN2
#define MOD_XOUT			STA_XOUT2
#endif

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
//	.gains 		= {-200,-200}, //-20 dB so to let space for volume ctr > 0 dB
	.gains 		= {-120,-120}, //-120 dB so to let space for volume ctr > 0 dB
//	.freq		=    3000, // 30 Hz
	.freq		=   44000, //440 Hz
//	.freq		=  700000, //  7 kHz
	.msec		= 20000, //20 sec
	.param		= SINE_FREQ,
};

//-------------
// SMOOTH GAINS
//-------------
#define GAIN_L	0x1
#define GAIN_R	0x2

typedef struct {
	STA_ModuleType	type;
	STAModule 		mod;
	u32				LRmask;
	s16				L, R;		//controlable
	u32				dbStep;
	u32				tup, tdown;
	bool			mute;
} GAIN;

static const GAIN g_gainsInit = {
//	.type		= STA_GAIN_STATIC_NCH,
	.type		= STA_GAIN_SMOOTH_NCH,
//	.type		= STA_GAIN_LINEAR_NCH,
	.mod 		= 0,
	.LRmask		= GAIN_L|GAIN_R,
	.L 			= -120,	 //  0 dB
	.R			= -120,	 //  0 dB
	.dbStep		=   60,	 // +-6dB
	.tup		=  100,  //msec
	.tdown		=  100,  //msec
};

//---------
// MODULES
//---------
//for user interaction
static SINEWAVEGEN 	g_sine;
static GAIN         g_gains;

static STAModule	g_selected;	//user selected module


//---------------------------------------------------------
// DSP FLOW
//---------------------------------------------------------

static void ResetSinewaveGen(SINEWAVEGEN* sine)
{
	STAModule mod = sine->mod; 				//bak handle
	*sine = g_sineInit; sine->mod = mod;	//reset

	STA_SetMode(mod, STA_SINE_ON);
	STA_SinewaveGenSet(mod, sine->freq, sine->msec, sine->gains[0], sine->gains[1]);
}

static void ResetGains(GAIN* gains)
{
	STAModule mod = gains->mod; 			//bak handle
	*gains = g_gainsInit; gains->mod = mod;	//reset

	STA_GainSetGains(gains->mod, GAIN_L, gains->L);
	STA_GainSetGains(gains->mod, GAIN_R, gains->R);

	if (gains->type != STA_GAIN_STATIC_NCH) {
//		STA_SmoothGainSetTC(gains->mod, 0, gains->tup, gains->tdown);
//		STA_SmoothGainSetTC(gains->mod, 1, gains->tup, gains->tdown);
		STA_SmoothGainSetRamp(gains->mod, 0xF, gains->tup, gains->tdown);
	}

}

void DSP_init(void)
{
	int i;

	STA_Reset();

#if 0
	//SINEWAVE
	g_sine.mod = STA_AddModule(DSP_SEL, STA_SINE_2CH);
	ResetSinewaveGen(&g_sine);
#endif

	//GAINS
	g_gains = g_gainsInit;
	g_gains.mod = STA_AddModule(DSP_SEL, g_gains.type); //2 gains by default
	g_gains.mod = STA_SetNumChannels(g_gains.mod, 6);	//just to try, even if only using 2ch

	if (g_gains.type != STA_GAIN_STATIC_NCH)
		ResetGains(&g_gains); //smooth gain can be set before BuildFlow!

	//CONNECTIONS
	FORi(2) {
#if 0
        STA_Connect(g_sine.mod,  i,  g_gains.mod, i);
        STA_Connect(g_gains.mod, i,  MOD_XOUT,   i);
#endif
		//XIN
        STA_Connect(MOD_XIN,     i,  g_gains.mod, i);
        STA_Connect(g_gains.mod, i,  MOD_XOUT,    i);

    }

	//BUILD
	STA_BuildFlow();

	ASSERT(STA_GetError() == STA_NO_ERROR);


	//START dsp
	STA_StartDSP(DSP_SEL);
	SLEEP(100);					//wait 100ms for DSP initialisation

	//post initialisation
	if (g_gains.type == STA_GAIN_STATIC_NCH)
		ResetGains(&g_gains);

	g_selected = g_gains.mod;
}

//---------------------------------------------------------
// DMABUS
//---------------------------------------------------------
/*
static void DMABUS_init(int nch)
{

	STA_DMAReset();

	//DSP input
	STA_DMAAddTransfer(STA_DMA_SAI2_RX1,	DMA_XIN, nch);			//SAI2 to XIN
	if (nch == 1)
		STA_DMAAddTransfer(DMA_XIN, 		DMA_XIN+1, 1); 			//R (copy from L)

	//DSP output
	STA_DMAAddTransfer(DMA_XOUT,  			STA_DMA_SAI3_TX1,  2);	//XOUT to SAI3

	//DSP dump from FIFO1
	STA_DMAAddTransfer(DMA_XOUT,			STA_DMA_FIFO_IN,   2);

	//DSP IO swap
	STA_DMAAddTransfer(STA_DMA_SAI2_RX1, 	DMA_XIN+127, 1);		//swap trigger

}
*/



//---------------------------------------------------------
// KEY / ROTARY
//---------------------------------------------------------
void DSP_toggleMute(void)
{
	g_gains.mute ^= 1;

	PRINTF(g_gains.mute ? "GAIN: mute\n" : "GAIN: unmute\n");

	if (g_gains.mute) {
		STA_GainSetGains(g_gains.mod, 0x3, -1200);
	}
	else {
		STA_GainSetGains(g_gains.mod, GAIN_L, g_gains.L);
		STA_GainSetGains(g_gains.mod, GAIN_R, g_gains.R);
	}
}

/*
static void keyCallback(int key, bool down)
{
	*
	key mapping (GAIN)
	[    ]  [xxxx]	    [VOL]  [RST]  [LMASK]  [RMASK]  <- GAIN
						[   ]  [   ]  [     ]  [     ]
	[MUTE]  [REC ]		[FRQ]  [RST]  [ VOL ]  [ BAL ]  <- SINEWAVEGEN
	*

	switch(key) {

	//MUTE
	case 0x13: 	toggleMute(); return;

	//REC
	case 0x14:	toggleRecord(); return;


	//SINEWAVE GEN
//	case 0x41:  g_selected = g_sine.mod; g_sine.onoff ^= 1; STA_SetMode(g_sine.mod, g_sine.onoff); PRINTF("SINEWAVE GEN: %s\n", g_sine.onoff ? "ON":"OFF"); return;
	case 0x41:  g_selected = g_sine.mod; g_sine.param = SINE_FREQ;  PRINTF("SINE: Frq = %d Hz\n", g_sine.freq/100); return;
	case 0x42:  g_selected = g_sine.mod; ResetSinewaveGen(&g_sine); PRINTF("SINE: Reset\n"); return;
	case 0x43:  g_selected = g_sine.mod; g_sine.param = SINE_VOL;   PRINTF("SINE: Vol: L = %d dB, R = %d dB\n", g_sine.gains[0]/10, g_sine.gains[1]/10); return;
	case 0x44:  g_selected = g_sine.mod; g_sine.param = SINE_BAL;   PRINTF("SINE: Bal: L = %d dB, R = %d dB\n", g_sine.gains[0]/10, g_sine.gains[1]/10); return;

	//GAIN
	case 0x31:  g_selected = g_gains.mod; PRINTF("GAIN: L,R = %d, %d dB\n", g_gains.L/10, g_gains.R/10); return;
	case 0x32:  g_selected = g_gains.mod; ResetGains(&g_gains); PRINTF("GAIN: L,R = %d, %d dB\n", g_gains.L/10, g_gains.R/10); return;
	case 0x33:  g_selected = g_gains.mod; g_gains.LRmask ^= GAIN_L; PRINTF("Change %s%s\n", (g_gains.LRmask&GAIN_L)?"L":"", (g_gains.LRmask&GAIN_R)?"R":""); return;
	case 0x34:  g_selected = g_gains.mod; g_gains.LRmask ^= GAIN_R; PRINTF("Change %s%s\n", (g_gains.LRmask&GAIN_L)?"L":"", (g_gains.LRmask&GAIN_R)?"R":""); return;

	//default: return;
	}
}
*/

void DSP_rtxCallback(int steps)
{

	//SINEWAVE GEN
	if (g_selected == g_sine.mod)
	{
		static int s_gainL = 0; //used for balance
		static int s_gainR = 0;

		switch(g_sine.param) {
		case SINE_FREQ: g_sine.freq += steps * 1000; // +/- 10 Hz
						PRINTF("SINE: Frq = %d Hz\n", g_sine.freq/100);
						break;
		case SINE_VOL:  g_sine.gains[0] += steps * 10;  s_gainL = g_sine.gains[0];
						g_sine.gains[1] += steps * 10;  s_gainR = g_sine.gains[1];
						PRINTF("SINE: Vol: L = %d dB, R = %d dB\n", g_sine.gains[0]/10, g_sine.gains[1]/10);
						break;
		case SINE_BAL:  s_gainL -= steps * 5; if (s_gainL <= 0) g_sine.gains[0] = s_gainL;
						s_gainR += steps * 5; if (s_gainR <= 0) g_sine.gains[1] = s_gainR;
						PRINTF("SINE: Bal: L = %d dB, R = %d dB\n", g_sine.gains[0]/10, g_sine.gains[1]/10);
						break;
		}
		STA_SinewaveGenSet(g_sine.mod, g_sine.freq, g_sine.msec, g_sine.gains[0], g_sine.gains[1]);
		return;
	}

	//GAIN
	if (g_selected == g_gains.mod)
	{
		if (g_gains.LRmask & GAIN_L) {
			g_gains.L += steps * g_gains.dbStep; //gain step = +-6 dB
			STA_GainSetGains(g_gains.mod, GAIN_L, g_gains.L);
		}
		if (g_gains.LRmask & GAIN_R) {
			g_gains.R += steps * g_gains.dbStep; //gain step = +-6 dB
			STA_GainSetGains(g_gains.mod, GAIN_R, g_gains.R);
		}
		if (g_gains.dbStep >= 10)
			PRINTF("GAIN: L,R = %d, %d dB\n", g_gains.L/10, g_gains.R/10);
		else
			PRINTF("GAIN: L,R = %d, %d 0.1dB\n", g_gains.L, g_gains.R);
		return;
	}
}



#endif //0
