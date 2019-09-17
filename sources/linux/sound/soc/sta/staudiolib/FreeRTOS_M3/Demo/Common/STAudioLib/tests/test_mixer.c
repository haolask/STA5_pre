/*
 * STAudioLib - test_mixer.c
 *
 * Created on 2013/04/22 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	/!\ THIS TEST DOES NOT USE DSP @48Khz, instead triggers XIN/XOUT swaps manually

	Testing mixer (2ch by default)
	All combinations of num channels and num ins per channel are possible,
	but the number of insets is always 2:

	E.g. Mixer 2ins x 2ch
		        +--------+
		XIN[0]--| Mixer0 |--XOUT[0]  (L)
		XIN[1]--|        |
	            +--------+
		XIN[2]--| Mixer1 |--XOUT[1]  (R)
		XIN[3]--|        |
	            +--------+

	E.g. Mixer 3ins x 3ch
		                   +--------+
    	XIN[0]--\        #=| Mixer0 |--XOUT[0]  (L)
	    XIN[1]--| insetA # +--------+
	    XIN[2]--/        #=| Mixer1 |--XOUT[1]  (R)
		XIN[3]--\          +--------+
	    XIN[4]--| insetB #=| Mixer2 |--XOUT[2]  (center)
	    XIN[5]--/          +--------+

	E.g. Mixer 3ins x 4ch
		                   +--------+
    	XIN[0]--\        #=| Mixer0 |--XOUT[0]  (LF)
	    XIN[1]--| insetA # +--------+
	    XIN[2]--/        #=| Mixer1 |--XOUT[1]  (LR)
		                   +--------+
	    XIN[3]--\        #=| Mixer2 |--XOUT[2]  (RF)
	    XIN[4]--| insetB # +--------+
	    XIN[5]--/        #=| Mixer3 |--XOUT[3]  (RR)
	                       +--------+

	etc...

 */

//TODO: Test saturation cases...


#if 1
#include "staudio_test.h"

static STAModule g_mixer;

//the maximum num of inputs are STA_MAX_MIXER_INS * 2 insets = 16
static const float g_in0[18] = {
0.01f, 0.01f,
0.01f, 0.01f,
0.01f, 0.01f,
0.01f, 0.01f,
0.01f, 0.01f,
0.01f, 0.01f,
0.01f, 0.01f,
0.01f, 0.01f,
0.01f, 0.01f};


//---------------------------------------------------------
//Test Mixer passthrough (XIN to XOUT) without DMA.
//
static void Test1_XINtoXOUT(int ninpc, int nout)
{
	const int nin = ninpc * 2/*insets*/;
	int i;

	//Since all linear gains are 0 by default, we need to set some gains.
	//Each mixer applies x InGains x Softmute x volume
#ifndef MIXER_VER_1
	STA_MixerSetOutRamps(g_mixer, 0xFFFF, 0/*ms*/);			//set all out_t = 0 ms
	STA_MixerSetOutGains(g_mixer, 0xFFFF, 0/*db*/);				//set all out_g = 1
	STA_MixerSetInRamps( g_mixer, 0xFFFF, 0xFFFF, 0/*ms*/);	//set all in_t = 0 ms
#endif
	STA_MixerSetInGains(g_mixer, 0xFFFF, 0xFFFF, 0/*db*/);		//set all in_g = 1
	STA_MixerSetVolume( g_mixer, 0xFFFF, 0/*db*/);				//set all out volume_g = 1
#ifdef MIXER_VER_1
	STA_MixerSetMute(g_mixer, 0xFFFF, FALSE, FALSE, 1/*don't update depth*/);
#endif

	//frame 0: in0 -> AHB_XIN
	FORi(nin) XIN[i] = FRAC(g_in0[i]);
	SWAP_DP(3);

	//frame 3: in0: DSP_XOUT -swap-> AHB_XOUT
	FORi(nout) CHECKF(FLT(XOUT[i]), 0.01f * ninpc);
}

//---------------------------------------------------------
//Test STA_MixerSetInGains()
//
static void Test2_SetInGains(int ninpc, int nout)
{
	const int nin = ninpc * 2 /*insets*/;
	int i,j;

	//Since all linear gains are 0 by default, we need to set some gains.
	//Each mixer applies x InGains x Softmute x volume
#ifndef MIXER_VER_1
	STA_MixerSetOutRamps(g_mixer, 0xFFFF, 0/*ms*/);	//set all out_t = 0 ms
	STA_MixerSetOutGains(g_mixer, 0xFFFF, 0/*db*/);	//set all out_g = 1
	STA_MixerSetInRamps(g_mixer, 0xFFFF, 0xFFFF, 0/*ms*/);	//set all in_t = 0 ms
#endif
	STA_MixerSetInGains(g_mixer, 0xFFFF, 0xFFFF, -1200);	//OFF all inputs (in_g = 0)
	STA_MixerSetInGain(g_mixer, 0, -1200);	//OFF ch 0 input (in_g = 0)
	STA_MixerSetVolume(g_mixer, 0xFFFF, 0);				//set all out volume_g = 1
#ifdef MIXER_VER_1
	STA_MixerSetMute(g_mixer, 0xFFFF, FALSE, FALSE, 1);	//
#endif

	//frame 0: in0 -> AHB_XIN
	FORi(nin) XIN[i] = FRAC(g_in0[i]);
	SWAP_DP(1);
	FORi(nin) XIN[i] = FRAC(g_in0[i]);

	//we loop through in channel j (for each out channel)
	FORj(ninpc)
	{
	STA_MixerSetInGain(g_mixer, j, -1200);		//ONLY input j (set in_g[j] = 1) (for each channel)
	STA_MixerSetInGains(g_mixer, 0xFFFF, 1<<j, 0);		//ONLY input j (set in_g[j] = 1) (for each channel)

	SWAP_DP(5);

	//frame 3:
	FORi(nout) CHECKF(FLT(XOUT[i]), 0.01f);

	STA_MixerSetInGains(g_mixer, 0xFFFF, 1<<j, -1200);	//OFF input j (in_g[j] = 0)
	STA_MixerSetInGain(g_mixer, j, -1200);	//OFF input j (in_g[j] = 0)
#ifndef MIXER_VER_1
    // RAMP DOWN
	STA_MixerSetInRamps(g_mixer, 0xFFFF, 1<<j, 1/*ms*/);	//set all in_t = 5 ms
	STA_MixerSetInGains(g_mixer, 0xFFFF, 1<<j, -60);		//ONLY input j (set in_g[j] = 0.5) (for each channel)

	SWAP_DP(3);
	SWAP_DP(48);

	//frame 3:
	FORi(nout) CHECKF(FLT(XOUT[i]), 0.01f*0.5);

    // RAMP UP
	STA_MixerSetInRamps(g_mixer, 0xFFFF, 1<<j, 1/*ms*/);	//set all in_t = 1 ms
	STA_MixerSetInGains(g_mixer, 0xFFFF, 1<<j, 0);		//ONLY input j (set in_g[j] = 1.0) (for each channel)

	SWAP_DP(3);
	SWAP_DP(48);

	//frame 3:
	FORi(nout) CHECKF(FLT(XOUT[i]), 0.01f);

	STA_MixerSetInRamps(g_mixer, 0xFFFF, 1<<j, 0/*ms*/);	//set all in_t = 0 ms
#endif

	STA_MixerSetInGains(g_mixer, 0xFFFF, 1<<j, -1200);	//OFF input j (in_g[j] = 0)
	}
}
//---------------------------------------------------------
//Test STA_MixerSetOutGains()
//
static void Test3_SetOutGains(int ninpc, int nout)
{
	const int nin = ninpc * 2 /*insets*/;
	int i;

	//Since all linear gains are 0 by default, we need to set some gains.
	//Each mixer applies x InGains x Softmute x volume
#ifndef MIXER_VER_1
	STA_MixerSetOutRamps(g_mixer, 0xFFFF, 0/*ms*/);	//set all out_t = 0 ms
	STA_MixerSetOutGains(g_mixer, 0xFFFF, 0/*db*/);	//set all out_g = 1
	STA_MixerSetOutGains(g_mixer, 0x2, -1200/*db*/);	//mute second channel
	STA_MixerSetInRamps(g_mixer, 0xFFFF, 0xFFFF, 0/*ms*/);	//set all in_t = 0 ms
#endif
	STA_MixerSetInGains(g_mixer, 0xFFFF, 0xFFFF, 0);		//ON all inputs (in_g = 0)
	STA_MixerSetInGain(g_mixer, 0, 0);		//ON ch 0 inputs (in_g = 0)
	/* TODO STA_MixerSetVolume(g_mixer, 0x1, -60);				//set volume0 = x0.5
	if (nout > 1)
	STA_MixerSetVolume(g_mixer, 0x2, -1200);			//set volume1 = x0 */
#ifdef MIXER_VER_1
	STA_MixerSetMute(g_mixer, 0xFFFF, FALSE, FALSE, 1);	//
#endif

    //frame 0:
    FORi(nin) XIN[i] = FRAC(g_in0[i]);
    SWAP_DP(1);
    FORi(nin) XIN[i] = FRAC(g_in0[i]);
    SWAP_DP(4);

    //frame 3: in0: DSP_XOUT -swap-> AHB_XOUT
	CHECKF(FLT(XOUT[0]), 0.01f * ninpc);
	if (nout > 1)
	CHECKF(FLT(XOUT[1]), 0.01f * ninpc * 0.0f);

#ifndef MIXER_VER_1
    // RAMP DOWN
	STA_MixerSetOutRamps(g_mixer, 0x1, 1/*ms*/);	//set all out_t = 1.0 ms
	STA_MixerSetOutGains(g_mixer, 0x1, -60/*db*/);	//set all out_g = 0.5

    SWAP_DP(3);
    SWAP_DP(3*48);

	CHECKF(FLT(XOUT[0]), 0.01f * ninpc * 0.5f);
	if (nout > 1)
	CHECKF(FLT(XOUT[1]), 0.01f * ninpc * 0.0f);

	// RAMP UP
	STA_MixerSetOutRamps(g_mixer, 0x1, 1/*ms*/);	//set all out_t = 5 ms
	// TODO STA_MixerSetVolume(g_mixer, 0x1, 0/*db*/);	//set all out_g = 1
	STA_MixerSetOutGains(g_mixer, 0x1, 200/*db*/);	//set all out_g = 16.0

    SWAP_DP(3);
    SWAP_DP(3*48);

	CHECKF(FLT(XOUT[0]), 0.01f * ninpc * 10.0f);
	if (nout > 1)
	CHECKF(FLT(XOUT[1]), 0.01f * ninpc * 0.0f);
#endif
}
//---------------------------------------------------------

static void BuildFlow(int ninpc, int nout)
{
	const int nin = ninpc * 2 /*insets*/;
	int i;

	//add modules
	g_mixer = STA_AddModule(STA_DSP0, STA_MIXE_2INS_NCH + ninpc -2 );//2 channels by default. All linear gains = 0 by default
	STA_SetNumChannels(g_mixer, nout);  					//add more output channels (sub-mixers)

	//connect
	FORi(nin)  STA_Connect(STA_XIN0,  i,  g_mixer,   i);	//ins
	FORi(nout) STA_Connect(g_mixer,   i,  STA_XOUT0, i);	//out

	//build
	STA_BuildFlow();
}
//---------------------------------------------------------

void TestMixer(void)
{
	int nch, ninpc; //num channels, num inputs per channels

	PRINTF("TestMixer...    ");


	for (nch = 2; nch <= 6; nch++)
	{
		for (ninpc = 2; ninpc <= 9; ninpc++)
		{
			PRINTF("\b\b\b%2d%%", 100*(8*(nch-2)+ninpc-1)/(5*8));

			STA_Reset();
			BuildFlow(ninpc, nch);

			//start
			STA_Play();
	//		SLEEP(1); 			//wait 5ms for DSP initialisation

			Test1_XINtoXOUT(ninpc, nch);
			Test2_SetInGains(ninpc, nch);
			Test3_SetOutGains(ninpc, nch);

		}
	}

	PRINTF(" ");

	//stop and clean
	STA_Reset();
	PRINT_OK_FAILED;
}


#endif //0
