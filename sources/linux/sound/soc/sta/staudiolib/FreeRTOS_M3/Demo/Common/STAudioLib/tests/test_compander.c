/*
 * STAudioLib - test_compander.c
 *
 * Created on 2016/02/05 by Olivier DOUVENOT
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	Testing COMPANDER 2CH module

			  S.GAIN
	+-----+  +------+  +---------+
	|SINE |--|gain0 |--|COMPANDER|--XOUT[0] L
	|GEN  |--|gain1 |--|   2CH   |--XOUT[1] R
	+-----+  +------+  +---------+

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
//	.gains 		= {-200,-200},	//-20 dB so to let space for volume ctr > 0 dB
	.gains 		= {-0,-0},	//-0 dB
//	.freq		=   3000, 		// 30 Hz
	.freq		=  44000, 		//440 Hz
//	.freq		= 700000, 		//  7 kHz
	.msec		=  20000, 		// 20 sec
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
	.mod 		= 0,
	.LRmask		= GAIN_L|GAIN_R,
	.L 			= 0,   //  0 dB
	.R			= 0,   //  0 dB
	.dbStep		= 30,  //+- 3dB
	.tup		= 100, //4000, //4 sec
	.tdown		= 100, //1000, //1 sec
};

//----------
// COMPANDER
//----------
static void set_compander(STAModule compander)
{
	u32 flags = STA_COMP_MEAN_MAX_MODE
	| STA_COMP_AVG_FACTOR
	| STA_COMP_ATTENUATION
	| STA_COMP_CPR_THRES
	| STA_COMP_CPR_SLOPE
	| STA_COMP_CPR_HYSTERESIS
	| STA_COMP_CPR_ATK_TIME
	| STA_COMP_CPR_REL_TIME
	| STA_COMP_EXP_THRES
	| STA_COMP_EXP_SLOPE
	| STA_COMP_EXP_HYSTERESIS
	| STA_COMP_EXP_ATK_TIME
	| STA_COMP_EXP_REL_TIME
;

	s16 values[STA_COMP_NUMBER_OF_IDX];

	values[STA_COMP_IDX_MEAN_MAX_MODE] = 0;          //STA_COMP_MEAN_MAX_MODE = mean method
	values[STA_COMP_IDX_AVG_FACTOR] = 120;        //STA_COMP_AVG_FACTOR = 12 ms
	values[STA_COMP_IDX_ATTENUATION] = 120;        //STA_COMP_ATTENUATION = 12 dB

	values[STA_COMP_IDX_CPR_THRES] = -420;       //STA_COMP_CPR_THRES = -42 dB
	values[STA_COMP_IDX_CPR_SLOPE] = -16384;     //STA_COMP_CPR_SLOPE = -0.5
	values[STA_COMP_IDX_CPR_HYSTERESIS] = 50;          //STA_COMP_CPR_HYSTERESIS = 0.50 dB
	values[STA_COMP_IDX_CPR_ATK_TIME] = 5;          //STA_COMP_CPR_ATK_TIME = 0.5 ms
	values[STA_COMP_IDX_CPR_REL_TIME] = 479;        //STA_COMP_CPR_REL_TIME = 47.9 ms

	values[STA_COMP_IDX_EXP_THRES] = -720;       //STA_COMP_EXP_THRES = -72 dB
	values[STA_COMP_IDX_EXP_SLOPE] = -14746;     //STA_COMP_EXP_SLOPE = -0.45
	values[STA_COMP_IDX_EXP_HYSTERESIS] = 20;         //STA_COMP_EXP_HYSTERESIS = 0.20 dB
	values[STA_COMP_IDX_EXP_ATK_TIME] = 3;         //STA_COMP_EXP_ATK_TIME = 0.3 ms
	values[STA_COMP_IDX_EXP_REL_TIME] = 9;         //STA_COMP_EXP_REL_TIME = 0.9 ms

	STA_CompanderSetParams(compander, flags, values);
	STA_SetMode(compander, STA_COMP_ON);
}

static void set_compander_6dB(STAModule compander)
{
	u32 flags =	STA_COMP_ATTENUATION;

	s16 values[STA_COMP_NUMBER_OF_IDX];

	values[STA_COMP_IDX_ATTENUATION] = 60;        //STA_COMP_ATTENUATION = 6 dB

	STA_CompanderSetParams(compander, flags, values);
	STA_SetMode(compander, STA_COMP_ON);
}

static void set_compander_24dB(STAModule compander)
{
	u32 flags =	STA_COMP_ATTENUATION;

	s16 values[STA_COMP_NUMBER_OF_IDX];

	values[STA_COMP_IDX_ATTENUATION] = 240;        //STA_COMP_ATTENUATION = 24 dB

	STA_CompanderSetParams(compander, flags, values);
	STA_SetMode(compander, STA_COMP_ON);
}

static void reset_compander(STAModule compander)
{
	STA_SetMode(compander, STA_COMP_OFF);
}




//---------
// MODULES
//---------
//for user interaction
static SINEWAVEGEN 	g_sine;
static GAIN         g_gains;
static STAModule    g_compander;
static int               g_Comp_option = 0;
static STA_CompanderMode g_Comp_mode = STA_COMP_OFF;
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

	if (gains->type != STA_GAIN_STATIC_NCH)
		STA_SmoothGainSetRamp(gains->mod, 0xF, gains->tup, gains->tdown);

}

static void DSP_init(void)
{
	int i;

	STA_Reset();

	//COMPANDER
	g_compander = STA_AddModule(DSP_SEL, STA_COMP_2CH);

	//SINEWAVE
	g_sine.mod = STA_AddModule(DSP_SEL, STA_SINE_2CH);
	ResetSinewaveGen(&g_sine);

	//GAINS
	g_gains = g_gainsInit;
	g_gains.mod = STA_AddModule(DSP_SEL, g_gains.type); //2 gains by default

//	g_gains.mod = STA_SetNumChannels(g_gains.mod, 6);	//just to try, even if only using 2ch

	if (g_gains.type != STA_GAIN_STATIC_NCH)
		ResetGains(&g_gains); //smooth gain can be set before BuildFlow!

	//CONNECTIONS
	FORi(2) {
        STA_Connect(g_sine.mod,  i,  g_gains.mod, i);
        STA_Connect(g_gains.mod, i,  g_compander,   i);
        STA_Connect(g_compander,   i,  MOD_XOUT,    i);
    }

#if 0
	//TMP for testing
	STA_Connect(g_sine.mod,  0,  g_gains.mod, 2);
	STA_Connect(g_sine.mod,  1,  g_gains.mod, 3);
    STA_Connect(g_gains.mod, 2,  MOD_XOUT,    0);
    STA_Connect(g_gains.mod, 3,  MOD_XOUT,    1);
#endif

	//BUILD
	STA_BuildFlow();

	ASSERT(STA_GetError() == STA_NO_ERROR);
}

//---------------------------------------------------------
// DMABUS
//---------------------------------------------------------

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

//---------------------------------------------------------
// Recording
//---------------------------------------------------------
static int	g_record = 0;
static u32*	g_rec_dst = (u32*)0x61E00000; //in SDRAM
static u32	g_rec_size;

static void toggleRecord(void)
{
	u32* dst 		= g_rec_dst;
//	u32 numSamples	= 20000*2;
	u32 numSamples	= -1; //max

	g_record ^= 1;

	if (g_record) {
		PRINTF("Recording...\n");
		PLY_start_dump_FIFO1(dst, numSamples, 2); //max numsamples
	}
	else
	{
		PRINTF("Stop recording\n");
		PLY_stop_dump_FIFO1();

		PRINTF("converting to 16 bits...");
		Convert_32to16((void*)dst, g_player.fifo1.m_numBytes, (void*)dst, &g_rec_size);
		PRINTF(" done\n");
	}

	//then retrieve th dump with J-Link Commander:
	//J-Link> savebin <filename>,  <addr>     <numBytes>
	//J-Link> savebin savebin.raw, 0x61E00010, 0x5000
}

//---------------------------------------------------------
// KEY / ROTARY
//---------------------------------------------------------
static void toggleMute(void)
{
	g_gains.mute ^= 1;

	PRINTF(g_gains.mute ? "GAIN: mute\n" : "GAIN: unmute\n");

	if (g_gains.mute)
		STA_GainSetGains(g_gains.mod, 0x3, -1200);
	else {
		STA_GainSetGains(g_gains.mod, GAIN_L, g_gains.L);
		STA_GainSetGains(g_gains.mod, GAIN_R, g_gains.R);

		//replay sine
		STA_SinewaveGenSet(g_sine.mod, g_sine.freq, g_sine.msec, g_sine.gains[0], g_sine.gains[1]);
	}
}

static void keyCallback(int key, bool down)
{
	/*
	key mapping (GAIN)
	[    ]  [xxxx]	    [ON ]  [OFF]  [     ]  [     ]  <- COMPANDER
                        [VOL]  [RST]  [LMASK]  [RMASK]  <- GAIN
	[MUTE]  [REC ]		[FRQ]  [RST]  [ VOL ]  [ BAL ]  <- SINEWAVEGEN
	*/

	switch(key) {

	//LIMITER
	case 0x11:
	if (g_Comp_mode == STA_COMP_OFF)
	{
        g_Comp_mode = STA_COMP_ON;
        set_compander(g_compander);
        PRINTF("Compander ON (-12 dB)\n");
    }
    else
    {
        g_Comp_mode = STA_COMP_OFF;
        reset_compander(g_compander);
        PRINTF("Compander OFF\n");
    }
    return;

	case 0x12:
	if (g_Comp_mode == STA_COMP_ON)
	{
        if (g_Comp_option == 0)
        {
            g_Comp_option = 1;
            set_compander_6dB(g_compander);
            PRINTF("Compander -6 dB\n");
        }
        else if (g_Comp_option == 1)
        {
            g_Comp_option = 2;
            set_compander_24dB(g_compander);
            PRINTF("Compander -24 dB\n");
        }
        else
        {
            g_Comp_option = 0;
            set_compander(g_compander);
            PRINTF("Compander -12 dB\n");
        }
    }
    else
    {
        g_Comp_option = 0;
        g_Comp_mode = STA_COMP_ON;
        set_compander(g_compander);
        PRINTF("Compander ON (-12 dB)\n");
    }
    return;

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

static void rtxCallback(int steps)
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
			g_gains.L += steps * g_gains.dbStep; //gain step
			STA_GainSetGains(g_gains.mod, GAIN_L, g_gains.L);
		}
		if (g_gains.LRmask & GAIN_R) {
			g_gains.R += steps * g_gains.dbStep; //gain step
			STA_GainSetGains(g_gains.mod, GAIN_R, g_gains.R);
		}
		PRINTF("GAIN: L,R = %d, %d dB\n", g_gains.L/10, g_gains.R/10);
		return;
	}
}

//---------------------------------------------------------
// Main
//---------------------------------------------------------

void TestCompander(void)
{
	int nch = 2;

	PRINTF("TestCompander... \n");

	//user control
	RTX_Init(rtxCallback);
	KPD_Init(keyCallback);

	//mini player
	PLY_Init();
//	PLY_Set_PCM_MSP2(g_songAddr, g_songSize, 2, 16, 16000);

	//DSP init
	DSP_init();

	//DMABUS init
	DMABUS_init(nch);

	//start dsp and dmabus
	STA_Play();
	SLEEP(100);					//wait 100ms for DSP initialisation

	//post initialisation
	if (g_gains.type == STA_GAIN_STATIC_NCH)
		ResetGains(&g_gains);

	g_selected = g_gains.mod;

	//play (in loop)
//	PLY_SetState(PLY_PLAY);		//starts MSP2/SAI2, SAI3,

	//SAI3 (master, enable)
	AIF->SAI3_CR.REG = SAI_I2S | SAI_MST | SAI_32_24VAL | SAI_EN;
	STA_DMAStart();				//start DMABUS

	while (1)
	{
		//Test_Play_PCM_48Khz_16bit(g_songAddr, g_songSize, nch);
		STA_SinewaveGenSet(g_sine.mod, g_sine.freq, g_sine.msec, g_sine.gains[0], g_sine.gains[1]);
		SLEEP(g_sine.msec - 10);
	}

	//stop and clean
	STA_Reset();
//	PRINT_OK_FAILED;
}

#endif //0
