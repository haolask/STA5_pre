/*
 * STAudioLib - test_delay.c
 *
 * Created on 2013/04/22 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	Testing DELAY module

	            GAINS      DELAY
	          +-------+  +-------+
	L XIN[0]--| Gain0 |--| DL0   |--XOUT[0]
	R XIN[1]--| Gain1 |--| DL1   |--XOUT[1]
	          +-------+  +-------+


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


#define MASK_L	0x1
#define MASK_R	0x2

//---------
// GAINS
//---------
#define GAIN_L	0x1
#define GAIN_R	0x2

typedef struct {
	STAModule 	mod;
	u32			LRmask;
	s16			L, R;		//controlable
} GAIN;

static const GAIN g_gainsInit = {
	.mod 		= 0,
	.LRmask		= GAIN_L|GAIN_R,
	.L 			= 0,//-150,	//-15dB when using BOSE headphone
	.R			= 0,//-150,
};

//---------
// DELAY
//---------
typedef struct {
	STAModule 		mod;
	STA_ModuleType	type;
	int 			lengths[2];
	int				delays[2];
	int				delaySteps;
	int				delayRvsL;
} DELAY;

static const DELAY g_delayInit = {
	.mod 		= 0,
	.type		= STA_DLAY_X_2CH,
	.lengths	= {1000, 1000},
	.delays		= {0, 0},
	.delaySteps = 1,
	.delayRvsL  = 0,
};

//---------
// MODULES
//---------
//for user interaction
static GAIN 		g_gains;
static DELAY		g_delay;

static STAModule	g_selected;	//user selected module


//---------------------------------------------------------
// BUILD FLOW
//---------------------------------------------------------

static void ResetGains(GAIN* gains)
{
	gains->LRmask 	= g_gainsInit.LRmask;
	gains->L 		= g_gainsInit.L;
	gains->R 		= g_gainsInit.R;

	STA_GainSetGains(gains->mod, GAIN_L, gains->L);
	STA_GainSetGains(gains->mod, GAIN_R, gains->R);
}

static void ResetDelay(DELAY* delay)
{
	STAModule mod = delay->mod; //bak
	*delay = g_delayInit; delay->mod = mod; //reset

	delay->delayRvsL = delay->delays[1] - delay->delays[0];

	STA_DelaySetDelays(delay->mod, MASK_L, delay->delays[0]);
	STA_DelaySetDelays(delay->mod, MASK_R, delay->delays[1]);
}

//---------------------------------------------------------

static void DSP_Flow(void)
{
	STAModule gains, delay;
	int i;

	STA_Reset();

	//GAINS
	g_gains = g_gainsInit;
	g_gains.mod = gains = STA_AddModule(DSP_SEL, STA_GAIN_STATIC_NCH);  //2 gains by default

	//DELAY
	g_delay = g_delayInit;
	g_delay.mod = delay = STA_AddModule(DSP_SEL, g_delayInit.type);
	STA_DelaySetLengths(g_delay.mod, MASK_L, g_delay.lengths[0]);
	STA_DelaySetLengths(g_delay.mod, MASK_R, g_delay.lengths[1]);

	//CONNECTIONS
	FORi(2) {
#if 0
		STA_Connect(MOD_XIN,  i,  gains,     i);
		STA_Connect(gains,    i,  delay,     i);
		STA_Connect(delay,    i,  MOD_XOUT,  i);

//		STA_Connect(MOD_XIN,  i,  gains,     i);
//		STA_Connect(gains,    i,  MOD_XOUT,  i);
#else
		STA_Connect(MOD_XIN,  i,  delay,     i);
		STA_Connect(delay,    i,  MOD_XOUT,  i);
#endif
//		STA_Connect(MOD_XIN,  i,  MOD_XOUT,  i);	//(shortcut all effects) for debug
	}

	//BUILD
	STA_BuildFlow();

	ASSERT(STA_GetError() == STA_NO_ERROR);
}


static void AUD_Routing(int nch)
{
	//DMABUS
	STA_DMAReset();
	STA_DMAAddTransfer(STA_DMA_SAI2_RX1,	DMA_XIN, nch);			//SAI2 to XIN
	if (nch == 1)
		STA_DMAAddTransfer(DMA_XIN, 		DMA_XIN+1, 1); 			//R (copy from L)
	STA_DMAAddTransfer(DMA_XOUT,  			STA_DMA_SAI3_TX1, 2);	//XOUT to SAI3
	STA_DMAAddTransfer(STA_DMA_SAI2_RX1, 	DMA_XIN+127, 1);		//swap trigger
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

static void process_key(void)
{
	int key = g_key_pressed;
	g_key_pressed = 0;  //key processed

	//module selection
	//row1
	switch(key) {
	case 0x13:	g_selected = g_gains.mod; PRINTF("GAIN  L = %d dB,  R = %d dB\n", g_gains.L/10, g_gains.R/10); return;
	case 0x14:	g_selected = g_delay.mod; PRINTF("DELAY L = %d,  R = %d samples\n", g_delay.delays[0], g_delay.delays[1]); return;
	}

	//GAIN
	//col1:
	if (g_selected == g_gains.mod) {
		switch(key) {
		case 0x21:  g_gains.LRmask ^= GAIN_R; break;//PRINTF("R %s\n", (g_gains.LRmask&GAIN_R)?"Unlocked":"Locked"); break;
		case 0x31:  g_gains.LRmask ^= GAIN_L; break;//PRINTF("L %s\n", (g_gains.LRmask&GAIN_L)?"Unlocked":"Locked"); break;
		case 0x41:  ResetGains(&g_gains); PRINTF("GAIN L,R = %d, %d dB\n", g_gains.L/10, g_gains.R/10); return;
		default: return;
		}
		PRINTF("Change %s%s\n", (g_gains.LRmask&GAIN_L)?"L":"", (g_gains.LRmask&GAIN_R)?"R":"");
		return;
	}

	//DELAY
	if (g_selected == g_delay.mod) {
		switch(key) {
		case 0x41:  ResetDelay(&g_delay); PRINTF("DELAY L = %d,  R = %d samples\n", g_delay.delays[0], g_delay.delays[1]); return;
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
	if (g_selected == g_gains.mod) {
		if (g_gains.LRmask & GAIN_L) {
			g_gains.L += steps * 10; //GAIN_L step = +-1 dB
			STA_GainSetGains(g_gains.mod, GAIN_L, g_gains.L);
		}
		if (g_gains.LRmask & GAIN_R) {
			g_gains.R += steps * 10; //GAIN_R step = +-1 dB
			STA_GainSetGains(g_gains.mod, GAIN_R, g_gains.R);
		}
		PRINTF("GAIN  L = %d dB,  R = %d dB\n", g_gains.L/10, g_gains.R/10);
		return;
	}

/*
	//DELAY
	if (g_selected == g_delay.mod) {
		g_delay.delays[0] += steps * g_delay.delaySteps; //DELAY step: +-20 samples
//		g_delay.delays[1] += steps * 20;
		STA_DelaySetDelays(g_delay.mod, MASK_L, g_delay.delays[0]);
//		STA_DelaySetDelays(g_delay.mod, MASK_R, g_delay.delays[1]);
		PRINTF("DELAY L = %d,  R = %d samples\n", g_delay.delays[0], g_delay.delays[1]);
		return;
	}
*/

	//DELAY
	if (g_selected == g_delay.mod)
	{
		int delta = steps * g_delay.delaySteps;

		if (g_delay.delayRvsL >= 0)
		{
			g_delay.delayRvsL += delta;
			g_delay.delays[1] += MAX(delta, -g_delay.delays[1]);
			g_delay.delays[0]  = g_delay.delays[1] - g_delay.delayRvsL;
		}
		else
		{
			g_delay.delayRvsL += delta;
			g_delay.delays[0] += MAX(-delta, -g_delay.delays[0]);
			g_delay.delays[1]  = g_delay.delays[0] + g_delay.delayRvsL;
		}
		STA_DelaySetDelays(g_delay.mod, MASK_L, g_delay.delays[0]);
		STA_DelaySetDelays(g_delay.mod, MASK_R, g_delay.delays[1]);

		PRINTF("DELAY L = %d,  R = %d samples\n", g_delay.delays[0], g_delay.delays[1]);
		return;
	}
}

//---------------------------------------------------------
// Main
//---------------------------------------------------------

void TestDelay(void)
{
	int nch = 2;

	PRINTF("TestDelay... \n");

	//user control
	RTX_Init(rtxCallback);
	KPD_Init(keyCallback);

	//audio init
	DSP_Flow();
	AUD_Routing(nch);

	//start dsp
	STA_Play();
	SLEEP(100);					//wait for DSP initialisation

	//post initialisation
	ResetGains(&g_gains);
	ResetDelay(&g_delay);
	g_selected = g_delay.mod;

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
//	PRINT_OK_FAILED;
}

#endif //0
