/*
 * STAudioLib - test_square_signal.c
 *
 * Created on 2017/06/13 by Christophe Quarre
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

//-------------
// SINEWAVE GEN
//-------------
enum SINE_PARAM {SINE_FREQ, SINE_SAT, SINE_VOL};

typedef struct {
	STAModule 	mod;
	s16			gains[2];		//saturation gain, vol
	u32			freq;			//user
	u32			msec;
	enum SINE_PARAM param;		//selected param to update
} SINEWAVEGEN;

static const SINEWAVEGEN g_sineInit = {
	.mod 		= 0,
//	.gains 		= {0,0},
	.gains 		= {0,-120},
	.freq		= 4000, //440 Hz
	.msec		= -1, //10000, //10 sec
	.param		= SINE_VOL,
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

static void print(void)
{
	switch(g_sine.param) {
	case SINE_FREQ:
		PRINTF("SINEWAVE: [F= %d Hz]  Sat= %d dB   Vol= %d dB\n", g_sine.freq/100, g_sine.gains[0]/10, g_sine.gains[1]/10);
		break;

	case SINE_SAT:
		PRINTF("SINEWAVE:  F= %d Hz  [Sat= %d dB]  Vol= %d dB\n", g_sine.freq/100, g_sine.gains[0]/10, g_sine.gains[1]/10);
		break;

	case SINE_VOL:
		PRINTF("SINEWAVE:  F= %d Hz   Sat= %d dB  [Vol= %d dB]\n", g_sine.freq/100, g_sine.gains[0]/10, g_sine.gains[1]/10);
		break;
	}
}

static void ResetSinewaveGen(SINEWAVEGEN* sine)
{
	STAModule mod = sine->mod; 				//bak handle
	*sine = g_sineInit; sine->mod = mod;	//reset

	STA_SetMode(mod, STA_SINE_ON);
	STA_SinewaveGenSet(mod, sine->freq, sine->msec, 0, 0);
	print();
}


//---------------------------------------------------------

static void DSP_Init(void)
{
	int i;

	STA_Reset();

	//SINEWAVE
	g_sine.mod = STA_AddModule(STA_DSP0, STA_SINE_2CH);
	ResetSinewaveGen(&g_sine);

	//GAINS
	g_gains = STA_AddModule(STA_DSP0, STA_GAIN_STATIC_NCH); //2 gains by default

	//CONNECTIONS
	STA_Connect(g_sine.mod, 0,  g_gains, 0);
	STA_Connect(g_gains, 0,     g_gains, 1);
	STA_Connect(g_gains, 1,     STA_XOUT0, 0);
	STA_Connect(g_gains, 1,     STA_XOUT0, 1);
/*
	STA_Connect(g_sine.mod, 0,  g_gains, 1);
	STA_Connect(g_gains, 1,     STA_XOUT0, 0);
	STA_Connect(g_gains, 1,     STA_XOUT0, 1);
*/
/*
	STA_Connect(g_sine.mod, 0,  STA_XOUT0, 0);
	STA_Connect(g_sine.mod, 0,  STA_XOUT0, 1);
*/

	//BUILD
	STA_BuildFlow();

	ASSERT(STA_GetError() == STA_NO_ERROR);

	//START DSP
	STA_Play();
	SLEEP(5);					//wait 5ms for DSP initialisation

	//POST-INIT
//	STA_GainSetLinearGainsAndShifts(g_gains, 0x1, 0xff800000, 23);
	STA_GainSetGain(g_gains, 0, g_sine.gains[0]);
	STA_GainSetGain(g_gains, 1, g_sine.gains[1]);

//	ResetSinewaveGen(&g_sine);
	g_selected = g_sine.mod;
}


static void AUD_Routing(int nch)
{
	//DMABUS
	STA_DMAReset();
	STA_DMAAddTransfer(STA_DMA_SAI2_RX1,	STA_DMA_DSP0_XIN, nch);		//SAI2 to XIN
	if (nch == 1)
		STA_DMAAddTransfer(STA_DMA_DSP0_XIN, STA_DMA_DSP0_XIN+1, 1); 	//R (copy from L)
	STA_DMAAddTransfer(STA_DMA_DSP0_XOUT,  	STA_DMA_SAI3_TX1,    2);	//XOUT to SAI3
	STA_DMAAddTransfer(STA_DMA_SAI2_RX1, 	STA_DMA_DSP0_XIN+127, 1);	//swap trigger
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

static void process_rtx(void)
{
	int steps = g_rtx_steps;
	g_rtx_steps = 0; //rtx processed


	//SINEWAVE GEN
	if (g_selected == g_sine.mod)
	{
		switch(g_sine.param) {
		case SINE_FREQ:
			g_sine.freq += steps * 1000; // +/- 10 Hz
			STA_SinewaveGenSet(g_sine.mod, g_sine.freq, g_sine.msec, 0, 0);
			break;

		case SINE_SAT:
			g_sine.gains[0] += steps * 10;
			STA_GainSetGain(g_gains, 0, g_sine.gains[0]);
			break;

		case SINE_VOL:
			g_sine.gains[1] += steps * 10;
			STA_GainSetGain(g_gains, 1, g_sine.gains[1]);
			break;
		}

		//STA_SinewaveGenSet(g_sine.mod, g_sine.freq, g_sine.msec, g_sine.gains[0], g_sine.gains[1]);
		print();

		return;
	}
}

static void process_key()
{
	int key = g_key_pressed;
	g_key_pressed = 0;  //key processed

	//module selection
	//row1
//	switch(key) {
//	case 0x13:	g_selected = g_gains.mod; PRINTF("GAIN  L = %d dB,  R = %d dB\n", g_gains.L/10, g_gains.R/10); return;
//	case 0x14:	g_selected = g_delay.mod; PRINTF("DELAY L = %d,  R = %d samples\n", g_delay.delays[0], g_delay.delays[1]); return;
//	}

	//SINEWAVE GEN
	if (g_selected == g_sine.mod)
	{
		switch(key) {
//		case 0x21:  g_sine.onoff ^= 1; STA_SetMode(g_sine.mod, g_sine.onoff); PRINTF("SINEWAVE GEN: %s\n", g_sine.onoff ? "ON":"OFF"); return;
		case 0x21:  g_sine.param = SINE_FREQ; break;
		case 0x23:  g_sine.param = SINE_SAT; break;
		case 0x24:  g_sine.param = SINE_VOL; break;
		case 0x41:  ResetSinewaveGen(&g_sine); return;
		default: return;
		}
		print();
		return;
	}

}

//---------------------------------------------------------
// Main
//---------------------------------------------------------

void TestSquareSignal(void)
{
	int nch = 2;

	PRINTF("TestSquareSignal... \n");

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

	STA_SinewaveGenSet(g_sine.mod, g_sine.freq, g_sine.msec, 0, 0);

	while (1)
	{

		if (g_key_pressed)
			process_key();

		if (g_rtx_steps)
			process_rtx();

		SLEEP(MIN(g_sine.msec - 10, 100));
	}

	//stop and clean
	STA_Reset();
//	PRINT_OK_FAILED;
}

#endif //0
