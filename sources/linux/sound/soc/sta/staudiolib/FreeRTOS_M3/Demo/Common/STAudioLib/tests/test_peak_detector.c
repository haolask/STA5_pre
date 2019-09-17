/*
 * STAudioLib - test_peak_detector.c
 *
 * Created on 2016/07/04 by Olivier DOUVENOT
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	Testing gain (2ch by default)
	/!\ THIS TEST DOES NOT USE DSP @48Khz, instead triggers XIN/XOUT swaps manually

            PEAK DETECTOR
	        +-------+
	XIN[0]--| Peak0 |
	XIN[1]--| Peak1 |
	        +-------+

 */

#if 1
#include "staudio_test.h"


static STAModule g_peak_detector;


//---------------------------------------------------------
//Test changing gain
//
static void Test_Auto(void)
{
    // strong values
	const float din0[6]  = {1.0f, 0.8f, 0.2f, 0.0001f, -0.8f, -1.0f};
	const float dout0[6]  = {1.0f, 0.8f, 0.2f, 0.0001f, 0.8f, 1.0f};
	const s32 log0[6]  = {0, -19, -140, -800, -19, 0};

    // weaker values
	const float din1[6]  = {0.5f, 0.4f, 0.1f, 0.0f, -0.4f, -0.5f};
	const float dout1[6]  = {0.5f, 0.4f, 0.1f, 0.0f, 0.4f, 0.5f};
	const s32 log1[6]  = {-60, -79, -200, -1400, -79, -60};

	const int nch = 6;
	int i;
	s32 level;

	STA_SetMode(g_peak_detector, nch);

	//frame 0: //din0 -> AHB_XIN
	FORi(nch) XIN[i] = FRAC(din0[i]);
	SWAP_DP(1);

	//frame 1: din0: AHB_XIN -swap-> DSP_XIN -transfer-> GAIN_IN -filter-> GAIN_OUT
	FORi(nch) XIN[i] = FRAC(din0[i]);		//din1 -> AHB_XIN
	SWAP_DP(1);

	//frame 2: din0: GAIN_OUT -transter-> DSP_XOUTFLT(XOUT[4])
	SWAP_DP(1);

	//test strong values
	FORi(nch) CHECKF(STA_PeakDetectorGetLevel(g_peak_detector, i, STA_PEAK_DB), log0[i]);
	FORi(nch)
	{
		level = STA_PeakDetectorGetLevel(g_peak_detector, i, STA_PEAK_LINEAR|STA_PEAK_RESET);
		CHECKF(FLT(level), dout0[i]);
	}
	SWAP_DP(1);

	//frame 0: //din0 -> AHB_XIN
	FORi(nch) XIN[i] = FRAC(din1[i]);
	SWAP_DP(1);

	//frame 1: din0: AHB_XIN -swap-> DSP_XIN -transfer-> GAIN_IN -filter-> GAIN_OUT
	FORi(nch) XIN[i] = FRAC(din1[i]);		//din1 -> AHB_XIN
	SWAP_DP(1);

    //needs to reset all peaks before to test weaker values
	FORi(nch) STA_PeakDetectorResetLevel(g_peak_detector, i);

	//frame 2: din0: GAIN_OUT -transter-> DSP_XOUTFLT(XOUT[4])
	SWAP_DP(1);

	//test and reset weaker values
	FORi(nch)
	{
		level = STA_PeakDetectorGetLevel(g_peak_detector, i, STA_PEAK_LINEAR);
		CHECKF(FLT(level), dout1[i]);
	}
	FORi(nch) CHECKF(STA_PeakDetectorGetLevel(g_peak_detector, i, STA_PEAK_DB|STA_PEAK_RESET), log1[i]);
	SWAP_DP(1);
}
//---------------------------------------------------------
static void BuildFlow(void)
{
	int i;

	//add modules
	g_peak_detector = STA_AddModule(STA_DSP0, STA_PEAKDETECT_NCH); //2 gains by default
	STA_SetNumChannels(g_peak_detector, 6);  						//add more channels

	//connect
	FORi(6) {
		STA_Connect(STA_XIN0,  i,  g_peak_detector,   i);
	}

	//build
	STA_BuildFlow();
}
//---------------------------------------------------------
void TestPeakDetector(void)
{
	PRINTF("TestPeakDetector... ");

	STA_Reset();

	BuildFlow();

	//start
	//STA_StartDSP(STA_DSP0);	//call STA_Play() instead
	STA_Play();
	SLEEP(5);					//wait 5ms for DSP initialisation

	Test_Auto();

	//stop and clean
//	STA_DeleteModule(&g_peak_detector);	//optionnal, cleaning done in STA_Reset anyway
	STA_Reset();
	PRINT_OK_FAILED;
}


#endif //0
