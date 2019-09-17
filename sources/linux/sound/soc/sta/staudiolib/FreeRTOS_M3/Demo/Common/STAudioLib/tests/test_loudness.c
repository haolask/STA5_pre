/*
 * STAudioLib - test_loudness.c
 *
 * Created on 2013/04/22 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	Testing Loudness module (static stereo)

	          GAINS      LOUD(*)
	        +-------+  +-------+
	XIN[0]--| Gain0 |--| Loud0 |--XOUT[0]
	XIN[1]--| Gain1 |--| Loud1 |--XOUT[1]
	        +-------+  +-------+

	(*) LOUD_STATIC_STEREO can't be bound to MIXER

 */

#if 1
#include "staudio_test.h"

#define BGS					STA_BIQ_GAIN_SCALE /*internal alias*/

#define DSP_SEL				0

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
	.L 			= -150,	//-15dB when using BOSE headphone
	.R			= -150,
};

//---------
// LOUDNESS
//---------
typedef struct {
	STAModule 	mod;
	u8			ON;
	u8			band;
	u8			param;
	s16			bassGQF[3]; //controlable
	s16			trebGQF[3]; //controlable
} LOUDNESS;

static const LOUDNESS g_loudnInit = {
	.mod 		= 0,
	.ON			= 0,
	.band		= 0,
	.param		= 0,
	.bassGQF	= {0, 30, 200},
	.trebGQF	= {0, 30, 10000},
};

//---------
// MODULES
//---------
//for user interaction
static STAModule	g_selMod;
static LOUDNESS		g_loudn;
static GAIN 		g_gains;

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

static void ResetLoudness(LOUDNESS* loud)
{
	STAModule mod = loud->mod; //bak
	*loud = g_loudnInit; loud->mod = mod; //reset

	STA_LoudSetTypes(mod, STA_LOUD_BANDBOOST_2, STA_LOUD_BANDBOOST_2);		//bass and trebble types
	STA_LoudSetAutoBassQLevels(mod, -24*BGS, 24*BGS, 30, 30);				// as default in ADR3
	STA_SetFilterv(mod, STA_LOUD_BASS, loud->bassGQF);
	STA_SetFilterv(mod, STA_LOUD_TREB, loud->trebGQF);
	STA_SetMode(mod, STA_LOUD_OFF);
}

//---------------------------------------------------------

static void DSP_Flow(void)
{
	STAModule gains, loudn;
	int i;

	STA_Reset();

	//GAINS
	g_gains = g_gainsInit;
	g_gains.mod = gains = STA_AddModule(STA_DSP0, STA_GAIN_STATIC_NCH);  //2 gains by default

	//LOUDNESS
	g_loudn = g_loudnInit;
	g_loudn.mod = loudn = STA_AddModule(STA_DSP0, STA_LOUD_STATIC_STEREO_DP);

	//CONNECTIONS
	FORi(2) {
		STA_Connect(STA_XIN0, i,  gains,     i);
		STA_Connect(gains,    i,  loudn,     i);
		STA_Connect(loudn,    i,  STA_XOUT0, i);
	}

	//BUILD
	STA_BuildFlow();

	ASSERT(STA_GetError() == STA_NO_ERROR);
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
static void rtxCallback(int steps)
{
	//PRINTF("ROTARY = %d\n", steps);

	//GAIN
	if (g_selMod == g_gains.mod) {
		if (g_gains.LRmask & GAIN_L) {
			g_gains.L += steps * 10; //gain step = +-1 dB
			STA_GainSetGains(g_gains.mod, GAIN_L, g_gains.L);
		}
		if (g_gains.LRmask & GAIN_R) {
			g_gains.R += steps * 10; //gain step = +-1 dB
			STA_GainSetGains(g_gains.mod, GAIN_R, g_gains.R);
		}
		PRINTF("GAIN L,R = %d, %d dB\n", g_gains.L/10, g_gains.R/10);
		return;
	}

	//LOUDNESS
	if (g_selMod == g_loudn.mod) {
		const char* bandName[] = {"BASS", "TREB"};
		s16* GQF = (g_loudn.band == 0) ? g_loudn.bassGQF : g_loudn.trebGQF;

		switch (g_loudn.param) {
		case STA_GAIN:	GQF[STA_GAIN] += steps * BGS;  //gain step = +- 1 dB;
						PRINTF("%s G = %d dB\n", bandName[g_loudn.band], GQF[0]/BGS);
						break;
		case STA_QUAL:	GQF[STA_QUAL] += steps * 2;  //qual step = +- 0.2 ;
						PRINTF("%s Q = %d\n", bandName[g_loudn.band], GQF[1]);
						break;
		case STA_FREQ:	GQF[STA_FREQ] += steps * 10; //freq step = +- 10 Hz;
						PRINTF("%s F = %d Hz\n", bandName[g_loudn.band], GQF[2]);
						break;
		}
		STA_SetFilterParam(g_loudn.mod, g_loudn.band, g_loudn.param, GQF[g_loudn.param]);
		return;
	}
}


static void keyCallback(int key, bool down)
{
//	PRINTF("KEY = %x\n", key);

	//module selection
	//row1
	switch(key) {
	case 0x13:	g_selMod = g_gains.mod; PRINTF("GAIN L,R = %d, %d dB\n", g_gains.L/10, g_gains.R/10); return;
	case 0x14:	g_selMod = g_loudn.mod; PRINTF("LOUDNESS\n"); return;
	}

	//GAIN
	//col1:
	if (g_selMod == g_gains.mod) {
		switch(key) {
		case 0x21:  g_gains.LRmask ^= GAIN_R; break;//PRINTF("R %s\n", (g_gains.LRmask&GAIN_R)?"Unlocked":"Locked"); break;
		case 0x31:  g_gains.LRmask ^= GAIN_L; break;//PRINTF("L %s\n", (g_gains.LRmask&GAIN_L)?"Unlocked":"Locked"); break;
		case 0x41:  ResetGains(&g_gains); PRINTF("GAIN L,R = %d, %d dB\n", g_gains.L/10, g_gains.R/10); return;
		default: return;
		}
		PRINTF("Change %s%s\n", (g_gains.LRmask&GAIN_L)?"L":"", (g_gains.LRmask&GAIN_R)?"R":"");
		return;
	}

	//LOUDNESS
	//col1: mode
	//col2: bass
	//col3: treble
	if (g_selMod == g_loudn.mod) {
		switch(key) {
		case 0x21:  //toggle on/off
			g_loudn.ON ^= 1; PRINTF("LOUDNESS %s\n",  g_loudn.ON ? "ON":"OFF");
			STA_SetMode(g_loudn.mod, g_loudn.ON ? STA_LOUD_MANUAL : STA_LOUD_OFF);
			break;
		case 0x22:	g_loudn.band = STA_LOUD_BASS; g_loudn.param = STA_GAIN; PRINTF("BASS G = %d dB\n", g_loudn.bassGQF[0]/BGS); break;
		case 0x32:	g_loudn.band = STA_LOUD_BASS; g_loudn.param = STA_QUAL; PRINTF("BASS Q = %d\n",    g_loudn.bassGQF[1]  ); break;
		case 0x42:	g_loudn.band = STA_LOUD_BASS; g_loudn.param = STA_FREQ; PRINTF("BASS F = %d Hz\n", g_loudn.bassGQF[2]  ); break;
		case 0x23:	g_loudn.band = STA_LOUD_TREB; g_loudn.param = STA_GAIN; PRINTF("TREB G = %d dB\n", g_loudn.trebGQF[0]/BGS); break;
		case 0x33:	g_loudn.band = STA_LOUD_TREB; g_loudn.param = STA_QUAL; PRINTF("TREB Q = %d\n",    g_loudn.trebGQF[1]  ); break;
		case 0x43:	g_loudn.band = STA_LOUD_TREB; g_loudn.param = STA_FREQ; PRINTF("TREB F = %d Hz\n", g_loudn.trebGQF[2]  ); break;
		case 0x41: 	ResetLoudness(&g_loudn); PRINTF("LOUDNESS RESETED\n"); break;
		default: return;
		}
		return;
	}
}

//---------------------------------------------------------
// Main
//---------------------------------------------------------

void TestLoudness(void)
{
	int nch = 2;

	PRINTF("TestLoudness... \n");

	//user control
	RTX_Init(rtxCallback);
	KPD_Init(keyCallback);

	//audio init
	DSP_Flow();
	AUD_Routing(nch);

	//start dsp
	STA_Play();
	SLEEP(5);					//wait 5ms for DSP initialisation

	//post initialisation
	ResetGains(&g_gains);
	ResetLoudness(&g_loudn);
	g_selMod = g_gains.mod;

	DMABUS_Set(PLY_MSP2, g_song1.nch, DSP_SEL);

	//mini player
	PLY_Init();

	//play
	PLY_Set_pcm_source_MSP(MSP2, &g_song1, PLY_NOFLAG);
	PLY_SetState_MSP(MSP2, PLY_PLAY);

	//play (in loop)
	while (1)
	{
		/*if (g_key_pressed)
			process_key();

		if (g_rtx_steps)
			process_rtx();*/

		SLEEP(1000);	//avoid sticky loop
	}


	//stop and clean
	STA_Reset();
//	PRINT_OK_FAILED;
}

#endif //0
