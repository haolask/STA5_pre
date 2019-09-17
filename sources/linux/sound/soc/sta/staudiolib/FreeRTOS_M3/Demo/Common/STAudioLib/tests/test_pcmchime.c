/*
 * STAudioLib - test_pcmchime.c
 *
 * Created on 2016/02/22 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	Testing PCM-CHIME module

	+------+  +---+
	| PCM  |--|   |  +----+
	|CHIME1|--| M |--|GAIN|--XOUT[0]
	+------+  | U |--|    |--XOUT[1]
	| PCM  |--| X |  +----+
	|CHIME2|--|   |
	+------+  +---+
*/

#if 1
#include "staudio_test.h"


static RAWINFO g_pcm1 = {"0:/A2_SONGS/click-clack_16bit_48k.wav",	48000, 16, 2, (void*)0, 0};
static RAWINFO g_pcm2 = {"0:/A2_SONGS/DA_16bit_48k.wav",			48000, 16, 2, (void*)0, 0};
//static RAWINFO g_pcm1 = {"0:/A2_SONGS/Click_Clack_Chime_33_48kHz_D3_2x16x3288.raw", 48000, 16, 2, (void*)0, 0};


//----------
// PCMCHIMES
//----------

static STA_PCMChimeParams	g_chime1Params = {
	.playCount 		= -1,		// < 0 means infinit loop
	.leftDelay 		= 0,		//msec
	.leftPostDelay	= 0,		//msec
	.rightDelay		= 500,		//msec  <-- RTX
	.rightPostDelay = 500,		//msec	<-/
};

static STA_PCMChimeParams	g_chime2Params = {
	.playCount 		= -1,		// < 0 means infinit loop
	.leftDelay 		= 0,		//msec
	.leftPostDelay	= 500,		//msec
	.rightDelay		= 500,		//msec  <-- RTX
	.rightPostDelay = 500,		//msec	<-/
};

static int					g_chimeDelaySteps = 100; //ms

static STAModule			g_chime1, g_chime2;

//----------
// GAINS
//----------
static STAModule g_gains;
static int 		 g_gainDb = 0;//-120;		//-12 dB

//----------
// MUX
//----------
static STAModule g_mux;
static u8		 g_muxSel[2] = {0, 1};


//----------
// MODULES
//----------
//for user interaction
static STAModule	g_selMod;



//---------------------------------------------------------
// BUILD FLOW
//---------------------------------------------------------

static void DSP_Init(void)
{
	int i;

	STA_Reset();

	//PCMCHIME1
	g_chime1 = STA_AddModule(STA_DSP0, STA_PCMCHIME_12BIT_Y_2CH);
	STA_PCMSetMaxDataSize(g_chime1, g_pcm1.byteSize);

	//PCMCHIME2
	g_chime2 = STA_AddModule(STA_DSP0, STA_PCMCHIME_12BIT_X_2CH);
	STA_PCMSetMaxDataSize(g_chime2, g_pcm2.byteSize);

	//MUX
	g_mux = STA_AddModule(STA_DSP0, STA_MUX_2OUT);
	STA_MuxSet(g_mux, g_muxSel);

	//GAINS
	g_gains = STA_AddModule(STA_DSP0, STA_GAIN_STATIC_NCH);  //(2 gains by default)


	//CONNECTIONS
	FORi(2) {
		STA_Connect(g_chime1, i,   g_mux,     i);
		STA_Connect(g_chime2, i,   g_mux,     i+2);
		STA_Connect(g_mux,    i,   g_gains,   i);
		STA_Connect(g_gains,  i,   STA_XOUT0, i);
	}


	//BUILD
	STA_BuildFlow();
	ASSERT(STA_GetError() == STA_NO_ERROR);

	//POST-BUILT
	STA_PCMLoadData_16bit(g_chime1, g_pcm1.data, g_pcm1.byteSize);
	STA_PCMLoadData_16bit(g_chime2, g_pcm2.data, g_pcm2.byteSize);

	//START dsp
	STA_Play();
	SLEEP(10);					//wait 10ms for DSP initialisation

	//POST INIT
	STA_GainSetGains(g_gains, 0x1, g_gainDb);
	STA_GainSetGains(g_gains, 0x2, g_gainDb);

	g_selMod = g_chime1;

	//DMABUS
	STA_DMAReset();
	STA_DMAAddTransfer(STA_DMA_DSP0_XOUT,  	STA_DMA_SAI3_TX1,     2);	//DSP to SAI3
	STA_DMAAddTransfer(STA_DMA_SAI2_RX1, 	STA_DMA_DSP0_XIN+127, 1);	//swap trigger
}


//---------------------------------------------------------
// toggle chime
//---------------------------------------------------------
static u32 g_toggleChime[2];
static u32 g_curChime = 1;
/*
static void toggleChime1(void)
{
	g_toggleChime1  ^= 1;

	PRINTF(g_toggleChime1 ? "CLIC-CLAC ON\n" : "CLIC-CLAC OFF\n");

	if (g_toggleChime1)
		STA_PCMPlay(g_chime1, &g_chime1Params);
	else
		STA_PCMStop(g_chime1);
}
*/

static void toggleChime(u32 chime)
{
	if (chime == g_curChime)
		g_toggleChime[chime-1] ^= 1;	//toggle on/off
	else
		g_toggleChime[chime-1] = 1;		//toggle on

	g_curChime = chime;


	//CHIME1
	if (chime == 1)
	{
		PRINTF(g_toggleChime[0] ? "PCMCHIME1 ON\n" : "PCMCHIME1 OFF\n");

		//update MUX
		g_muxSel[0] = 0; g_muxSel[1] = 1;
		STA_MuxSet(g_mux, g_muxSel);

		if (g_toggleChime[0])
			STA_PCMPlay(g_chime1, &g_chime1Params);
		else
			STA_PCMStop(g_chime1);
	}
	//CHIME2
	else
	{
		PRINTF(g_toggleChime[1] ? "PCMCHIME2 ON\n" : "PCMCHIME2 OFF\n");

		//update MUX
		g_muxSel[0] = 2; g_muxSel[1] = 3;
		STA_MuxSet(g_mux, g_muxSel);

		if (g_toggleChime[1])
			STA_PCMPlay(g_chime2, &g_chime2Params);
		else
			STA_PCMStop(g_chime2);
	}

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
	/*
	[    ]	[    ]		[  ]  [  ]  [  ]  [  ]
						[  ]  [  ]  [  ]  [  ]
	[CHM1]	[CHM2]		[  ]  [  ]  [  ]  [  ]
	*/

	int key = g_key_pressed;
	g_key_pressed = 0;  //key processed

	//module selection
	switch(key) {

	//Clic-Clac
	case 0x13: 	toggleChime(1); g_selMod = g_chime1; return;
	case 0x14: 	toggleChime(2); g_selMod = g_chime2; return;

	}


}

static void process_rtx(void)
{
	int steps = g_rtx_steps;
	g_rtx_steps = 0; //rtx processed

/*
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
*/
	//CHIME1
	if (g_selMod == g_chime1)
	{
		g_chime1Params.rightDelay     += steps * g_chimeDelaySteps;
		g_chime1Params.rightPostDelay += steps * g_chimeDelaySteps;
		PRINTF("PCMCHIME1 rightDelay = %d\n", g_chime1Params.rightDelay);
		STA_PCMPlay(g_chime1, &g_chime1Params);
	}
	//CHIME2
	else if (g_selMod == g_chime2)
	{
		g_chime2Params.rightDelay     += steps * g_chimeDelaySteps;
		g_chime2Params.rightPostDelay += steps * g_chimeDelaySteps;
		PRINTF("PCMCHIME2 rightDelay = %d\n", g_chime2Params.rightDelay);
		STA_PCMPlay(g_chime2, &g_chime2Params);
	}
}


//---------------------------------------------------------
// Main
//---------------------------------------------------------

void TestPCMChime(void)
{
	int nch = 2;

	PRINTF("TestPCMChime... \n");

	//load the clicclac song
	load_song(&g_pcm1, 0, 0);
	load_song(&g_pcm2, 0, 0);

	//user control
	RTX_Init(rtxCallback);
	KPD_Init(keyCallback);

	//audio init
	DSP_Init();

	//play
	//SAI3 (master, enable)
	AIF->SAI3_CR.REG = SAI_I2S | SAI_MST | SAI_32_24VAL | SAI_EN;
	STA_DMAStart();				//start DMABUS

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


#endif //0
