/*
 * STAudioLib - test_bitshift.c
 *
 * Created on 2016/06/07 by Olivier Douvenot
 * Copyright: STMicroelectronics
 *
 * ACCORDO2/ACCORDO2 STAudio test application
 */

/*
	Testing bitshifter (2ch by default)
	/!\ THIS TEST DOES NOT USE DSP @48Khz, instead triggers XIN/XOUT swaps manually

            BITSHIFT
	        +-------+
	XIN[0]--| Shft0 |--XOUT[0]
	XIN[1]--| Shft1 |--XOUT[1]
	        +-------+

 */

#if 1
#include "staudio_test.h"

#undef XIN
#define XIN P4_XIN2
#undef XOUT
#define XOUT P4_XOUT2
#define STA_DSPN STA_DSP2
#define STA_XOUTN STA_XOUT2
#define STA_XINN STA_XIN2

static STAModule g_bitshift;

//---------------------------------------------------------
//Test DSP passthrough (XIN to XOUT) without DMA.
//Assuming a simple stereo passthrough via a 2ch gain,
//an input data in XIN takes 3 frames to arrive to XOUT
//
static void Test1_XINtoXOUT(int nch)
{
	//frame 0: din0 -> AHB_XIN
	XIN[0] = 0xAAAAAAAA;
	XIN[1] = 0x77777777;
	if (nch > 2)
	XIN[2] = 0xBBBBBBBB;
	SWAP_DP(1);

	//frame 1: din0: AHB_XIN -swap-> DSP_XIN -transfer-> GAIN_IN -filter-> GAIN_OUT
	//		   din1 -> AHB_XIN
	XIN[0] = 0xCCCCCCCC;
	XIN[1] = 0x33333333;
	if (nch > 2)
	XIN[2] = 0xDDDDDDDD;
	SWAP_DP(1);

	//frame 2: din0: GAIN_OUT -transter-> DSP_XOUT
	SWAP_DP(1);

	//frame 3: din0: DSP_XOUT -swap-> AHB_XOUT
	CHECK(XOUT[0] == 0x00AAAAAA);
	CHECK(XOUT[1] == 0x00777777);
	if (nch > 2)
	CHECK(XOUT[2] == 0x00BBBBBB);
	SWAP_DP(1);

	//frame 4: din1: DSP_XOUT -swap-> AHB_XOUT
	CHECK(XOUT[0] == 0x00CCCCCC);
	CHECK(XOUT[1] == 0x00333333);
	if (nch > 2)
	CHECK(XOUT[2] == 0x00DDDDDD);
	SWAP_DP(1);
}


//---------------------------------------------------------
//Test changing gain
//
static void Test3_SetBitShift(int nch)
{
	const s32 din0[6] = { 0x123456, 0x123456, 0x123456, 0x123456, 0x123456, 0x123456};
	const s32 din1[6] = { 0xABCDEF, 0xABCDEF, 0xABCDEF, 0xABCDEF, 0xABCDEF, 0xABCDEF};
	const s32 leftshift[6] = {    8,       -8,       12,      -12,       16,     -16};
	const s32 dout0[6] = {0x345600,   0x1234, 0x456000,    0x123, 0x560000,     0x12};
	const s32 dout1[6] = {0xCDEF00, 0xFFABCD, 0xDEF000, 0xFFFABC, 0xEF0000, 0xFFFFAB};
	int i;

	// limit to max 6ch
	if (nch > 6) nch = 6;

	FORi(nch)
		STA_BitShifterSetLeftShifts(g_bitshift,	1<<i, leftshift[i]);

	//frame 0: //din0 -> AHB_XIN
	FORi(nch) XIN[i] = din0[i];
	SWAP_DP(1);

	//frame 1: din0: AHB_XIN -swap-> DSP_XIN -transfer-> GAIN_IN -filter-> GAIN_OUT
	//STA_GainSetGains(g_gains, 0xFF, +60);	//+6dB (amplitude ~ x2) to all gains
	FORi(nch) XIN[i] = din1[i];
	SWAP_DP(1);

	//frame 2: din0: GAIN_OUT -transter-> DSP_XOUTFLT(XOUT[4])
	SWAP_DP(1);

	//frame 3: din0: DSP_XOUT -swap-> AHB_XOUT
	FORi(nch) CHECK(XOUT[i]== dout0[i]);
	SWAP_DP(1);

	//frame 4: din1: DSP_XOUT -swap-> AHB_XOUT
	FORi(nch) CHECK(XOUT[i]== dout1[i]);
	SWAP_DP(1);
}
//---------------------------------------------------------
static void BuildFlow(int nch)
{
	int i;

	//add modules
	g_bitshift = STA_AddModule(STA_DSPN, STA_BITSHIFTER_NCH); //2 gains by default
	STA_SetNumChannels(g_bitshift, nch);  						//add more channels

	//connect
	FORi(nch) {
	STA_Connect(STA_XINN, i, g_bitshift, i);
	STA_Connect(g_bitshift, i, STA_XOUTN, i);
	}

	//build
	STA_BuildFlow();
}
//---------------------------------------------------------
void TestBitShift(void)
{
	PRINTF("TestBitShift (DSP%d)... ", STA_DSPN);
	int nch = 8;

	STA_Reset();

	BuildFlow(nch);

	//start
	STA_StartDSP(STA_DSPN);	//call STA_Play() instead
	//STA_Play();
	SLEEP(5);					//wait 5ms for DSP initialisation

	Test1_XINtoXOUT(nch);
	Test3_SetBitShift(nch);

	//stop and clean
	STA_Reset();
	PRINT_OK_FAILED;
}


#endif //0
