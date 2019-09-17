/*
 * STAudioLib - staudio_test.c
 *
 * Created on 2013/04/22 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

#include "staudio_test.h"
#include "ff.h"		//filesystem


//-----------------------------------------------------------------
// Globals
//-----------------------------------------------------------------

int g_sta_test_failed = 0;

//s32 g_buf[G_BUF_WSIZE]; //max DMA transfer size +1

volatile s32* g_XIN0DMA = (volatile s32*)0x48d14000;

//-----------------------------------------------------------------
// Misc Functions
//-----------------------------------------------------------------
/*
void CHECKF(float val, float ref)
{
    if (ABSF(ref) > 1.192E-5)	//=100/2^23
    {
    	if (ABSF((val) - (ref)) > 0.01*ABSF(ref))
            Error();
    }
    else
    {
    	if (ABSF((val) - (ref)) > 1.192E-7)
            Error();
    }
}
*/
void CHECKF(float val, float ref)
{
	float ratioErr;

    // avoid divide by 0
	if (ref == 0.0)
	{
		ratioErr = ABSF(val - ref);
		if (ratioErr > 0.00001)
            Error();
	}
	else
	{
		ratioErr = ABSF(val / ref - 1);
		if (ratioErr > 0.01)
            Error();
	}
}

void CHECKD(double val, double ref, double maxRatioError)
{
	double ratioErr;

    // avoid divide by 0
	if (ref == 0.0)
	{
		ratioErr = ABSD(val - ref);
		if (ratioErr > 0.00001)
            Error();
	}
	else
	{
		ratioErr = ABSD(val / ref - 1);
		if (ratioErr > maxRatioError)
            Error();
	}
}

void CHECKDIFF(double val, double ref, double maxDiffError)
{
	double Err = ABSD(val - ref );
	if (Err > maxDiffError)
		Error();
}

void CHECKQ23(q23 val, q23 ref)
{
	//extend the sign bit from q23 to s32
	val = (val << 8) >> 8;
	ref = (ref << 8) >> 8;

	if (abs(val - ref) > 0x2)
		Error();
}

void Error(void)
{
	g_sta_test_failed = 1;
}

void SWAP_DP(u32 numSwap)
{
	u32 i;
	FORi(numSwap) {
	//	SLEEP(1);		//wait for DMA transfers completion
		P4_XIN0[127] = 1;	//swap dualport
		P4_XIN1[127] = 1;	//swap dualport
		P4_XIN2[127] = 1;	//swap dualport
		SLEEP(1);		//wait for DSP processing completion
    }
}


//-------------------------------------------------------------
// Support for FIFOs
//-------------------------------------------------------------
//NOT USED. Use DMA_SetTransfer() instead
/*
//FIFO2
void FIFO_SetDMA1C0WriteFrom(u32* buf, int numt, bool enableDMA)
//void DMA1C0_TransferToFIFO2(s32* buf, int numt, bool enableDMA)
{
	//DMA1: set transfer from buf to FIFO2
	DMA1_TCICR  = 0xFF;					//clear all TC irq
	DMA1_EICR   = 0xFF;					//clear all Err irq
	DMA1_C0CFG  = DMA1_CFG_MEM2FIFO2;	//disable C0
	DMA1_C0SADR = DMA_ADDR(buf);
	DMA1_C0DADR = (u32)FIFOWR_AHB;
//	DMA1_C0CR   = 0x074A0000 | (numt&0xFFF);	//s++, M1 to M1, DB=32, transfer = numt
//	DMA1_C0CR   = 0x07498000 | (numt&0xFFF);	//s++, M1 to M1, DB=16, transfer = numt
//	DMA1_C0CR   = 0x07490000 | (numt&0xFFF);	//s++, M1 to M1, DB=8, transfer = numt
//	DMA1_C0CR   = 0x07488000 | (numt&0xFFF);	//s++, M1 to M1, DB=4, transfer = numt
//	DMA1_C0CR   = 0x07480000 | (numt&0xFFF);	//s++, M1 to M1, DB=1, transfer = numt
	DMA1_C0CR   = DMA_SI | DMA_M2P | DMA_DBIT_32 | DMA_SBIT_32 | DMA_DBUSRT_1 | (numt&0xFFF);
	DMA1_C0CFG |= enableDMA & 1;	//mem to perip(DMA ctr, perip req), MEM to FIFO2
}


void FIFO_SetDMA1C1ReadTo(u32* buf, int numt, bool enableDMA)
{
	int i,tmp;

	//disable DMA1 C1
	DMA1_C1CFG  = 0;

	//FIFO1: reset and empty FIFO1. Set DMA read request when reach half fifo (16)
	FORi(32) FIFORD_IN[0] = 0;	//fill FIFO1 with 0 (32 deep) from P4 (AHB2DMA)
	FIFOS_CR |= 0x40010; 		//reset FIFO1 rd pointers, Fifo1MaxFL = 16
	AUDCR->ACO_CR.BIT.EnableRDFIFO = 1;

	//BUG? As soon as RdFIFO1 and DMA are enabled, the transfer happens even if the FIFO1 as not loaded.
	//HACK: prefetch FIFO1 into tmp
	DMA1_TCICR  = 0xFF;			//clear all TC irq
	DMA1_EICR   = 0xFF;			//clear all Err irq
	DMA1_C1CFG  = 0;			//disable C1
	DMA1_C1SADR = (u32)FIFORD_AHB;
	DMA1_C1DADR = DMA_ADDR(&tmp);
	DMA1_C1CR   = 0x01484000 | (32);//M1 to M0, SB=32, transfer = 32
	DMA1_C1CFG  = 0x1001;			//perip to mem (DMA ctr, perip req), FIFO1 to MEM
	SLEEP(1);

	//DMA1_C1: set transfers from FIFO1 to buf2
	DMA1_TCICR  = 0xFF;			//clear all TC irq
	DMA1_EICR   = 0xFF;			//clear all Err irq
	DMA1_C1CFG  = 0;			//disable C1
	DMA1_C1SADR = (u32)FIFORD_AHB;
	DMA1_C1DADR = DMA_ADDR(buf);
//	DMA1_C1CR   = 0x09484000 | (numt&0xFFF);	//d++, M1 to M0, SB=32, transfer = numt <-- WORKING
//	DMA1_C1CR   = 0x09483000 | (numt&0xFFF);	//d++, M1 to M0, SB=16, transfer = numt <-- WORKING
	DMA1_C1CR   = 0x0B483000 | (numt&0xFFF);	//d++, M1 to M1, SB=16, transfer = numt <-- WORKING

//	DMA1_C1CR   = 0x09480000 | (numt&0xFFF);	//d++, M1 to M0, SB=1,  transfer = numt <--
	DMA1_C1CFG  = 0x1000;						//perip to mem (DMA ctr, perip req), FIFO1 to MEM

	if (enableDMA)
		DMA1_C1CFG |= 1;						//enable
}

void FIFO1_FlushDMA1C1(void)
{
	//flush FIFO1
	//If channel still active (means transfer not complete), write dummy data to trigger last DMA burst.
	int i;
	if (DMA1_C1CFG & 1) {
		FORi(32) FIFORD_IN[0] = 0;
//		SLEEP(1);	//wait for the fifo to be read out.
	}
//	DPMBOX_CR |= 0x40000; 		//reset FIFO rd/wr pointers
}
*/



//-----------------------------------------------------------------
// MINI-PLAYER
//-----------------------------------------------------------------
// MSP2_SAI2_DMABUS_SAI3_DAC
// /!\ DMABUS must be programmed before !!
void Test_Play_PCM_48Khz_16bit(const u32* data, int size, int nch)
{
	const u8* pSrc = (const u8*) data;
	u32* pDmaBuf1  = (u32*)SDRAM_DMABUF1;

	int numSamples = size / 2;	//16 BIT
	int numLoaded  = 0;


	//reset buses (always first)
	DMA0_C0CFG  = 0;			//stop DMA0 C0
//	DMABUS_Stop();				//stop DMABus

	//SAI3 (master, enable)
	AIF->SAI3_TX1[0] = 0;
	AIF->SAI3_TX1[1] = 0;
	AIF->SAI3_CR.REG = SAI_I2S | SAI_MST | SAI_32_16VAL | SAI_RL | SAI_EN;

	//SAI2 (slave, enable)
	AIF->SAI2_CR.REG = SAI_16_16VAL | SAI_EN;

	//MSP2
	MSP_Reset(MSP2);
	MSP_ConfigSampleRateGen(MSP2, 52000000, 48000, 32, 16);	//2x16 bit
	MSP2->MSP_GCR.REG = MSP_SCK_52MHZ | MSP_TFS_OUT | MSP_TCK_OUT | MSP_TCKPOL_DW | MSP_TFSPOL_L | MSP_TFEN;	//master i2s
	MSP2->MSP_TCF.REG = MSP_P1_32BIT | MSP_P1LEN(1) | MSP_FSIG;	//1x32 bits
	MSP2->MSP_DMACR.REG = MSP_TDMAEN;

/*	//DMABUS
	//set transfers from SAI2 to SAI3
	STA_DMAReset();
	STA_DMAAddTransfer(STA_DMA_SAI2_RX1,  STA_DMA_SAI3_TX1, nch);
	if (nch == 1)
		DMABUS_TX[1] = STA_DMA_SAI3_TX1 << 16 | (STA_DMA_SAI3_TX1+1); //R (copy from L)
//	DMABUS_SelfStop();	//HACK to self-stop the DMABUS
*/

	//DMA BUF
	memset(pDmaBuf1, 0, SDRAM_DMABUF1_SIZE);

	//START
//	DMABUS_Start();				//start DMABUS
	MSP_EnableTx(MSP2);			//start MSP2 (master)
//	DMA0_C0CFG |= 1;			//start DMA0C0

	while (numLoaded < numSamples) {

		//int numDMAt = 500; //4095 max; //num dma transfer
		//observation: 4095 gives "helicopter" sound. Might be too much to load (memcpy)
		//             1000 no helicopter sound but a bit noisy
		//             500  gives clear sound
		u32 numDMAt = MIN(500, (numSamples-numLoaded)/2); //num of dma transfer (2x16 bits)


		//preload samples
//		if (pDmaBuf1 == SDRAM_DMABUF1) {
			memcpy(pDmaBuf1, pSrc, numDMAt*4);
			pSrc      += numDMAt*4;
			numLoaded += numDMAt*2;
//		}

		while (DMA0_C0CFG & 1);	//wait for the previous transfer to be completed...

		//DMA0_C0 transfers
		DMA_SetTransfer(DMA0, 0, pDmaBuf1, (u32*)&MSP2_DR, DMA_SI | DMA_M2P | DMA_SBIT_32 | DMA_DBIT_32 | DMA_DBUSRT_4,
						DMA0_CFG_MEM2MSP2 | DMA_ENABLE, numDMAt);

		SLEEP(numDMAt/48); //avoid sticky loop (4095/48000 = 0.085 s)
		while (DMA0_C0CFG & 1);	//wait for the previous transfer to be completed...
	}

	//stop
	MSP_DisableTx(MSP2);
//	DMABUS_Stop();
	AIF->SAI2_CR.BIT.EN = 0;
	AIF->SAI3_CR.BIT.EN = 0;
	AIF->SAI3_TX1[0] = 0;
	AIF->SAI3_TX1[1] = 0;
}
//------------------------------------------------------------------------
//                    -> must be 32bit(24valid) from here
//FLOW: SDRAM -> DMA1 -> FIFOWR -> DMABUS -> DSP -> SAI3 -> DAC
//
//NOTE: with this flow, we have to unpack from 2x16 to 2x32(24valid) on the fly
//		when loading from SDRAM
//
// /!\ DMABUS must be programmed before !!
//
void Test_Play_PCM_48Khz_16bit_FIFO2(const u32* data, int size, int nch)
{
	const u8* pSrc = (const u8*) data;
	u32* pDmaBuf1  = (u32*)SDRAM_DMABUF1;

	int numSamples = size / 2;	//16 BIT
	int numLoaded  = 0;

	ASSERT(nch == 2);


	//STOP buses (always first)
	DMA1_C0CFG  = 0;			//stop DMA1 C0
//	DMABUS_Stop();				//stop DMABus

	//FIFO2
	FIFO_Reset(16, 16);			//reset FIFO
	FIFO_SetDMAWriteRequest(TRUE);	//enable DMA req to write to FIFO

	//SAI3 (master, enable)
	AIF->SAI3_TX1[0] = 0;
	AIF->SAI3_TX1[1] = 0;
	AIF->SAI3_CR.REG = SAI_I2S | SAI_MST | SAI_32_16VAL | SAI_RL | SAI_EN;

	//START
//	DMABUS_Start();				//start DMABUS


	while (numLoaded < numSamples) {

		//observation: 4095 gives "helicopter" sound. Might be too much to load (memcpy)
		//             100  gives clear sound after a while
		//             50   gives clear sound (and is the minimum to allow SLEEP(1)
		u32 numDMAt = MIN(50, (numSamples-numLoaded)/2); //num of dma transfer (2x16 bits)

		//preload samples
//		if (pDmaBuf1 == SDRAM_DMABUF1) {
			Convert_16to32(pSrc, numDMAt*4, pDmaBuf1, NULL);
			pSrc      += numDMAt*4;
			numLoaded += numDMAt*2;
//		}

		while (DMA1_C0CFG & 1);	//wait for the previous transfer to be completed...

		//DMA1_C0 transfers
		DMA_SetTransfer(DMA1, 0, pDmaBuf1, (u32*)FIFOWR_AHB,
						DMA_SI | DMA_M2P | DMA_SBIT_32 | DMA_DBIT_32 | DMA_DBUSRT_4,
						DMA1_CFG_MEM2FIFO2 | DMA_ENABLE, numDMAt * 2); //from 1x32 to 2x32

//		SLEEP(numDMAt/48); //avoid sticky loop (4095/48000 = 0.085 s)
		SLEEP(1);
		while (DMA1_C0CFG & 1);	//wait for the previous transfer to be completed...
	}

	//stop
//	DMABUS_Stop();
	FIFO_SetDMAWriteRequest(FALSE);	//disable DMA req to write to FIFO

//	AIF->SAI3_CR.BIT.EN = 0;
	AIF->SAI3_TX1[0] = 0;
	AIF->SAI3_TX1[1] = 0;
}

//------------------------------------------------------------------------
// MSP2_SAI2_DMABUS_SAI3_DAC
// /!\ DMABUS must be programmed before !!
//TODO: only works for MONO
void Test_Play_PCM_48Khz_24of32bit(const u32* data, int size, int nch)
{
	const u32* pSrc = data;
	u32* pDmaBuf1   = (u32*)SDRAM_DMABUF1;

	int numSamples = size / 4;	//32 BIT (24 valid)
	int numLoaded  = 0;

	ASSERT(nch == 1); //<---- MONO

	//reset buses (always first)
	DMA0_C0CFG  = 0;			//stop DMA0 C0
	DMABUS_Stop();				//stop DMABus

	//SAI3 (master, enable)
	AIF->SAI3_TX1[0] = 0;
	AIF->SAI3_TX1[1] = 0;
	AIF->SAI3_CR.REG = SAI_32_24VAL | SAI_I2S | SAI_MST | SAI_EN;

	//SAI2 (slave, enable)
	AIF->SAI2_CR.REG = SAI_32_24VAL | SAI_DIR_LSB | SAI_EN;

	//MSP2
	MSP_Reset(MSP2);
	MSP_ConfigSampleRateGen(MSP2, 52000000, 48000, 64, 32);	//00LLLLLL_00000000 bit
	MSP2->MSP_GCR.REG = MSP_SCK_52MHZ | MSP_TFS_OUT | MSP_TCK_OUT | MSP_TCKPOL_DW | MSP_TFSPOL_L | MSP_TFEN; //master i2s
	MSP2->MSP_TCF.REG = MSP_P1_32BIT | MSP_P1LEN(1) | MSP_LSBIT;// | MSP_FSIG;	//1x32 bits
	MSP2->MSP_DMACR.REG = MSP_TDMAEN;

	//DMABUS
	//Should be set before

	//DMA BUF
	memset(pDmaBuf1, 0, SDRAM_DMABUF1_SIZE);

	//START
	DMABUS_Start();				//start DMABUS
	MSP_EnableTx(MSP2);			//start MSP2 (master)
//	DMA0_C0CFG |= 1;			//start DMA0C0

	while (numLoaded < numSamples) {

		//int numDMAt = 500; //4095 max; //num dma transfer
		//observation: 4095 gives "helicopter" sound. Might be too much to load (memcpy)
		//             1000 no helicopter sound but a bit noisy
		//             500  gives clear sound
		u32 numDMAt = MIN(500, (numSamples-numLoaded)); //num of dma transfer (32 bits)


		//preload samples
//		if (pDmaBuf1 == SDRAM_DMABUF1) {
			memcpy(pDmaBuf1, pSrc, numDMAt*4);
			pSrc      += numDMAt;
			numLoaded += numDMAt;
//		}

		while (DMA0_C0CFG & 1);	//wait for the previous transfer to be completed...

		//DMA0_C0 transfers
		DMA_SetTransfer(DMA0, 0, pDmaBuf1, (u32*)&MSP2_DR, DMA_SI | DMA_M2P | DMA_SBIT_32 | DMA_DBIT_32 | DMA_DBUSRT_4,
						DMA0_CFG_MEM2MSP2 | DMA_ENABLE, numDMAt);

		SLEEP(numDMAt/48); //avoid sticky loop (4095/48000 = 85 ms)
		while (DMA0_C0CFG & 1);	//wait for the previous transfer to be completed...
	}

	//stop
	MSP_DisableTx(MSP2);
	DMABUS_Stop();
	AIF->SAI2_CR.BIT.EN = 0;
	AIF->SAI3_CR.BIT.EN = 0;
	AIF->SAI3_TX1[0] = 0;
	AIF->SAI3_TX1[1] = 0;
}
//--------------------------------------------------------------------------
//						  LR		only L channel
//DMA BUF -> DMA0 -> MSP2 -> SAI2 -> DMABUS -> FIFO1 -> DMA1 -> DST_BUF
//returns the number of dumped samples
u32 Test_Dump_PCM_48Khz_16_to_24of32bit(const u32* src, u32* dst, u32 numSamples, u32 ch)
{
	u16* srcbuf = (u16*)src;	//0xLLLLRRRR
	u32* dstbuf = (u32*)dst;	//0x00LLLL00 0x00RRRR00

	u32 numLoaded  = 0;


	ASSERT(ch == 2);

	//reset buses and masters
	DMA0_C0CFG  = 0;			//stop DMA0 C0
	DMA1_C1CFG  = 0;			//stop DMA1 C1 (FIFOs only works with DMA1)
	DMABUS_Stop();				//reset DMABus

	//DST BUF
	//reset some bytes
	memset((void*)dst, 0, 40 * 4);

	//SAI2 (slave, enable)
	AIF->SAI2_CR.REG = SAI_16_16VAL | SAI_SLV | SAI_EN;

	//MSP2 (master) -> SAI2
	MSP_Reset(MSP2);
	MSP_ConfigSampleRateGen(MSP2, 52000000, 48000, 32, 16);	//2x16 bit
//	MSP_ConfigSampleRateGen(MSP2, 51200000, 48000, 32, 16);	//2x16 bit
	MSP2->MSP_GCR.REG = MSP_SCK_52MHZ | MSP_TFS_OUT | MSP_TCK_OUT | MSP_TCKPOL_DW | MSP_TFSPOL_L | MSP_TFEN; //master i2s
	MSP2->MSP_TCF.REG = MSP_P1_32BIT | MSP_P1LEN(1) | MSP_FSIG;	//1x32 bits
	MSP2->MSP_DMACR.REG = MSP_TDMAEN;

	//DMABUS: SAI2 -> FIFO1
//	DMABUS_Reset();
//	DMABUS_AddTransfers(DMABUS_SAI2_RX1,	DMABUS_FIFO_IN, ch);	//SAI2 to FIFO1 (ch)

	//FIFO1
	FIFO_Reset(16, 16);			//reset FIFO
	FIFO_SetDMAReadRequest(TRUE);

	//START
	DMABUS_Start();				//start DMABUS
	MSP_EnableTx(MSP2);			//start MSP2 (master)


	while (numLoaded < numSamples)
	{
		//note: DMA1 must transfer twice the number of 32bit words than DMA0
		u32 numDMAt = MIN(4095/2, (numSamples-numLoaded)/2); //LR 16 bits (dma transfer in words)


		//DMA1_C1: FIFO1 -> DST_BUF
		/////////////////////////
		//Note: MSP2 is > ~48kHz, thus some samples are missed by the DMABUS
		//HACK: remove some transfers from DMA1...
		/////////////////////////
		while (DMA1_C1CFG & 1); //wait for the previous transfer to be completed...
		DMA_SetTransfer(DMA1, 1, (u32*)FIFORD_AHB, &dstbuf[numLoaded],				//src, dst
					DMA_DI | DMA_P2M | DMA_SBIT_32 | DMA_DBIT_32 | DMA_SBUSRT_4, 	//CR
					DMA_P2M_DMACTL | DMA_SREQ(DMA1_DSPFIFO1)| DMA_ENABLE, numDMAt*2); //CFG

		//DMA0_C0: DMA_BUF -> MSP2
		while (DMA0_C0CFG & 1);	//wait for the previous transfer to be completed...
		DMA_SetTransfer(DMA0, 0, (u32*)&srcbuf[numLoaded], (u32*)&MSP2_DR,				//src, dst
					DMA_SI | DMA_M2P | DMA_SBIT_32 | DMA_DBIT_32 | DMA_DBUSRT_1, 	//CR
					DMA0_CFG_MEM2MSP2 | DMA_ENABLE, numDMAt);						//CFG

		SLEEP(numDMAt/48); 		//avoid sticky loop (4095/48000 = 85 ms)
		while (DMA0_C0CFG & 1);	//wait for the previous transfer to be completed...


		//loaded samples
		numLoaded += numDMAt * 2;
	}

	FIFO_FlushDMARead(1);
	SLEEP(1);
	FIFO_SetDMAReadRequest(FALSE);

	//STOP
	DMA0_C0CFG  = 0;			//stop DMA0 C0
	DMA1_C1CFG  = 0;			//stop DMA1 C1 (FIFOs only works with DMA1)
	DMABUS_Stop();				//reset DMABus
	MSP_DisableTx(MSP2);

	return numLoaded;
}

//-----------------------------------------------------------------
// Miniplayer
//-----------------------------------------------------------------
#if 0
PLAYER g_player = {0};


void PLY_Init(void)
{
	s32 ret;

	g_player.m_data = 0;


	//create player tasks
	if (!g_player.m_htask)
	{
		ret = xTaskCreate( (pdTASK_CODE) PLY_Task, (signed char*) "Player task", 1000, NULL, 120, &g_player.m_htask);
		if (ret != pdPASS) {
			PRINTF("PLY_Init: xTaskCreate() failed!\n"); g_player.m_htask = 0; return;}
	}

}


void PLY_Set_PCM_48Khz_16bit_FIFO2(const u32* data, int numBytes, int nch)
{
	//input
	g_player.m_data 		= data;
	g_player.m_numBytes 	= numBytes;
	g_player.m_nch			= nch;
	g_player.m_bitsPerCh	= 16;
	g_player.m_freq			= 48000;

	//internal
	g_player.m_pdata 		= (u8*) data;
	g_player.m_bytesPerCh	= g_player.m_bitsPerCh / 8;
	g_player.m_numSamples	= numBytes / g_player.m_bytesPerCh;
	g_player.m_numPlayed	= 0;

	g_player.m_source1	= PLY_FIFO2;

	//STOP buses (always first)
	DMA1_C0CFG  = 0;			//stop DMA1 C0

	//FIFO2
	FIFO_Reset(16, 16);			//reset FIFO
}

void PLY_Set_PCM_48Khz_16bit_MSP2(const u32* data, int numBytes, int nch)
{
	//input
	g_player.m_data 		= data;
	g_player.m_numBytes 	= numBytes;
	g_player.m_nch			= nch;
	g_player.m_bitsPerCh	= 16;
	g_player.m_freq			= 48000;

	//internal
	g_player.m_pdata 		= (u8*) data;
	g_player.m_bytesPerCh	= g_player.m_bitsPerCh / 8;
	g_player.m_numSamples	= numBytes / g_player.m_bytesPerCh;
	g_player.m_numPlayed	= 0;

	g_player.m_source1	= PLY_MSP2;

	//STOP buses (always first)
	DMA0_C0CFG  = 0;			//stop DMA1 C0

	//MSP2 (master)
	MSP_Reset_(MSP2);
	MSP_ConfigSampleRateGen(MSP2, 52000000, g_player.m_freq, 32, g_player.m_bitsPerCh);	//2x16 bit
	MSP2->MSP_GCR.REG = MSP_SCK_52MHZ | MSP_TFS_OUT | MSP_TCK_OUT | MSP_TCKPOL_DW | MSP_TFSPOL_L | MSP_TFEN;	//master i2s
	MSP2->MSP_TCF.REG = MSP_P1_32BIT | MSP_P1LEN(1) | MSP_FSIG;	//1x32 bits
	MSP2->MSP_DMACR.REG = MSP_TDMAEN;
}


void PLY_Set_PCM_MSP2(const u32* data, int numBytes, int nch, int bitsPerCh, int freq)
{
	//input
	g_player.m_data 		= data;
	g_player.m_numBytes 	= numBytes;
	g_player.m_nch			= nch;
	g_player.m_bitsPerCh	= bitsPerCh;
	g_player.m_freq			= freq;

	//internal
	g_player.m_pdata 		= (u8*) data;
	g_player.m_bytesPerCh	= g_player.m_bitsPerCh / 8;
	g_player.m_numSamples	= numBytes / g_player.m_bytesPerCh;
	g_player.m_numPlayed	= 0;

	g_player.m_source1	= PLY_MSP2;

	//STOP buses (always first)
	DMA0_C0CFG  = 0;			//stop DMA1 C0

	//MSP2 (master)
	MSP_Reset_(MSP2);
	MSP_ConfigSampleRateGen(MSP2, 52000000, g_player.m_freq, 32, g_player.m_bitsPerCh);	//2x16 bit
	MSP2->MSP_GCR.REG = MSP_SCK_52MHZ | MSP_TFS_OUT | MSP_TCK_OUT | MSP_TCKPOL_DW | MSP_TFSPOL_L | MSP_TFEN;	//master i2s
	MSP2->MSP_TCF.REG = MSP_P1_32BIT | MSP_P1LEN(1) | MSP_FSIG;	//1x32 bits
	MSP2->MSP_DMACR.REG = MSP_TDMAEN;

	//SAI2 / SAI3
	//set in PLY_SetState(PLY_PLAY)

	//AIMUX / AOMUX  --> LPFSRC_0
	AIF->AIMUX.BIT.CH0SEL = AIMUX_SAI2RX0;
	AIF->AOMUX.BIT.CH0SEL = AOMUX_SRCDO;

	//LPFSRC 0
	if (freq == 16000)
		AIF->LPF_CR.BIT.FS_0 = LPF0_UPx2;

	else if (freq == 48000)
		AIF->LPF_CR.BIT.FS_0 = LPF_BYPASS;

}

void PLY_Set_PCM_MSP(MSP_TypeDef *MSP, const u32* data, int numBytes, int nch, int bitsPerCh, int freq)
{
	//input
	g_player.m_data 		= data;
	g_player.m_numBytes 	= numBytes;
	g_player.m_nch			= nch;
	g_player.m_bitsPerCh	= bitsPerCh;
	g_player.m_freq			= freq;

	//internal
	g_player.m_pdata 		= (u8*) data;
	g_player.m_bytesPerCh	= g_player.m_bitsPerCh / 8;
	g_player.m_numSamples	= numBytes / g_player.m_bytesPerCh;
	g_player.m_numPlayed	= 0;

	g_player.m_source1	= PLY_MSP2;

	//STOP buses (always first)
	DMA0_C0CFG  = 0;			//stop DMA1 C0

	//MSP2 (master)
	MSP_Reset_(MSP2);
	MSP_ConfigSampleRateGen(MSP2, 52000000, g_player.m_freq, 32, g_player.m_bitsPerCh);	//2x16 bit
	MSP2->MSP_GCR.REG = MSP_SCK_52MHZ | MSP_TFS_OUT | MSP_TCK_OUT | MSP_TCKPOL_DW | MSP_TFSPOL_L | MSP_TFEN;	//master i2s
	MSP2->MSP_TCF.REG = MSP_P1_32BIT | MSP_P1LEN(1) | MSP_FSIG;	//1x32 bits
	MSP2->MSP_DMACR.REG = MSP_TDMAEN;

	//SAI2 / SAI3
	//set in PLY_SetState(PLY_PLAY)

	//AIMUX / AOMUX  --> LPFSRC_0
	AIF->AIMUX.BIT.CH0SEL = AIMUX_SAI2RX0;
	AIF->AOMUX.BIT.CH0SEL = AOMUX_SRCDO;

	//LPFSRC 0
	if (freq == 16000)
		AIF->LPF_CR.BIT.FS_0 = LPF0_UPx2;

	else if (freq == 48000)
		AIF->LPF_CR.BIT.FS_0 = LPF_BYPASS;

}

void PLY_SetState(u32 state)
{

	if (state == PLY_PLAY)
	{
		if (!g_player.m_htask) {
			PRINTF("PLY: No player Task!\n");
			g_player.m_state = PLY_STOP;
			return;
		}
		if (!g_player.m_data) {
			PRINTF("PLY: No data!\n");
			g_player.m_state = PLY_STOP;
			return;
		}

		PRINTF("PLY: PLAY...\n");

		//FIFO2
		if (g_player.m_source1 == PLY_FIFO2)
			FIFO_SetDMAWriteRequest(TRUE);	//enable DMA req to write to FIFO

		//MSP2/SAI2
		if (g_player.m_source1 == PLY_MSP2) {

			//SAI2 (slave, enable)
			AIF->SAI2_CR.REG = SAI_16_16VAL | SAI_EN;

			//MSP2 (master enable)
			MSP_EnableTx(MSP2);
		}

		//SAI3 (master, enable)
		AIF->SAI3_CR.REG = SAI_I2S | SAI_MST | SAI_32_16VAL | SAI_RL | SAI_EN;

	}
	else if (state == PLY_PAUSE)
	{
		PRINTF("PLY: PAUSE...\n");

		//stop FIFO2
		if (g_player.m_source1 == PLY_FIFO2)
			FIFO_SetDMAWriteRequest(FALSE);	//disable DMA req to write to FIFO

		//stop MSP2/SAI2
		if (g_player.m_source1 == PLY_MSP2) {
			MSP_DisableTx(MSP2);
			AIF->SAI2_CR.BIT.EN = 0;
		}

		//SAI3: keep running

		//AIF->SAI3_TX1[0] = 0;
		//AIF->SAI3_TX1[1] = 0;
	}
	else if (state == PLY_STOP)
	{
		PRINTF("PLY: STOP\n");

		//stop DMA1
		DMA1_C0CFG  = 0;			//stop DMA1 C0

		//stop FIFO2
		if (g_player.m_source1 == PLY_FIFO2)
			FIFO_SetDMAWriteRequest(FALSE);	//disable DMA req to write to FIFO

		//stop MSP2/SAI2
		if (g_player.m_source1 == PLY_MSP2) {
			MSP_DisableTx(MSP2);
			AIF->SAI2_CR.BIT.EN = 0;
		}

		//SAI3: keep running

		//reset internal
		g_player.m_pdata 		= (u8*) g_player.m_data;
		g_player.m_numPlayed	= 0;
	}

	g_player.m_state = state;
}

void PLY_TogglePlayPause(void)
{
	PLY_SetState(g_player.m_state == PLY_PLAY ? PLY_PAUSE : PLY_PLAY);
}

void PLY_Task(void)
{
	u32* pDmaBuf1  = (u32*)SDRAM_DMABUF1;


	while (1)
	{

		//TODO: this is only for PCM_48Khz_16bit_FIFO2
		//
		while (g_player.m_state == PLY_PLAY && g_player.m_numPlayed < g_player.m_numSamples)
		{
			//PLY_FIFO2
			if (g_player.m_source1 == PLY_FIFO2)
			{
				//observation: 4095 gives "helicopter" sound. Might be too much to load (memcpy)
				//             100  gives clear sound after a while
				//             50   gives clear sound (and is the minimum to allow SLEEP(1)
				u32 numDMAt = MIN(50, (g_player.m_numSamples - g_player.m_numPlayed)/2); //num of dma transfer (2x16 bits)

				//preload samples
				Convert_16to32(g_player.m_pdata, numDMAt*4, pDmaBuf1, NULL);
				g_player.m_pdata     += numDMAt*4;
				g_player.m_numPlayed += numDMAt*2;

				while (DMA1_C0CFG & 1);	//wait for the previous transfer to be completed...

				//DMA1_C0 transfers
				DMA_SetTransfer(DMA1, 0, pDmaBuf1, (u32*)FIFOWR_AHB,
								DMA_SI | DMA_M2P | DMA_SBIT_32 | DMA_DBIT_32 | DMA_DBUSRT_4,
								DMA1_CFG_MEM2FIFO2 | DMA_ENABLE, numDMAt * 2); //from 1x32 to 2x32

				SLEEP(1);
				while (DMA1_C0CFG & 1);	//wait for the previous transfer to be completed...
			}

			//PLY_MSP2
			else
			{
				u32 numDMAt = MIN(4000, (g_player.m_numSamples - g_player.m_numPlayed)/2); //num of dma transfer (2x16 bits)

				while (DMA0_C0CFG & 1);	//wait for the previous transfer to be completed...

				//DMA0_C0 transfers
				DMA_SetTransfer(DMA0, 0, (u32*)g_player.m_pdata, (u32*)&MSP2_DR, DMA_SI | DMA_M2P | DMA_SBIT_32 | DMA_DBIT_32 | DMA_DBUSRT_4,
								DMA0_CFG_MEM2MSP2 | DMA_ENABLE, numDMAt);

				SLEEP(4000 * 1000 / g_player.m_freq - 1);
				while (DMA1_C0CFG & 1);	//wait for the previous transfer to be completed...

				g_player.m_pdata     += numDMAt*4;
				g_player.m_numPlayed += numDMAt*2;
			}

		}

		//repeat mode
		if (g_player.m_state == PLY_PLAY && g_player.m_numPlayed >= g_player.m_numSamples) {
			g_player.m_pdata 		= (u8*) g_player.m_data;
			g_player.m_numPlayed	= 0;
		}


		SLEEP(10);
	}
}

#endif //0 miniplayer



