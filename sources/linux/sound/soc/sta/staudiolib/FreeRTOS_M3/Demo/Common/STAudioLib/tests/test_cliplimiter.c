/*
 * STAudioLib - test_clilimiter.c
 *
 * Created on 2013/04/22 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	Testing LIMITER 2CH module

Playing internal sinewave

			  S.GAIN
	+-----+  +------+  +-----------+
	|SINE |--|gain0 |--|    Clip   |--XOUT[0] L
	|GEN  |--|gain1 |--|  Limiter  |--XOUT[1] R
	+-----+  +------+  |    2 CH   |
CLIP->DCO_DAT->XIN[2]--|           |
                       +-----------+

or PLAY SONG

			  S.GAIN
	         +------+  +-----------+
	XIN[0] --|gain0 |--|    Clip   |--XOUT[0] L
	XIN[1] --|gain1 |--|  Limiter  |--XOUT[1] R
	         +------+  |    2 CH   |
CLIP->DCO_DAT->XIN[2]--|           |
                       +-----------+

 */

#if 1

#include "staudio_test.h"

// 0 for sine generator, 1 to play a sound
#define PLAY_SONG 0


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

#if  (PLAY_SONG==0)

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
	.gains 		= {-0,-0},		//-0 dB, 1 ~ 0.1 dB
	.freq		=  44000, 		//440 Hz, 1 ~ 0.01 Hz
	.msec		=  100, 		// 20 sec, 1 ~ 1 ms
	.param		= SINE_FREQ,
};

#endif

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
	.dbStep		= 1,  //+- 0.1dB
	.tup		= 100, //4000, //4 sec
	.tdown		= 100, //1000, //1 sec
};

static void set_cliplimiter_params(STAModule cliplimiter)
{
	u32 flags =
	  STA_CLIPLMTR_CLIP_BITMASK
	| STA_CLIPLMTR_CLIP_POLARITY
	| STA_CLIPLMTR_MAX_ATTENUATION
	| STA_CLIPLMTR_MIN_ATTENUATION
	| STA_CLIPLMTR_ATTACK_TIME
	| STA_CLIPLMTR_RELEASE_TIME
	| STA_CLIPLMTR_ADJ_ATTENUATION;

	s32 values[STA_CLIPLMTR_NUMBER_OF_IDX]	 = {0};

	values[STA_CLIPLMTR_IDX_CLIP_BITMASK]    = 1 << 18; //NEED TO MATCH the DCO_CR setting
	values[STA_CLIPLMTR_IDX_CLIP_POLARITY]   = 0;		//NEED TO MATCH the POWER-AMP
	values[STA_CLIPLMTR_IDX_MAX_ATTENUATION] = 1200;	// = 120.0 dB
	values[STA_CLIPLMTR_IDX_MIN_ATTENUATION] = 0;		// = 0.0 dB
	values[STA_CLIPLMTR_IDX_ATTACK_TIME] 	 = 5*10;	// = 5 ms
	values[STA_CLIPLMTR_IDX_RELEASE_TIME]	 = 500*10;	// = 500 ms
	values[STA_CLIPLMTR_IDX_ADJ_ATTENUATION] = 18;		// = 1.8 dB

	STA_ClipLimiterSetParams(cliplimiter, flags, values);
}

static void set_cliplimiter(STAModule cliplimiter, u32 mode)
{
    set_cliplimiter_params(cliplimiter);
	STA_SetMode(cliplimiter, mode);
}

static void reset_cliplimiter(STAModule cliplimiter)
{
	STA_SetMode(cliplimiter, STA_CLIPLMTR_OFF);
}

//---------
// MODULES
//---------
//for user interaction
#if (PLAY_SONG==0)
static SINEWAVEGEN 	g_sine;
#endif
static GAIN         g_gains;
static STAModule	g_selected;	//user selected module

static STAModule    g_cliplimiter;
static STA_ClipLimiterMode g_clipmode = STA_CLIPLMTR_OFF;


//---------------------------------------------------------
// DSP FLOW
//---------------------------------------------------------
#if (PLAY_SONG==0)
static void ResetSinewaveGen(SINEWAVEGEN* sine)
{
	STAModule mod = sine->mod; 				//bak handle
	*sine = g_sineInit; sine->mod = mod;	//reset

	STA_SetMode(mod, STA_SINE_ON);
	STA_SinewaveGenSet(mod, sine->freq, sine->msec, sine->gains[0], sine->gains[1]);
}
#endif

static void ResetGains(GAIN* gains)
{
	STAModule mod = gains->mod; 			//bak handle
	*gains = g_gainsInit; gains->mod = mod;	//reset

	STA_GainSetGains(gains->mod, GAIN_L, gains->L);
	STA_GainSetGains(gains->mod, GAIN_R, gains->R);

	if (gains->type == STA_GAIN_SMOOTH_NCH)
		STA_SmoothGainSetRamp(gains->mod, 0xF, gains->tup, gains->tdown);

}

static void DSP_init(int nch)
{
	int i;

	STA_Reset();

	//CLIPLIMITER
	g_cliplimiter = STA_AddModule(DSP_SEL, STA_CLIPLMTR_NCH); // 2ch by default

	g_cliplimiter = STA_SetNumChannels(g_cliplimiter, nch); // update to nch

    #if (PLAY_SONG==0)
    //SINEWAVE
	g_sine.mod = STA_AddModule(DSP_SEL, STA_SINE_2CH);
	ResetSinewaveGen(&g_sine);
    #endif

	//GAINS
	g_gains = g_gainsInit;
	g_gains.mod = STA_AddModule(DSP_SEL, g_gains.type); //2 gains by default

	g_gains.mod = STA_SetNumChannels(g_gains.mod, nch);	//just to try, even if only using 2ch

	if (g_gains.type == STA_GAIN_SMOOTH_NCH)
		ResetGains(&g_gains); //smooth gain can be set before BuildFlow!


	//CONNECTIONS
	FORi(nch) {
        #if (PLAY_SONG==0)
        if (i < 2)
            STA_Connect(g_sine.mod, i, g_gains.mod, i);
        #else
        STA_Connect(MOD_XIN, i, g_gains.mod, i);
        #endif
		STA_Connect(g_gains.mod, i, g_cliplimiter, i);
        STA_Connect(g_cliplimiter, i, MOD_XOUT, i);
    }
    STA_Connect(MOD_XIN, nch, g_cliplimiter, nch);


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

	STA_DMAAddTransfer(STA_DMA_DCO_DATA, 	DMA_XIN+nch, 1);	//DCO_DATA to XIN

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

		#if (PLAY_SONG==0)
        //replay sine
		STA_SinewaveGenSet(g_sine.mod, g_sine.freq, g_sine.msec, g_sine.gains[0], g_sine.gains[1]);
		#endif
	}
}
int g_key = 0;
bool g_down = 0;
int g_key_cb = 0;

static void keyCallback(int key, bool down)
{
    if (g_key_cb == 0)
    {
        g_key = key;
        g_down = down;
        g_key_cb = 1;
    }
}

static void keyCallbackProcess(int key, bool down)
{
	/*
	key mapping (GAIN)
	[MOD1]  [MOD2]	    [   ]  [   ]  [     ]  [     ]  <- CLIP LIMITER
                        [VOL]  [RST]  [LMASK]  [RMASK]  <- GAIN
	[MUTE]  [REC ]		[FRQ]  [RST]  [ VOL ]  [ BAL ]  <- SINEWAVEGEN
	*/

    if (down != TRUE) return;

	switch(key) {

	//MUTE
	case 0x13: 	toggleMute(); return;

	//REC
	case 0x14:	toggleRecord(); return;

	//CLIPLIMITER
	case 0x11:
        if (g_clipmode == STA_CLIPLMTR_ATT_REL_MODE)
        {
            g_clipmode = STA_CLIPLMTR_OFF;
            reset_cliplimiter(g_cliplimiter); PRINTF("ClipLimiter OFF\n");
        }
        else
        {
            reset_cliplimiter(g_cliplimiter);
            g_clipmode = STA_CLIPLMTR_ATT_REL_MODE;
            set_cliplimiter(g_cliplimiter, g_clipmode); PRINTF("ClipLimiter ON mode 1\n");
        }
        return;
	case 0x12:
        if (g_clipmode == STA_CLIPLMTR_FULL_ATT_MODE)
        {
            g_clipmode = STA_CLIPLMTR_OFF;
            reset_cliplimiter(g_cliplimiter); PRINTF("ClipLimiter OFF\n");
        }
        else
        {
            reset_cliplimiter(g_cliplimiter);
            g_clipmode = STA_CLIPLMTR_FULL_ATT_MODE;
            set_cliplimiter(g_cliplimiter, g_clipmode); PRINTF("ClipLimiter ON mode 2\n");
        }
        return;

#if (PLAY_SONG==0)
	//SINEWAVE GEN
	case 0x41:  g_selected = g_sine.mod; g_sine.param = SINE_FREQ;  PRINTF("SINE: Frq = %d Hz\n", g_sine.freq/100); return;
	case 0x42:  g_selected = g_sine.mod; ResetSinewaveGen(&g_sine); PRINTF("SINE: Reset\n"); return;
	case 0x43:  g_selected = g_sine.mod; g_sine.param = SINE_VOL;   PRINTF("SINE: Vol: L = %d dB, R = %d dB\n", g_sine.gains[0]/10, g_sine.gains[1]/10); return;
	case 0x44:  g_selected = g_sine.mod; g_sine.param = SINE_BAL;   PRINTF("SINE: Bal: L = %d dB, R = %d dB\n", g_sine.gains[0]/10, g_sine.gains[1]/10); return;
#endif
	//GAIN
	case 0x31:  g_selected = g_gains.mod; PRINTF("GAIN: L,R = %d, %d dB\n", g_gains.L/10, g_gains.R/10); return;
	case 0x32:  g_selected = g_gains.mod; ResetGains(&g_gains); PRINTF("GAIN: L,R = %d, %d dB\n", g_gains.L/10, g_gains.R/10); return;
	case 0x33:  g_selected = g_gains.mod; g_gains.LRmask ^= GAIN_L; PRINTF("Change %s%s\n", (g_gains.LRmask&GAIN_L)?"L":"", (g_gains.LRmask&GAIN_R)?"R":""); return;
	case 0x34:  g_selected = g_gains.mod; g_gains.LRmask ^= GAIN_R; PRINTF("Change %s%s\n", (g_gains.LRmask&GAIN_L)?"L":"", (g_gains.LRmask&GAIN_R)?"R":""); return;

	//default: return;
	}
}

int g_steps = 0;
int g_rtx_cb = 0;

static void rtxCallback(int steps)
{
    if (g_rtx_cb == 0)
    {
        g_steps = steps;
        g_rtx_cb = 1;
    }
}
static void rtxCallbackProcess(int steps)
{

#if (PLAY_SONG==0)
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
#endif
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
		PRINTF("GAIN: L,R = %d, %d dB/10\n", g_gains.L, g_gains.R);
		return;
	}
}

//---------------------------------------------------------
// Main
//---------------------------------------------------------

void TestClipLimiter(void)
{
	int nch = 2;

    #if (PLAY_SONG==1)
	PRINTF("TestClipLimiter: Play song %d channels... \n", nch);
	#else
	PRINTF("TestClipLimiter: Sine wave %d channels... \n", nch);
	#endif

    // GPIO10 enabled
	GPIO_Set((GPIO_Typedef*)R4_GPIO0_BASE, 1<<10, GPIO_NOALT | GPIO_IN | GPIO_NOTRIG);

    // Connect DCO bit X to GPIO10
	AUDCR->DCO_CR0.REG = 0;			   //(reset all mux)
	AUDCR->DCO_CR1.REG = 0; 		   //(reset all mux)
	AUDCR->DCO_CR0.BIT.MUX0SEL = 10;   //gpio10 to DCO bit 18 => CLIP CH0
	//AUDCR->DCO_CR0.BIT.MUX1SEL = 10; //gpio10 to DCO bit 19 => CLIP CH1
	//AUDCR->DCO_CR0.BIT.MUX2SEL = 10; //gpio10 to DCO bit 20 => CLIP CH2
	//AUDCR->DCO_CR1.BIT.MUX3SEL = 10; //gpio10 to DCO bit 21 => CLIP CH3
	//AUDCR->DCO_CR1.BIT.MUX4SEL = 10; //gpio10 to DCO bit 22 => CLIP CH4
	//AUDCR->DCO_CR1.BIT.MUX5SEL = 10; //gpio10 to DCO bit 23 => CLIP CH5



	//user control
	RTX_Init(rtxCallback);
	KPD_Init(keyCallback);

    #if (PLAY_SONG==1)
	//mini player
	DMABUS_Set(PLY_MSP2, g_song1.nch, DSP_SEL);

	//mini player
	PLY_Init();

	//play
	PLY_Set_pcm_source_MSP(MSP2, &g_song1, PLY_NOFLAG);
	PLY_SetState_MSP(MSP2, PLY_PLAY);
    #endif

	//DSP init
	DSP_init(nch);

	//DMABUS init
	DMABUS_init(nch);

	//start dsp and dmabus
	STA_Play();
	SLEEP(100);					//wait 100ms for DSP initialisation

	//post initialisation
	if (g_gains.type == STA_GAIN_STATIC_NCH)
		ResetGains(&g_gains);

	g_selected = g_gains.mod;


	//SAI3 (master, enable)
	AIF->SAI3_CR.REG = SAI_I2S | SAI_MST | SAI_32_24VAL | SAI_EN;
	STA_DMAStart();				//start DMABUS

	while (1)
	{
        #if (PLAY_SONG==0)
		STA_SinewaveGenSet(g_sine.mod, g_sine.freq, g_sine.msec, g_sine.gains[0], g_sine.gains[1]);
		SLEEP(g_sine.msec - 10);
		#else
		SLEEP(1000);
		#endif
		if (g_key_cb)
		{
            keyCallbackProcess(g_key, g_down);
            g_key_cb = 0;
		}
		if (g_rtx_cb)
		{
            rtxCallbackProcess(g_steps);
            g_rtx_cb = 0;
		}
	}

	//stop and clean
	STA_Reset();
//	PRINT_OK_FAILED;
}

#endif //0
