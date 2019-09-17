/*
 * STAudioLib - test_userModule.c
 *
 * Created on 2013/04/22 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	Testing USER module. For that, we reuse the ST SINEWAVE GEN module as a USER module,
	that we will call USERSWG module in this test.

	DSP Firmware
	------------
	Hence, we assume that the dsp firmware has been compiled with the following
	g_user_func_table in dsp_user_def.c:

	T_FilterFuncs _YMEM g_user_func_table[] = {
		//AudioFuncs                    InitFuncs             UpdateFuncs
		{ audio_sinewave,               NULL,                 NULL },      //USER MODULE 0
		{ , , },
		{ , , },
		{ , , },
		...
	};

	STAudioLib API extension
	------------------------
	The USERSWG module API is implemented in this test.

	Application / test
	------------------
	Finally we build the audio effect flow to test the USERSWG module

		USERSWG Module
		+-----+
		|SINE |--XOUT[0]
		|GEN  |--XOUT[1]
		+-----+

 */

#if 1

#include "staudio.h"
#include "audio.h"			//DSP audio types (for T_SinewaveData and T_SinewaveCoeffs)
#include "staudio_test.h"	//assert...


//sizeof in Words
#define WSIZEOF(type)	(sizeof(type)>>2)


//=========================================================
// USERSWG MODULE (Sinewave Generator) API
//=========================================================

#define USERSWG_OFF		0
#define USERSWG_ON		1

//USERSWG HOST params (internal)
typedef struct USERSinewaveGen_s {
	u32				m_mode;			//USERSWG_ON, USERSWG_OFF
	u32				m_freq;
	u32				m_msec;
	s32				m_gainDb[2];	//user input gain (dB * 10)
	s32				m_gainLn[2];	//linear gain (in Fraction 1.23) (ONE = 0x7FFFFF)
} USERSinewaveGen_t;

//InitModule() callback
void USERSWG_InitModule(STA_UserModuleInfo* info)
{
	USERSinewaveGen_t* const sine = info->m_hostParams;

	//init HOST params
	sine->m_mode 	  = USERSWG_ON;
	sine->m_freq 	  = 44000;							//init with A4 440Hz
	sine->m_msec 	  = 0;
	sine->m_gainDb[0] = sine->m_gainDb[1] = 0; 		    //0 dB;
	sine->m_gainLn[0] = sine->m_gainLn[1] = 0x7FFFFF;	//linear gains = 1
}

//AdjustDspMemSize() callback (NOT USED)
void USERSWG_AdjustDspMemSize(STA_UserModuleInfo* info)
{
}

//UpdateDspCoefs() callback
void USERSWG_UpdateDspCoefs(STA_UserModuleInfo* info)
{
	USERSinewaveGen_t* 			const sine = info->m_hostParams;	//arm
	volatile T_SinewaveCoeffs* 	const coef = info->m_dspCoefs;		//dsp

	u32 Fs = STA_GetInfo(STA_INFO_SAMPLING_FREQ);
	assert(info->m_dspCoefs);

	//update DSP params
	coef->on_off 	  = (sine->m_mode == USERSWG_OFF) ? 0 : (Fs / 1000 * sine->m_msec); //duration (in num of samples)
	coef->out_gain[0] = sine->m_gainLn[0];
	coef->out_gain[1] = sine->m_gainLn[1];
	coef->phase_inc   = FRAC((float) sine->m_freq / (Fs * 50));
}

//InitDspData() callback
void USERSWG_InitDspData(STA_UserModuleInfo* info)
{
	volatile T_SinewaveData* 	const data = info->m_dspData;	//ARM addr
	volatile T_SinewaveCoeffs* 	const coef = info->m_dspCoefs;	//ARM addr
	u32 xdata = info->m_xdata;							//DSP xmem offset
	u32 ycoef = info->m_ycoef;							//DSP ymem offset

	assert(data && coef);

	//init DSP data (XMEM) and coefs (YMEM)

	//XMEM layout: T_SinewaveData | dout[2]
	xdata += WSIZEOF(T_SinewaveData);
	data->dout[0] 	= (fraction*) xdata;
	data->dout[1] 	= (fraction*)(xdata + 1);
	data->phase   	= 0;

	//YMEM layout: T_SinewaveCoeffs
	USERSWG_UpdateDspCoefs(info);
}

//GetDspInAddr() callback (NOT USED)
void* USERSWG_GetDspInAddr(const STA_UserModuleInfo* info, u32 in)
{
	return NULL;
}

//GetDspInAddr() callback
void* USERSWG_GetDspOutAddr(const STA_UserModuleInfo* info, u32 out)
{
	assert(out < info->m_nout);

	return (void*) ((T_SinewaveData*) info->m_dspData)->dout[out];
}

//SetMode() callback
void USERSWG_SetMode(STA_UserModuleInfo* info, u32 mode)
{
}


//USERSW API
void USERSWG_PlayTone(STAModule module, u32 freq, u32 msec, s32 gainL, s32 gainR)
{
	STA_UserModuleInfo* info = (STA_UserModuleInfo*)module;	//allowed casting!!
	USERSinewaveGen_t*  sine = info->m_hostParams;

	/*
	CHECK_VALUE( -1200 <= gainL && gainL <= 0
			 &&  -1200 <= gainR && gainR <= 0
			 &&   2000 <= freq  && freq  <= 2000000); // x0.01 Hz
	*/

	sine->m_freq = freq;
	sine->m_msec = msec;

	//convert from dB to Linear in fraction format
	if (sine->m_gainDb[0] != gainL) {
		sine->m_gainDb[0] = gainL;
		sine->m_gainLn[0] = STA_DBtoLinear(gainL, 10, NULL, TRUE);
	}

	if (sine->m_gainDb[1] != gainR) {
		sine->m_gainDb[1] = gainR;
		sine->m_gainLn[1] = (gainR == gainL) ? sine->m_gainLn[0] : STA_DBtoLinear(gainR, 10, NULL, TRUE);
	}

	//update dsp
	if (info->m_dspCoefs)
		USERSWG_UpdateDspCoefs(info);
}

//USERSWG module info
STA_UserModuleInfo g_USERSinewaveGenInfo = {
	.m_dspType 			= 0, //DSP USER MODULE 0
	.m_nin  			= 0,
	.m_nout 			= 2,
	.m_sizeofParams 	= sizeof(USERSinewaveGen_t),
	.m_wsizeofData  	= WSIZEOF(T_SinewaveData) + 2,
	.m_wsizeofCoefs 	= WSIZEOF(T_SinewaveCoeffs),
	.InitModule 		= USERSWG_InitModule,
	.InitDspData 		= USERSWG_InitDspData,
	.UpdateDspCoefs 	= USERSWG_UpdateDspCoefs,
	.GetDspInAddr 		= NULL,	//USERSWG_GetDspInAddr,
	.GetDspOutAddr 		= USERSWG_GetDspOutAddr,
	.SetMode 			= NULL, //USERSWG_SetMode,
	.AdjustDspMemSize 	= NULL, //USERSWG_AdjustDspMemSize
};



//=========================================================
// Testing USERSWG MODULE (Sinewave Generator) API
//=========================================================

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
	s16			gains[2];		//user param
	u32			freq;			//user param
	u32			msec;			//user param
	enum SINE_PARAM param;		//selected param to update
} SINEWAVEGEN;

static const SINEWAVEGEN g_sineInit = {
	.mod 		= 0,
	.gains 		= {0,0},
	.freq		= 44000, //440 Hz
	.msec		= 10000, //10 sec
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

//	STA_SetMode(mod, USERSWG_ON);
	USERSWG_PlayTone(mod, sine->freq, sine->msec, sine->gains[0], sine->gains[1]);
}


//---------------------------------------------------------

static void DSP_Flow(void)
{
	int i;
	STA_Reset();

	//SINEWAVE
	g_sine.mod = STA_AddUserModule(DSP_SEL, STA_USER_0, &g_USERSinewaveGenInfo);
	ResetSinewaveGen(&g_sine);

	//GAINS
	g_gains = STA_AddModule(DSP_SEL, STA_GAIN_STATIC_NCH); //2 gains by default
//	g_gains = STA_AddModule(DSP_SEL, STA_GAIN_SMOOTH_NCH); //2 gains by default

	//CONNECTIONS
	FORi(2) {
        STA_Connect(g_sine.mod, i,  g_gains, i);
        STA_Connect(g_gains, i,  	MOD_XOUT, i);
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
		USERSWG_PlayTone(g_sine.mod, g_sine.freq, g_sine.msec, g_sine.gains[0], g_sine.gains[1]);
		return;
	}
}

static void keyCallback(int key, bool down)
{
	//module selection
	//...

	//SINEWAVE GEN
	if (g_selected == g_sine.mod) {
		switch(key) {
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

void TestUserModule(void)
{
	int nch = 2;

	PRINTF("TestUserModule... \n");

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
//	ResetSinewaveGen(&g_sine);
	g_selected = g_sine.mod;

	//play (in loop)

	//SAI3 (master, enable)
	AIF->SAI3_CR.REG = SAI_I2S | SAI_MST | SAI_32_24VAL | SAI_RL | SAI_EN;
	STA_DMAStart();				//start DMABUS

	while (1) {
		//Test_Play_PCM_48Khz_16bit(g_songAddr, g_songSize, nch);
		USERSWG_PlayTone(g_sine.mod, g_sine.freq, g_sine.msec, g_sine.gains[0], g_sine.gains[1]);
		SLEEP(g_sine.msec - 10);
	}

	//stop and clean
	STA_Reset();
//	PRINT_OK_FAILED;
}

#endif //0
