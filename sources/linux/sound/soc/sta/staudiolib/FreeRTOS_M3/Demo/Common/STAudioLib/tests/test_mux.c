/*
 * STAudioLib - test_mux.c
 *
 * Created on 2013/04/22 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	Testing MUX module

	          MUX_2OUT
	         +----+
	(def) 0--|0   |
	         |    |
	|SINE0|--|1--0|-- XOUT[0] L
	|SINE1|--|2\  |
	|SINE2|--|3 \1|-- XOUT[1] R
	|SINE3|--|4   |
			 |    |
	          ...
	       --|9   |
			 +----+
 */

#if 1
#include "staudio_test.h"

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
	.freq		= 44000,	//"440 Hz (La A4)"
	.msec		= 1000,
	.param		= SINE_FREQ,
};

//---------
// MUX
//---------
/*
typedef struct {
	STAModule	mod;
	u8			sels[STA_MAX_MUX_OUT];
}
*/

//---------
// MODULES
//---------
//for user interaction
#define NUM_SINE	4
static SINEWAVEGEN 	g_sine[NUM_SINE];  //  C4     D4     E4     F4
static u32 			g_sineFreqs[4]   = {26162, 29366, 32963, 34923};

static STAModule	g_mux;
static u8 			g_muxSels[2] = {1, 1};

//static STAModule	g_selected;	//user selected module


//---------------------------------------------------------
// BUILD FLOW
//---------------------------------------------------------
/*
static void ResetSinewaveGen(SINEWAVEGEN* sine)
{
	STAModule mod = sine->mod; 				//bak handle
	*sine = g_sineInit; sine->mod = mod;	//reset

	STA_SinewaveGenSet(mod, sine->freq, sine->msec, sine->gains[0], sine->gains[1]);
//	STA_SetMode(mod, sine->onoff);
}
*/

//---------------------------------------------------------

static void DSP_Flow(void)
{
	int i;

	STA_Reset();

	//MUX
	g_mux = STA_AddModule(STA_DSP0, STA_MUX_2OUT);
	STA_MuxSet(g_mux, g_muxSels);

	PRINTF("SELECTOR: toneL = %d, toneR = %d\n", g_muxSels[0], g_muxSels[1]);

	//SINEWAVE x4
	FORi(NUM_SINE) {
		g_sine[i]      = g_sineInit;
		g_sine[i].freq = g_sineFreqs[i];
		g_sine[i].mod  = STA_AddModule(STA_DSP0, STA_SINE_2CH);

//		STA_SinewaveGenSet(g_sine[i].mod, g_sine[i].freq, g_sine[i].msec, g_sine[i].gains[0], g_sine[i].gains[1]);
		STA_SetMode(g_sine[i].mod, STA_SINE_ON);
	}

	//CONNECTIONS
	FORi(NUM_SINE)
		STA_Connect(g_sine[i].mod, 0,  g_mux, i+1);	//note: keep mux_in0 open
	STA_Connect(g_mux, 0,  STA_XOUT0, 0);
	STA_Connect(g_mux, 1,  STA_XOUT0, 1);

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
}

static void keyCallback(int key, bool down)
{
	static int L = 0, R = 0;

	switch(key) {
	//change L and R
	case 0x21:  L = R = 0; break;
	case 0x22:  L = R = 1; break;
	case 0x23:  L = R = 2; break;
	case 0x24:  L = R = 3; break;
	//change L only
	case 0x31:  L = 0; break;
	case 0x32:  L = 1; break;
	case 0x33:  L = 2; break;
	case 0x34:  L = 3; break;
	//change R only
	case 0x41:  R = 0; break;
	case 0x42:  R = 1; break;
	case 0x43:  R = 2; break;
	case 0x44:  R = 3; break;

	default: return;
	}

	//select tones
	g_muxSels[0] = L+1;
	g_muxSels[1] = R+1;
	STA_MuxSet(g_mux, g_muxSels);

	//play tones
	STA_SinewaveGenSet(g_sine[L].mod, g_sine[L].freq, g_sine[L].msec, g_sine[L].gains[0], g_sine[L].gains[1]);
	STA_SinewaveGenSet(g_sine[R].mod, g_sine[R].freq, g_sine[R].msec, g_sine[R].gains[0], g_sine[R].gains[1]);

	PRINTF("SELECTOR: toneL = %d, toneR = %d\n", g_muxSels[0], g_muxSels[1]);
	return;
}

//---------------------------------------------------------
// Main
//---------------------------------------------------------

void TestMux(void)
{
	int nch = 2;

	PRINTF("TestMux... \n");

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
//	g_selected = g_sine.mod;

	//play (in loop)

	//SAI3 (master, enable)
	AIF->SAI3_CR.REG = SAI_I2S | SAI_MST | SAI_32_16VAL | SAI_RL | SAI_EN;
	STA_DMAStart();				//start DMABUS

	while (1)
		//Test_Play_PCM_48Khz_16bit(g_songAddr, g_songSize, nch);
		SLEEP(1000);


	//stop and clean
	STA_Reset();
//	PRINT_OK_FAILED;
}

#endif //0
