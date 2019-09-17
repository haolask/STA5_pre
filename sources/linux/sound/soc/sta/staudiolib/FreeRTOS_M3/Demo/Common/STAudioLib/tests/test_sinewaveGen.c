/*
 * STAudioLib - test_sinewaveGen.c
 *
 * Created on 2013/04/22 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	Testing SINEWAVE GENERATOR module

	+-----+  +----+
	|SINE |--|Gain|-- XOUT[0]
	|GEN  |--|    |-- XOUT[1]
	+-----+  +----+


 */

#if 1
#include "staudio_test.h"

#define STA_DSPN STA_DSP2
#define STA_XOUTN STA_XOUT2
#define STA_DMA_DSPN_XIN STA_DMA_DSP2_XIN
#define STA_DMA_DSPN_XOUT STA_DMA_DSP2_XOUT

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
	.gains 		= {0,0},
	.freq		= 44000, //440 Hz
	.msec		= -1, //10000, //10 sec
	.param		= SINE_FREQ,
};

//---------
// MODULES
//---------
//for user interaction
static SINEWAVEGEN 	g_sine;
static STAModule 	g_gains;

static STAModule	g_selected;	//user selected module


//---------------------------------------------------------
// BUILD FLOW
//---------------------------------------------------------

static void ResetSinewaveGen(SINEWAVEGEN* sine)
{
	STAModule mod = sine->mod; 				//bak handle
	*sine = g_sineInit; sine->mod = mod;	//reset

	STA_SetMode(mod, STA_SINE_ON);
	STA_SinewaveGenSet(mod, sine->freq, sine->msec, sine->gains[0], sine->gains[1]);
}


//---------------------------------------------------------

static void DSP_Init(void)
{
	int i;
	u32 xmemsize;

	STA_Reset();

	//SINEWAVE
	g_sine.mod = STA_AddModule(STA_DSPN, STA_SINE_2CH);
	ResetSinewaveGen(&g_sine);

	//GAINS
	g_gains = STA_AddModule(STA_DSPN, STA_GAIN_STATIC_NCH); //2 gains by default

	//CONNECTIONS
//	STA_Connect(g_sine.mod, 0,  STA_XOUTN, 0);
//	STA_Connect(g_sine.mod, 1,  STA_XOUTN, 1);
	FORi(2) {
        STA_Connect(g_sine.mod, i,  g_gains, i);
        STA_Connect(g_gains, i,  STA_XOUTN, i);
    }

	STA_GetDspMemCost(STA_DSPN, &xmemsize, 0);

	//BUILD
	STA_BuildFlow();

	ASSERT(STA_GetError() == STA_NO_ERROR);

	//START DSP
	STA_Play();
	SLEEP(5);					//wait 5ms for DSP initialisation

	//POST-INIT
//	ResetSinewaveGen(&g_sine);
	g_selected = g_sine.mod;
}


static void AUD_Routing(int nch)
{
	//DMABUS
	STA_DMAReset();
	STA_DMAAddTransfer(STA_DMA_SAI2_RX1,	STA_DMA_DSPN_XIN, nch);		//SAI2 to XIN
	if (nch == 1)
		STA_DMAAddTransfer(STA_DMA_DSPN_XIN, STA_DMA_DSPN_XIN+1, 1); 	//R (copy from L)
	STA_DMAAddTransfer(STA_DMA_DSPN_XOUT,  	STA_DMA_SAI3_TX1,    2);	//XOUT to SAI3
	STA_DMAAddTransfer(STA_DMA_SAI2_RX1, 	STA_DMA_DSPN_XIN+127, 1);	//swap trigger
}

//---------------------------------------------------------
// KEY / ROTARY
//---------------------------------------------------------

static void rtxCallback(int steps)
{
	static int s_gainL = 0; //used for balance
	static int s_gainR = 0;

	//SINEWAVE GEN
	if (g_selected == g_sine.mod) {
		switch(g_sine.param) {
		case SINE_FREQ: g_sine.freq += steps * 1000; // +/- 10 Hz
						PRINTF("SINEWAVE GEN: Frq = %d Hz\n", g_sine.freq/100);
						break;
		case SINE_VOL:  g_sine.gains[0] += steps * 10;  s_gainL = g_sine.gains[0];
						g_sine.gains[1] += steps * 10;  s_gainR = g_sine.gains[1];
						PRINTF("SINEWAVE GEN: Vol: L = %d dB, R = %d dB\n", g_sine.gains[0]/10, g_sine.gains[1]/10);
						break;
		case SINE_BAL:  s_gainL -= steps * 5; if (s_gainL <= 0) g_sine.gains[0] = s_gainL;
						s_gainR += steps * 5; if (s_gainR <= 0) g_sine.gains[1] = s_gainR;
						PRINTF("SINEWAVE GEN: Bal: L = %d dB, R = %d dB\n", g_sine.gains[0]/10, g_sine.gains[1]/10);
						break;
		}
		STA_SinewaveGenSet(g_sine.mod, g_sine.freq, g_sine.msec, g_sine.gains[0], g_sine.gains[1]);
		return;
	}
}

static void keyCallback(int key, bool down)
{
	//module selection
	//row1
//	switch(key) {
//	case 0x13:	g_selected = g_gains.mod; PRINTF("GAIN  L = %d dB,  R = %d dB\n", g_gains.L/10, g_gains.R/10); return;
//	case 0x14:	g_selected = g_delay.mod; PRINTF("DELAY L = %d,  R = %d samples\n", g_delay.delays[0], g_delay.delays[1]); return;
//	}

	//SINEWAVE GEN
	if (g_selected == g_sine.mod) {
		switch(key) {
//		case 0x21:  g_sine.onoff ^= 1; STA_SetMode(g_sine.mod, g_sine.onoff); PRINTF("SINEWAVE GEN: %s\n", g_sine.onoff ? "ON":"OFF"); return;
		case 0x22:  g_sine.param = SINE_FREQ;  PRINTF("SINEWAVE GEN: Frq = %d Hz\n", g_sine.freq/100); return;
		case 0x23:  g_sine.param = SINE_VOL;   PRINTF("SINEWAVE GEN: Vol: L = %d dB, R = %d dB\n", g_sine.gains[0]/10, g_sine.gains[1]/10); return;
		case 0x24:  g_sine.param = SINE_BAL;   PRINTF("SINEWAVE GEN: Bal: L = %d dB, R = %d dB\n", g_sine.gains[0]/10, g_sine.gains[1]/10); return;
		case 0x41:  ResetSinewaveGen(&g_sine); PRINTF("SINEWAVE GEN: Reset\n"); return;
		default: return;
		}
		return;
	}

}

//---------------------------------------------------------
// Main
//---------------------------------------------------------

void TestSinewaveGen(void)
{
	int nch = 2;

	PRINTF("TestSinewaveGen (DSP%d)... \n", STA_DSPN);

	//user control
	RTX_Init(rtxCallback);
	KPD_Init(keyCallback);

	//audio init
	DSP_Init();
	AUD_Routing(nch);

	//play (in loop)

	//SAI3 (master, enable)
	AIF->SAI3_CR.REG = SAI_I2S | SAI_MST | SAI_32_24VAL | SAI_RL | SAI_EN;

	STA_DMAStart();				//start DMABUS

	STA_SinewaveGenSet(g_sine.mod, g_sine.freq, g_sine.msec, g_sine.gains[0], g_sine.gains[1]);

	while (1) {
		//Test_Play_PCM_48Khz_16bit(g_songAddr, g_songSize, nch);
//		STA_SinewaveGenSet(g_sine.mod, g_sine.freq, g_sine.msec, g_sine.gains[0], g_sine.gains[1]);
//		SLEEP(g_sine.msec - 10);
		SLEEP(MIN(g_sine.msec - 10, 100));
	}

	//stop and clean
	STA_Reset();
//	PRINT_OK_FAILED;
}

#endif //0
