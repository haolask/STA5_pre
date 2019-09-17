/*
 * STAudioLib - test_dump.c
 *
 * Created on 2013/04/22 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	Testing EQUALIZER module (static stereo)

	          GAINS    7-band EQ
	        +-------+  +-------+
	XIN[0]--| Gain0 |--|  EQ0  |--XOUT[0]
	XIN[1]--| Gain1 |--|  EQ1  |--XOUT[1]
	        +-------+  +-------+


 7-Band EQUALIZER Presets:
 =========================
 +/-10 unit   scale -> 0,  1,  2,  3, 4 5 6 7 8 9 10
 +/-24 dB(x4) scale -> 0, 10, 19, 29, ....

		FLAT			CLASSIC			POP				ROCK			JAZZ
64    ---o----------  ---o--|-------  -|-o----------  ---o----|-----  ---o|---------
150   ---o----------  ---o-|--------  ---|----------  ---o-|--------  ---|----------
400   ---o----------  ---o|---------  ---o--|-------  ---|----------  ---|----------
1000  ---o----------  ---|----------  ---o----|-----  ---o--|-------  ---o-|--------
2500  ---o----------  --|o----------  ---o-|--------  ---|----------  ---o---|------
6300  ---o----------  ---o-|--------  ---|----------  ---o-|--------  ---o----|-----
16000 ---o----------  ---o----|-----  --|o----------  ---o---|------  ---o-|--------

*/

#if 0
#include "staudio_test.h"

//----------
// EQ
//----------

#define EQ_NUM_BANDS  7

//enum EQ_MODE {EQ_FLAT, EQ_CLASSIC, EQ_POP, EQ_ROCK, EQ_JAZZ};

s16 g_bandGQF[EQ_NUM_BANDS][3] = {{0, 30, 63},{0, 30, 150},{0, 30, 400},{0, 30, 1000},{0, 30, 2500},{0, 30, 6300},{0, 30, 16000}}; //7-bands FLAT

/*
static const s16 g_eq_preset_G[][7] = {
	//63 150 400  1K 2.5 6.3  16K
	{  0,  0,  0,  0,  0,  0,  0}, //FLAT
	{ 30, 20, 10,  0,-10, 20, 50}, //CLASSIC
	{-20,  0, 30, 50, 20,  0,-10}, //POP
	{ 50, 20,  0, 30,  0, 20, 40}, //ROCK
	{ 10,  0,  0, 20, 40, 50, 20}};//JAZZ
*/

//----------
// MODULES
//----------

static STAModule 		g_gains;
static STAModule		g_EQ;

//---------------------------------------------------------
// BUILD FLOW
//---------------------------------------------------------

static void DSP_nogain(void)
{
	int i;

	STA_Reset();

	//CONNECTIONS
	FORi(2) {
		STA_Connect(STA_XIN0, i,  STA_XOUT0, i);
	}

	//BUILD
	STA_BuildFlow();
	ASSERT(STA_GetError() == STA_NO_ERROR);

	//START DSP
	STA_Play();
	SLEEP(5);					//wait 5ms for DSP initialisation
}

static void DSP_gain(void)
{
	int i;

	STA_Reset();

	//GAINS
	g_gains = STA_AddModule(STA_DSP0, STA_GAIN_STATIC_NCH);  //(2 gains by default)
//	g_gains = STA_SetNumChannels(g_gains, 6);
	STA_GainSetGain(g_gains, 0, 50); //L = R = 0 dB

	//CONNECTIONS
	FORi(2) {
		STA_Connect(STA_XIN0, i,  g_gains,   i);
		STA_Connect(g_gains,  i,  STA_XOUT0, i);
	}

	//BUILD
	STA_BuildFlow();
	ASSERT(STA_GetError() == STA_NO_ERROR);

	//START DSP
	STA_Play();
	SLEEP(100);					//wait 5ms for DSP initialisation

//	STA_GainSetGains(g_gains, 0xFF, 60); //L = R = 0 dB
}

static void DSP_gain_EQ(void)
{
	int i;

	STA_Reset();

	//GAINS
	g_gains = STA_AddModule(STA_DSP0, STA_GAIN_STATIC_NCH);  //(2 gains by default)
	STA_GainSetGain(g_gains, 0, 0); //L = R = 0 dB

	//EQ
	g_EQ = STA_AddModule(STA_DSP0, STA_EQUA_STATIC_7BANDS_DP);  //(2 ch by default)

	//CONNECTIONS
	FORi(2) {
		STA_Connect(STA_XIN0, i,  g_gains,   i);
		STA_Connect(g_gains,  i,  g_EQ,      i);
		STA_Connect(g_EQ,     i,  STA_XOUT0, i);
	}

	//BUILD
	STA_BuildFlow();
	ASSERT(STA_GetError() == STA_NO_ERROR);

	//START DSP
	STA_Play();
	SLEEP(5);					//wait 5ms for DSP initialisation

	//POST PLAY INIT
	STA_SetMode(g_EQ, STA_EQ_ON);

	FORi(EQ_NUM_BANDS) {
//		STA_SetFilterType(g_EQ, i, STA_BIQUAD_BAND_BOOST_2);
		STA_SetFilterv(g_EQ, i, g_bandGQF[i]); //FLAT mode
	}
}

static void test_dump_noDSP(void)
{
	u32 dst = 0x61E00000; //in SDRAM
	u32 numSamples = 20000*2;
	u32 numLoaded;
	u32 numDstBytes;

	//DMABUS: SAI2 -> FIFO1
	DMABUS_Reset();
	DMABUS_AddTransfers(DMABUS_SAI2_RX1, DMABUS_FIFO_IN, 2);	//SAI2 to FIFO1 (ch)

	numLoaded = Test_Dump_PCM_48Khz_16_to_24of32bit(g_songAddr+11, (u32*)dst, numSamples, 2);

	Convert_32to16((void*)dst, numLoaded*4, (void*)dst, &numDstBytes);
}


static void test_dump_DSP(void)
{
	u32 dst = 0x61E00000; //in SDRAM
	u32 numSamples = 20000*2;
	u32 numLoaded;
	u32 numDstBytes;
	int i;

//	DSP_nogain();
	DSP_gain();
//	DSP_gain_EQ();

	//DMABUS: SAI2 -> DSP0 -> FIFO1
	DMABUS_Reset();
#if 1
	DMABUS_AddTransfers(DMABUS_SAI2_RX1,  DMABUS_DSP0_XIN,   2);	//SAI2 to DSP0 (2ch)
	DMABUS_AddTransfers(DMABUS_DSP0_XOUT, DMABUS_FIFO_IN,    2);	//DSP0 to FIFO1 (2ch)
#else
	//for debug
	DMABUS_AddTransfers(DMABUS_SAI2_RX1,  DMABUS_DSP0_XIN,   2);	//SAI2 to DSP0 (2ch) --> READ
	DMABUS_AddTransfers(DMABUS_SAI2_RX1,  DMABUS_DSP0_XIN+2, 2);	//SAI2 to DSP0 (2ch)
	DMABUS_AddTransfers(DMABUS_SAI2_RX1,  DMABUS_DSP0_XIN+4, 2);	//SAI2 to DSP0 (2ch)
	DMABUS_AddTransfers(DMABUS_DSP0_XOUT, DMABUS_SAI3_TX1,   2);	//DSP0 to SAI3 (2ch)
	DMABUS_AddTransfers(DMABUS_DSP0_XOUT, DMABUS_SAI3_TX2,   2);	//DSP0 to SAI3 (2ch)
	DMABUS_AddTransfers(DMABUS_DSP0_XOUT, DMABUS_SAI3_TX3,   2);	//DSP0 to SAI3 (2ch)
	DMABUS_AddTransfers(DMABUS_DSP0_XOUT, DMABUS_FIFO_IN,    2);	//DSP0 to FIFO1 (2ch) --> DUMP
#endif
	DMABUS_AddTransfers(DMABUS_SAI2_RX1,  DMABUS_DSP0_XIN+127, 1);	//swap trigger

	//PLAY
	numLoaded = Test_Dump_PCM_48Khz_16_to_24of32bit(g_songAddr+11, (u32*)dst, numSamples, 2);

	Convert_32to16((void*)dst, numLoaded*4, (void*)dst, &numDstBytes);

	//then retrieve th dump with J-Link Commander:
	//J-Link> savebin <filename>,  <addr>     <numBytes>
	//J-Link> savebin savebin.raw, 0x61E00010, 0x5000
}




//---------------------------------------------------------
// Main
//---------------------------------------------------------

void TestDump(void)
{
	const char* songFilename = "0:/29_400Hz_48k_16bit.raw";


	PRINTF("TestDump... \n");
	PRINTF("EQ %d-BANDS\n", EQ_NUM_BANDS);

	if (!ReadFile(songFilename, &g_songSize, g_songAddr)) {
		PRINTF("ERR: cannot copy file %s\n", songFilename);	return;	}


//	test_dump_noDSP();
	test_dump_DSP();

	//stop and clean
	STA_Reset();
//	PRINT_OK_FAILED;
}


#endif //0
