/*
 * STAudioLib - test_limiter.c
 *
 * Created on 2013/04/22 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	Testing LIMITER (2ch) module

	+-----+  +-------+
	|SINE |--|LIMITER|--XOUT[0] L
	|GEN  |--|       |--XOUT[1] R
	+-----+  +-------+

*/

#if 0
#include "staudio_test.h"


//----------
// MODULES
//----------

static STAModule 		g_gains;
static STAModule		g_limiter;

//---------------------------------------------------------
// BUILD FLOW
//---------------------------------------------------------

static void set_limiter(STAModule limiter)
{
	u32 flags = STA_LMTR_THRES;
	s16 values[7];

	values[2] = -60; //STA_LMTR_THRES = -6 dB

	STA_LimiterSetParams(limiter, flags, values);
	STA_SetMode(limiter, STA_LMTR_ON);
}

static void DSP_Flow(void)
{
	int i;

	STA_Reset();

	//GAINS
//	g_gains = STA_AddModule(STA_DSP0, STA_GAIN_STATIC_NCH);  //(2 gains by default)
//	g_gains = STA_SetNumChannels(g_gains, 6);
//	STA_GainSetGain(g_gains, 0, 50); //L = R = 0 dB	<-- init


	//LIMITER
	g_limiter = STA_AddModule(STA_DSP0, STA_LMTR_2CH);  //(2 gains by default)
	STA_SetMode(g_limiter, STA_LMTR_ON);

	//CONNECTIONS
	FORi(2) {
		STA_Connect(STA_XIN0,   i,  g_limiter,   i);
		STA_Connect(g_limiter,  i,  STA_XOUT0,   i);

//		STA_Connect(STA_XIN0,   i,  STA_XOUT0,   i);  //TMP for dbg
	}

	//BUILD
	STA_BuildFlow();
	ASSERT(STA_GetError() == STA_NO_ERROR);

	//POST-BUILD INITS

	//START DSP
	STA_Play();
	SLEEP(100);					//wait 100ms for DSP initialisation

	//POST-DSP-INIT INITS
//	STA_GainSetGains(g_gains, 0xFF, 60); //L = R = 0 dB

}


static void play_and_dump_DSP(void)
{
	u32 dst = 0x61E00000; //in SDRAM
	u32 numSamples = 20000*2;
	u32 numLoaded;
	u32 numDstBytes;
	int i;

	//DMABUS: SAI2 -> DSP0 -> FIFO1
	DMABUS_Reset();
	DMABUS_AddTransfers(DMABUS_SAI2_RX1,  DMABUS_DSP0_XIN,   2);	//SAI2 to DSP0 (2ch)
	DMABUS_AddTransfers(DMABUS_DSP0_XOUT, DMABUS_FIFO_IN,    2);	//DSP0 to FIFO1 (2ch)
	DMABUS_AddTransfers(DMABUS_SAI2_RX1,  DMABUS_DSP0_XIN+127, 1);	//swap trigger

	//PLAY
	numLoaded = Test_Dump_PCM_48Khz_16_to_24of32bit(g_songAddr+11, (u32*)dst, numSamples, 2);

	Convert_32to16((void*)dst, numLoaded*4, (void*)dst, &numDstBytes);

	//then retrieve th dump with J-Link Commander:
	//J-Link> savebin <filename>,  <addr>     <numBytes>
	//J-Link> savebin savebin.raw, 0x61E00010, 0x5000
}


/*
static void test_STA_DB2Frac(void)
{
	int i;
	s32 db[7] = { -1920, -100, -10, 0, 10, 100, 1920};
	s32 frfl[7] = {0}, frfx[7] = {0};

	FORi(7){
		frfx[i] = db[i] * 0x1102;			//using fixed
		frfl[i] = _STA_DB2Frac(db[i], 10, 0);	//using float
	}
}
*/
//---------------------------------------------------------
// Main
//---------------------------------------------------------

void TestLimiter(void)
{
	const char* songFilename = "0:/29_400Hz_48k_16bit.raw";


	PRINTF("TestLimiter... \n");

	if (!ReadFile(songFilename, &g_songSize, g_songAddr)) {
		PRINTF("ERR: cannot copy file %s\n", songFilename);	return;	}

	DSP_Flow();

 	play_and_dump_DSP();

	//stop and clean
	STA_Reset();
//	PRINT_OK_FAILED;
}


#endif //0
