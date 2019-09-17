/*
 * STAudioLib - test_gain.c
 *
 * Created on 2013/04/22 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	Testing gain (2ch by default)
	/!\ THIS TEST DOES NOT USE DSP @48Khz, instead triggers XIN/XOUT swaps manually

	          GAINS
	        +-------+
	XIN[0]--| Gain0 |--XOUT[0]
	XIN[1]--| Gain1 |--XOUT[1]
	        +-------+

 */

#if 1
#include "staudio_test.h"
#include "math.h"

#define DB2LIN(db)		pow(10, (double)db / 20)
#define LIN2DB(lin)		(20.0 * log10((double)lin))

static STAModule g_gains;

//---------------------------------------------------------
//Test DSP passthrough (XIN to XOUT) without DMA.
//Assuming a simple stereo passthrough via a 2ch gain,
//an input data in XIN takes 3 frames to arrive to XOUT
//
static void Test1_XINtoXOUT(void)
{
	//frame 0: din0 -> AHB_XIN
	XIN[0] = 0xAAAAAAAA;
	XIN[1] = 0xBBBBBBBB;
	SWAP_DP(1);

	//frame 1: din0: AHB_XIN -swap-> DSP_XIN -transfer-> GAIN_IN -filter-> GAIN_OUT
	//		   din1 -> AHB_XIN
	XIN[0] = 0xCCCCCCCC;
	XIN[1] = 0xDDDDDDDD;
	SWAP_DP(1);

	//frame 2: din0: GAIN_OUT -transter-> DSP_XOUT
	SWAP_DP(1);

	//frame 3: din0: DSP_XOUT -swap-> AHB_XOUT
	CHECK(XOUT[0] == 0x00AAAAAA);
	CHECK(XOUT[1] == 0x00BBBBBB);
	SWAP_DP(1);

	//frame 4: din1: DSP_XOUT -swap-> AHB_XOUT
	CHECK(XOUT[0] == 0x00CCCCCC);
	CHECK(XOUT[1] == 0x00DDDDDD);
	SWAP_DP(1);
}

//---------------------------------------------------------
//Test DMA transfer + DSP passthrough
//
static void Test2_XINtoXOUT_withDMA(void)
{
	//NOTE: To test the DMA transfer we use the LPF data registers because
	//		they are the only one to be both r/w

	//init the DMA transfers
	STA_DMAReset();
	STA_DMAAddTransfer(STA_DMA_LPF0_DI,  STA_DMA_DSP0_XIN, 2);  //LPF0_DI  to XIN0
//	STA_DMAAddTransfer(STA_DMA_DSP0_XOUT, STA_DMA_LPF1_DI, 2);  //XOUT0    to LPF1_DI
	STA_DMAStart();

	//frame 0
	AIF->LPF0_DI[0].REG = 0xAAAAAAAA;
	AIF->LPF0_DI[1].REG = 0xBBBBBBBB;
	SLEEP(1);								//wait for DMAB transfer
	SWAP_DP(1);

	//frame 1
	AIF->LPF0_DI[0].REG = 0xCCCCCCCC;
	AIF->LPF0_DI[1].REG = 0xDDDDDDDD;
	SLEEP(1);								//wait for DMAB transfer
	SWAP_DP(1);

	//frame 2
	SWAP_DP(1);

	//frame 3
	CHECK(XOUT[0] == 0x00AAAAA0);  //(note: LPF forces least signigicant quartet to 0)
	CHECK(XOUT[1] == 0x00BBBBB0);
	SWAP_DP(1);

	//frame 4
	CHECK(XOUT[0] == 0x00CCCCC0);  //(note: LPF forces least signigicant quartet to 0)
	CHECK(XOUT[1] == 0x00DDDDD0);
	SWAP_DP(1);

	STA_DMAReset();
}
//---------------------------------------------------------
//Test changing gain
//
//static void Test3_SetGain(void)
//{
//	const float din0[6]  = {1.0f, 0.8f, 0.2f, 0.0f, -0.8f, -1.0f};
////	const float dout1[6] = {1.0f, 1.0f, 0.4f, 0.0f, -1.0f, -1.0f};  // = din1 x 2 (+6dB)
//	const int nch = 6;
//	int i;
//
//	//frame 0: //din0 -> AHB_XIN
//	STA_GainSetGain(g_gains, 0, -60);		//-6dB (amplitude ~ x0.5) to channel 0
//	STA_GainSetGains(g_gains, 0xFE, -60);	//-6dB (amplitude ~ x0.5) to all other gains
//
//	FORi(nch) XIN[i] = FRAC(din0[i]);
//	SWAP_DP(1);
//
//	//frame 1: din0: AHB_XIN -swap-> DSP_XIN -transfer-> GAIN_IN -filter-> GAIN_OUT
//	STA_GainSetGains(g_gains, 0xFF, +60);	//+6dB (amplitude ~ x2) to all gains
//	FORi(nch) XIN[i] = FRAC(din0[i]);		//din1 -> AHB_XIN
//	SWAP_DP(1);
//
//	//frame 2: din0: GAIN_OUT -transter-> DSP_XOUTFLT(XOUT[4])
//	SWAP_DP(1);
//
//	//frame 3: din0: DSP_XOUT -swap-> AHB_XOUT
//	FORi(nch) CHECKF(FLT(XOUT[i]), din0[i] * 0.5f);
//	SWAP_DP(1);
//
//	//frame 4: din1: DSP_XOUT -swap-> AHB_XOUT
//	FORi(nch) CHECKF(FLT(XOUT[i]), CLAMPF(din0[i] * 2));
//	SWAP_DP(1);
//}

static void Test3_SetGain(void)
{
//	const double din0[6]  = {1.0, 0.8, 0.2, 0.0, -0.8, -1.0};	//note: din < 0.5 fails the test....
	const double din0[6]  = {1.0, 0.8, 0.5, 0.0, -0.8, -1.0};
	const int nch     = 6;
	const int dbStart = -120+6;				//starting from -120dB would fail because DSP snap -120dB to linGain = 0 (MUTE)
	const int dbEnd   = 24;
	const int dbStep  = 6;
	const int dbDelay = (3-1) * dbStep;		//3 samples delay from XIN + GAIN + XOUT

	s32 dbGain;
	double linGain;
	int i;

	for (dbGain = dbStart; dbGain <= dbEnd + dbDelay; dbGain += dbStep)
	{
		//set gains
		if (dbGain <= 24) {
			STA_GainSetGain(g_gains, 0, dbGain*10);		//testing one channel (channel 0)
			STA_GainSetGains(g_gains, 0xFE, dbGain*10);	//testing masked channel (all but channel 0)
		}

		//send input data
		FORi(nch) XIN[i] = FRAC(din0[i]);
		SWAP_DP(1);

		//check output (after 3 samples delay)
		if (dbGain - dbDelay >= dbStart)
		{
			//linGain 3 steps before because of the 3 samples delay
			linGain = DB2LIN((double)(dbGain - dbDelay));

			FORi(nch)
				CHECKDIFF(DBL(XOUT[i]), CLAMPD(linGain * din0[i]), 0.000014);
		}
	}
}

static void Test4_SetLinGain(void)
{
//	const double din0[6]  = {1.0, 0.8, 0.2, 0.0, -0.8, -1.0};	//note: din < 0.5 fails the test....
	const double din0[6]  = {1.0, 0.8, 0.5, 0.1, -0.8, -1.0};

	const q23 lingain1[6]  = {8388607, 5242880, 4194304, 5242880, -5242880, -8388608};
	const s32 shift1[6]  = {0, 1, 2, 4, 1, 0};

	const q23 lingain2[6]  = {5368709, 6710886, 5368709, 6710886, -6710886, -5368709};
	const s32 shift2[6]  = {-6, -6, -5, -3, -6, -6};

	const int nch     = 6;

	int i;


    //set gains
    FORi(nch)
        STA_GainSetLinearGainsAndShifts(g_gains, 1<<i, lingain1[i], shift1[i]);

	//send input data
	FORi(nch)
        XIN[i] = FRAC(din0[i]);

	SWAP_DP(3);

	FORi(nch)
        CHECKD(DBL(XOUT[i]), 1.0, 0.000001);

    //set gains
    FORi(nch)
        STA_GainSetLinearGainsAndShifts(g_gains, 1<<i, lingain2[i], shift2[i]);

	//send input data
	FORi(nch)
        XIN[i] = FRAC(din0[i]);

	SWAP_DP(3);

	FORi(nch)
        CHECKD(DBL(XOUT[i]), 0.01, 0.000001);
}
//---------------------------------------------------------
static void BuildFlow(void)
{
	int i;

	//add modules
	g_gains = STA_AddModule(STA_DSP0, STA_GAIN_STATIC_NCH); //2 gains by default
	STA_SetNumChannels(g_gains, 6);  						//add more channels

	//connect
	FORi(6) {
	STA_Connect(STA_XIN0,  i,  g_gains,   i);
	STA_Connect(g_gains,   i,  STA_XOUT0, i);
	}

	//build
	STA_BuildFlow();
}
//---------------------------------------------------------
void TestGain(void)
{
	PRINTF("TestGain... ");

	STA_Reset();

	BuildFlow();

	//start
	STA_StartDSP(STA_DSP0);
	//STA_Play();				//DSPStart + DMABStart
	SLEEP(1);					//wait 5ms for DSP initialisation

	Test1_XINtoXOUT();
	Test2_XINtoXOUT_withDMA();
	Test3_SetGain();
	Test4_SetLinGain();

	//stop and clean
//	STA_DeleteModule(&g_gains);	//optionnal, cleaning done in STA_Reset anyway
	STA_Reset();
	PRINT_OK_FAILED;
}

//Just for example
/*
					 DSP GAINS
		  DMA		 +-------+        DMA
	SRC0_L -> XIN[0]-| Gain0 |-XOUT[0] -> LPF0_L
	SRC0_R -> XIN[1]-| Gain1 |-XOUT[1] -> LPF0_R
	SRC1_L -> XIN[2]-| Gain2 |-XOUT[2] -> LPF1_L
					 +-------+
*/
/*
void InitPassthrough(void)
{
	int i;

	//init STAudio
	STA_Init(NULL);
	STA_LoadDSP(STA_DSP0);

	//add modules
	g_gains = STA_AddModule(STA_DSP0, STA_GAIN_STATIC_NCH); //2 gains by default
	STA_SetNumChannel(g_gains, 3);  						//add more channels

	//connect modules
	for(i = 0; i < 3; i++) {
	STA_Connect(STA_XIN0,  i,  g_gains,   i);	//inputs
	STA_Connect(g_gains,   i,  STA_XOUT0, i);	//ouputs
	}

	//build
	STA_BuildFlow();

	STA_GainSetGains(g_gains, 0xF, 0);	//0dB: passthrough gains (as by default)

	//program the DMA transfers
	STA_DMAReset();
	//inputs
	STA_DMAAddTransfer(STA_DMA_SRC0_DO,     STA_DMA_DSP0_XIN,   2);  //SRC0 to XIN0[0:1] (stereo)
	STA_DMAAddTransfer(STA_DMA_SRC1_DO,     STA_DMA_DSP0_XIN+2, 1);  //SRC1 to XIN0[2]   (mono)
	//outputs
	STA_DMAAddTransfer(STA_DMA_DSP0_XOUT,   STA_DMA_LPF0_DI,    2);  //XOUT[0:1] to LPF0 (stereo)
	STA_DMAAddTransfer(STA_DMA_DSP0_XOUT+2, STA_DMA_LPF1_DI,    1);  //XOUT[2]   to LPF1 (mono)

	//Play
	STA_Play();			//start DSP and DMABus
	vTaskDelay(5); 		//wait 5ms for DSP initialisation

	//playing....

	STA_Stop();			//stop DSP and DMABus(optional)
	STA_Reset();		//stop and clean all (optional)
	STA_Exit();			//stop, clean all and exit
}
*/

#endif //0
