/*
 * STAudioLib - test_polychime.c
 *
 * Created on 2016/02/22 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	Testing POLY-CHIME module

	+------+  +---+
	|      |--|   |  +----+
	|CHIME1|  | M |--|GAIN|--XOUT[0]
	+------+  | I |--|    |--XOUT[1]
	|      |--| X |  +----+
	|CHIME2|  |   |
	+------+  +---+
*/

#if 1
#include "staudio_test.h"

//enum {MONOCHIME, BEEP, POLYCHIME};

//--------------------
// BEEP / CHIMES DATA
//-------------------
#define FREQ1	440*100
#define FREQ2	840*100

//---- beep 1 --------

static STA_RampParams g_beep1_ramps[] = {
	{.type= STA_RAMP_EXP,  .ampl= 1000,  .msec= 100,  .freq= FREQ1},
	{.type= STA_RAMP_STC,  .ampl= 1000,  .msec= 200,  .freq= FREQ1},
	{.type= STA_RAMP_EXP,  .ampl=    0,  .msec= 100,  .freq= FREQ1},
};

static STA_ChimeParams	g_beep1 = {
	.ramps				= g_beep1_ramps,
	.numRamps			= 3,
	.repeatCount		= 4,
	.postRepeatRampIdx	= (u16)-1, //no post repeat ramps
};

//---- beep 2 --------

static STA_RampParams g_beep2_ramps[] = {
	{.type= STA_RAMP_EXP,  .ampl= 1000,  .msec= 100,  .freq= FREQ2},
	{.type= STA_RAMP_STC,  .ampl= 1000,  .msec= 200,  .freq= FREQ2},
	{.type= STA_RAMP_EXP,  .ampl=    0,  .msec= 100,  .freq= FREQ2},
};

static STA_ChimeParams	g_beep2 = {
	.ramps				= g_beep2_ramps,
	.numRamps			= 3,
	.repeatCount		= 2,
	.postRepeatRampIdx	= (u16)-1, //no post repeat ramps
};

//---- beep 3 --------

static STA_BeepParams g_beep3 = {
	//--- same as sinewaveGen-----
	.freq        = 650*100,
	.peakTime    = 1,
	.peakAmpl    = 1000,
    //--- ¡°beep¡± params ----------
	.atkType     = STA_RAMP_EXP,
	.atkTime     = 2,
	.relType     = STA_RAMP_EXP,
	.relTime     = 100,
	.muteTime    = 0,
	.repeatCount = 4
};

//---- Gong --------

#define GONG_FREQ	750*100
#define GONG_FREQ2	500*100
#if 0
static STA_RampParams g_gong_ramps[] = {
	{.type= STA_RAMP_LIN,  .ampl= 800,  .msec=    2,  .freq= GONG_FREQ},
	{.type= STA_RAMP_EXP,  .ampl=   0,  .msec=  500,  .freq= GONG_FREQ},
	//post repeat
	{.type= STA_RAMP_LIN,  .ampl= 800,  .msec=    2,  .freq= GONG_FREQ},
	{.type= STA_RAMP_EXP,  .ampl=   0,  .msec= 1000,  .freq= GONG_FREQ},
};
#else
static STA_RampParams g_gong_ramps[] = {
	{.type= STA_RAMP_LIN,  .ampl= 800,  .msec=    0,  .freq= GONG_FREQ},  // add dummy ramp with duration= 0 ms to test its removal
	{.type= STA_RAMP_LIN,  .ampl= 800,  .msec=    2,  .freq= GONG_FREQ},
	{.type= STA_RAMP_LIN,  .ampl= 800,  .msec=    0,  .freq= GONG_FREQ},  // add dummy ramp with duration= 0 ms to test its removal 
	{.type= STA_RAMP_EXP,  .ampl=   0,  .msec= 0,  .freq= GONG_FREQ},  // add dummy ramp with duration= 0 ms to test its removal 
	{.type= STA_RAMP_EXP,  .ampl=   0,  .msec= 1200,  .freq= GONG_FREQ},
	{.type= STA_RAMP_EXP,  .ampl=   0,  .msec= 0,  .freq= GONG_FREQ},  // add dummy ramp with duration= 0 ms to test its removal 
	//post repeat
	{.type= STA_RAMP_LIN,  .ampl= 800,  .msec=    0,  .freq= GONG_FREQ},  // add dummy ramp with duration= 0 ms to test its removal 
	{.type= STA_RAMP_LIN,  .ampl= 800,  .msec=    2,  .freq= GONG_FREQ},
	{.type= STA_RAMP_EXP,  .ampl=   0,  .msec= 4800,  .freq= GONG_FREQ},
	{.type= STA_RAMP_EXP,  .ampl=   0,  .msec= 0,  .freq= GONG_FREQ},  // add dummy ramp with duration= 0 ms to test its removal 
};
#endif
static STA_ChimeParams	g_gong = {
	.ramps				= g_gong_ramps,
	.numRamps			= 10,
	.repeatCount		= 4,
	.postRepeatRampIdx	= 6, //post repeat ramp
};

//---- Beltminder B --------

static STA_RampParams g_beltminderB_ramps[] = {
	//pulse1
	{.type= STA_RAMP_LIN,  .ampl= 1000,  .msec=   5,  .freq= 740*100},
	{.type= STA_RAMP_STC,  .ampl= 1000,  .msec= 125,  .freq= 740*100},
	{.type= STA_RAMP_LIN,  .ampl=  500,  .msec= 230,  .freq= 740*100},
	//pulse2
	{.type= STA_RAMP_LIN,  .ampl= 1000,  .msec=   5,  .freq= 740*100},
	{.type= STA_RAMP_STC,  .ampl= 1000,  .msec= 135,  .freq= 740*100},
	{.type= STA_RAMP_LIN,  .ampl=  500,  .msec= 430,  .freq= 740*100},
	//post repeat
	{.type= STA_RAMP_LIN,  .ampl=    0,  .msec= 370,  .freq= 740*100},
};

static STA_ChimeParams	g_beltminderB = {
	.ramps				= g_beltminderB_ramps,
	.numRamps			= 7,
	.repeatCount		= 3,
	.postRepeatRampIdx	= 6,
};
//---- Polychime (2 gongs) ---------------------

static STA_RampParams g_polygong_ramps1[] = { 	//<---  master chime because the longest
	{.type= STA_RAMP_LIN,  .ampl= 1000,  .msec=   2,  .freq= GONG_FREQ},
	{.type= STA_RAMP_EXP,  .ampl=    0,  .msec= 400,  .freq= GONG_FREQ},
	{.type= STA_RAMP_STC,  .ampl=    0,  .msec= 400,  .freq= GONG_FREQ},
	//post repeat
	{.type= STA_RAMP_LIN,  .ampl= 1000,  .msec=    2, .freq= GONG_FREQ},
	{.type= STA_RAMP_EXP,  .ampl=    0,  .msec= 4800, .freq= GONG_FREQ},
};

static STA_RampParams g_polygong_ramps2[] = {
	{.type= STA_RAMP_STC,  .ampl=   0,  .msec= 400,  .freq= GONG_FREQ2},
    {.type= STA_RAMP_LIN,  .ampl= 800,  .msec=   2,  .freq= GONG_FREQ2},
	{.type= STA_RAMP_EXP,  .ampl=   0,  .msec= 400,  .freq= GONG_FREQ2},
	{.type= STA_RAMP_STC,  .ampl=   0,  .msec=   1,  .freq= GONG_FREQ2}, //<-- this STC ramp is useless, but we keep for testing against bug
	//post repeat
	{.type= STA_RAMP_LIN,  .ampl= 800,  .msec=    2, .freq= GONG_FREQ2},
	{.type= STA_RAMP_EXP,  .ampl=   0,  .msec= 4800, .freq= GONG_FREQ2},
};

static STA_ChimeParams	g_polygong1 = {
	.ramps				= g_polygong_ramps1,
	.numRamps			= 5,
	.repeatCount		= 4,
	.postRepeatRampIdx	= 3, //post repeat ramp
};

static STA_ChimeParams	g_polygong2 = {
	.ramps				= g_polygong_ramps2,
	.numRamps			= 6,
	.repeatCount		= 4,
	.postRepeatRampIdx	= 4, //post repeat ramp
};


//-----------------
// Chime Generators
//----------------
//STOP RAMP
#define STOP_RAMP_TYPE		STA_RAMP_LIN
#define STOP_RAMP_TIME		200

static STAModule			g_chimeGen1, g_chimeGen2;

//----------
// GAINS
//----------
static STAModule g_gains;
static int 		 g_gainDb = -120;//-120;		//note: set -12 dB to avoid saturation when rec on PC

//----------
// MIXER
//----------
static STAModule g_mix;


//----------
// USER Control
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

	//CHIMEGEN1
	g_chimeGen1 = STA_AddModule(STA_DSP0, STA_CHIMEGEN);
	STA_ChimeGenSetMaxNumRamps(g_chimeGen1, 20); //max 20 ramps

	//CHIMEGEN2
	g_chimeGen2 = STA_AddModule(STA_DSP0, STA_CHIMEGEN);
	STA_ChimeGenSetMaxNumRamps(g_chimeGen2, 20); //max 20 ramps

	//MIXER
	g_mix = STA_AddModule(STA_DSP0, STA_MIXE_2INS_NCH);

	//GAINS
	g_gains = STA_AddModule(STA_DSP0, STA_GAIN_STATIC_NCH);  //(2 gains by default)


	//CONNECTIONS
	STA_Connect(g_chimeGen1, 0,   g_mix,     0);
	STA_Connect(g_chimeGen2, 0,   g_mix,     1);
	FORi(2) {
	STA_Connect(g_mix,    i,   g_gains,   i);
	STA_Connect(g_gains,  i,   STA_XOUT0, i);
	}

	//BUILD
	STA_BuildFlow();
	ASSERT(STA_GetError() == STA_NO_ERROR);

	//POST-BUILT
	STA_MixerSetInGains(g_mix, 0x3, 0x3, 0);

	//START dsp
	STA_Play();
	SLEEP(10);					//wait 10ms for DSP initialisation

	//POST INIT
	STA_GainSetGains(g_gains, 0x1, g_gainDb);
	STA_GainSetGains(g_gains, 0x2, g_gainDb);

	//DMABUS
	STA_DMAReset();
	STA_DMAAddTransfer(STA_DMA_DSP0_XOUT,  	STA_DMA_SAI3_TX1,     2);	//DSP to SAI3
	STA_DMAAddTransfer(STA_DMA_SAI2_RX1, 	STA_DMA_DSP0_XIN+127, 1);	//swap trigger

	//current selected module
	g_selMod = g_chimeGen1;
}


//---------------------------------------------------------
// toggle chime
//---------------------------------------------------------

static void toggleChime(STAModule chimeGen, const void *chime, bool isbeep, STAModule master)
{
	static u32 s_toggleChime[3];

	u32 chimeGenId = (chimeGen == g_chimeGen1) ? 1 :
					 (chimeGen == g_chimeGen2) ? 2 :
											     3 ;
	int on = s_toggleChime[chimeGenId-1] ^= 1;	//toggle on/off

	PRINTF(on?"Play CHIMEGEN_%d\n":"Stop CHIMEGEN_%d\n", chimeGenId);

	//set the master chime gen (or unset)
	STA_ChimeGenSetParam(chimeGen, STA_CHIME_MASTER_CHIME, master);

	//on
	if (on)
	{
		if (isbeep)
			STA_ChimeGenBeep(chimeGen, chime);
		else
			STA_ChimeGenPlay(chimeGen, chime);
	}
	//off
	else
	{
		STA_ChimeGenStop(chimeGen, STOP_RAMP_TYPE, STOP_RAMP_TIME);
	}

	//selected
	g_selMod = chimeGen;
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
	[CHM1]	[CHM2]		[CHM3]  [  ]  [  ]  [  ]
	*/

	int key = g_key_pressed;
	g_key_pressed = 0;  //key processed

	//module selection
	switch(key)
	{
	//SIMPLE CHIMES
	case 0x31: 	toggleChime(g_chimeGen1, &g_gong, FALSE, 0); return;
	case 0x33: 	toggleChime(g_chimeGen1, &g_beltminderB, FALSE, 0); return;

	//BEEP
	case 0x34: 	toggleChime(g_chimeGen1, &g_beep3, TRUE, 0); return;

	//POLYCHIME
	case 0x41:
		toggleChime(g_chimeGen1, &g_polygong1, FALSE, 0);
		toggleChime(g_chimeGen2, &g_polygong2, FALSE, g_chimeGen1);
		return;

	case 0x42:
		//test soft stop synchro of polychime (just stop the master)
		STA_ChimeGenSetParam(g_chimeGen1, STA_CHIME_REPEAT_COUNT, 0);

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
/*
	//CHIME1
	if (g_selMod == g_chimeGen1)
	{
		PRINTF("PCMCHIME1 rightDelay = %d\n", g_chime1Params.rightDelay);
		STA_ChimeGenPlay(g_chimeGen1, &g_chime1Params);
	}
	//CHIME2
	else if (g_selMod == g_chimeGen2)
	{
		g_chime2Params.rightDelay     += steps * g_chimeDelaySteps;
		g_chime2Params.rightPostDelay += steps * g_chimeDelaySteps;
		PRINTF("PCMCHIME2 rightDelay = %d\n", g_chime2Params.rightDelay);
		STA_PCMPlay(g_chimeGen2, &g_chime2Params);
	}
*/
}


//---------------------------------------------------------
// Main
//---------------------------------------------------------

void TestPolyChime(void)
{
	PRINTF("TestPolyChime... \n");

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
