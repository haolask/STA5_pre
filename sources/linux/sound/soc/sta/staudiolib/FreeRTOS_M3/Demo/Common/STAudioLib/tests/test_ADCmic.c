/*
 * STAudioLib - test_ADCmin.c
 *
 * Created on 2013/04/22 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	Recording:
	ADCMIC -> SAI3RX1 -> DMABUS -> FIFO1 -> DMA1 -> DST_BUF

	Playing:
	DST_BUF -> DMA0 -> MSP2 -> SAI2 -> DMABUS -> SAI3 -> DAC

*/


#if 0
#include "staudio_test.h"

#define R4ESRAM_BASE		0x70000000
#define R4ESRAM_SIZE		0x100000

#define DST_BUF				R4ESRAM_BASE
#define DST_BUFSIZE			R4ESRAM_SIZE

//#define ADCMIC_CHSEL		1
//#define ADCMIC_STBY_WAIT	1000
//#define NUM_SAMPLES		8192

static bool g_record = 0;
static bool g_play = 0;

//---------------------------------------------------------
// KEY / ROTARY
//---------------------------------------------------------
static void rtxCallback(int steps)
{
}


static void keyCallback(int key, bool down)
{

	switch(key) {
	case KEY_3: g_record ^= 1;
				PRINTF( g_record ? "Recording ADCMIC...\n" : "Stop Recording\n");
				break;

	case KEY_4: g_play ^= 1;
				PRINTF( g_play ? "Playing...\n" : "Stop Playing\n");
				break;
	}
}


//---------------------------------------------------------
// TEST ADCMIC
//---------------------------------------------------------


//ADCMIC -> SAI3RX -> DMABUS -> FIFO1 -> DMA1 -> DST_BUF

//if numSamples = -1 use g_record
//returns the number of samples recorded
static u32 record_ADCMIC_FIFO1(u32* dstbuf, int wsize, int numSamples)
{
	u32 numRecorded = 0;


	//reset buses and masters (always first)
	DMA1_C1CFG  = 0;			//stop DMA1 C1 (FIFOs only works with DMA1)
	STA_DMAStop();

	//DST BUF [OPTIONAL]
	if (numSamples > 0)
		memset((void*)dstbuf, 0, numSamples * 4 + 64); //32 bits


	//SAI3_RX (master, enable)
	AIF->SAI3_CR.REG = SAI_I2S | SAI_MST | SAI_32_24VAL | SAI_EN;

	//DMABUS: SAI3 -> FIFO1
	STA_DMAReset();
	STA_DMAAddTransfer(DMABUS_SAI3_RX1, DMABUS_FIFO_IN, 1);	//SAI3 to FIFO1 (1 ch)

	//FIFO1
	FIFO_Reset(16, 16);			//reset FIFO
	FIFO_SetDMAReadRequest(TRUE);

	//START RECORDING
	STA_DMAStart();

	while (numRecorded < numSamples || g_record)
	{
		u32 numDMAt = (numSamples > 0) ? MIN(4095, (numSamples-numRecorded)) : 4095;

		if (numRecorded + numDMAt >= wsize){
			PRINTF("Out of memory ! Stop recording.\n");
			break;
		}

		//DMA1: FIFO1 -> dstbuf
		DMA_SetTransfer(DMA1, 1, (u32*)FIFORD_AHB, &dstbuf[numRecorded],
						DMA_DI | DMA_P2M | DMA_SBIT_32 | DMA_DBIT_32 | DMA_SBUSRT_1, 	//CR
						DMA_P2M_DMACTL | DMA_SREQ(DMA1_DSPFIFO1)| DMA_ENABLE, numDMAt); //CFG

		//recording....
		SLEEP(numDMAt/48);
		DMA_WaitAvailable(DMA1, 1);

		numRecorded += numDMAt;
	}

	FIFO_FlushDMARead(1); //flush to dma1 channel1
	SLEEP(1);
	FIFO_SetDMAReadRequest(FALSE);

	//STOP
	STA_DMAStop();
	g_record = 0;
	return numRecorded;
}

//---------------------------------------------------------
// Play mono 1x32(24valid) via MSP2
//---------------------------------------------------------

static void play(int numSamples)
{
	//DMABUS: SAI2 -> SAI3
	STA_DMAReset();
	STA_DMAAddTransfer(STA_DMA_SAI2_RX1,  STA_DMA_SAI3_TX1,   1);	//L
	STA_DMAAddTransfer(STA_DMA_SAI2_RX1,  STA_DMA_SAI3_TX1+1, 1);	//R

	Test_Play_PCM_48Khz_24of32bit((const u32*)DST_BUF, numSamples * 4, 1);

}

//---------------------------------------------------------
// Main
//---------------------------------------------------------

void TestADCmic(void)
{
	int numSamples  = -1;
	int numRecorded = 0;
	u32 numBytes24;

	PRINTF("TestADCmic... \n");

	//user control
	KPD_Init(keyCallback);

	//ADCMIC
	ADCMIC_Init(0);		//0:AIN1(mic_ext),  1:MICIN
	SLEEP(500);		//it seems that the ADCMIC needs some warmup-time....


	//looping
	while (1)
	{
		if (g_record || numSamples > 0) {
			numRecorded = record_ADCMIC_FIFO1( (u32*)DST_BUF, DST_BUFSIZE/4, numSamples); //exit after finished recording
			numSamples = -1; //record once only

			//convert from 32bit (24valid left align) to 24 bit
			//Convert_32to24((void*)DST_BUF, numRecorded * 4, (void*)DST_BUF, &numBytes24);
		}

		if (g_play)
			play(numRecorded); //exit after finished playing (can't stop)

		SLEEP(10); //avoid sticky loop
	}


}

#endif //0
