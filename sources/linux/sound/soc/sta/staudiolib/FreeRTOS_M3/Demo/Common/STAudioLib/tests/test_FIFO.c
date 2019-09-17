/*
 * STAudioLib - test_FIFO.c
 *
 * Created on 2013/10/23 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	Testing...

	     DMA1C0       DMABUS     DMABUS       DMA1C1
	buf1 -----> FIFO2 -----> XIN -----> FIFO1 -----> buf2

	DMABUS @48kHz
	DMA1 in full duplex

note:
	FIFORD_AHB <=> DMABUS_FIFO_TX  (DMA1_REQ = 0)
	FIFOWR_AHB <=> DMABUS_FIFO_RX  (DMA1_REQ = 1)

 */
#if 0
#include "staudio_test.h"

//static s32 g_buf1[128]; <-- use g_buf in staudio_test.c
#define g_buf1		g_buf
static s32 g_buf2[128];


//--- test FIFO simple ------------------------------------------------------

static void test_FIFOs_simple(void)
{
	int i;

	//MEM
	FORi(128) {g_buf1[i] = 0; g_buf2[i] = 0;}

	//FIFO CR
	FIFOS_CR = 0x60000; //reset FIFOs ptrs

	//write to FIFOs (32 deep)
	FORi(32) FIFORD_IN[0] = (i+1);
	FORi(32) FIFOWR_AHB[0] = (i+1) << 16;

	//read from FIFOs
	FORi(32) g_buf1[i] = FIFORD_AHB[0];
	FORi(32) g_buf2[i] = FIFOWR_OUT[0];

	//check
	FORi(32) CHECK(g_buf1[i] == (i+1));
	FORi(32) CHECK(g_buf2[i] == (i+1) << 16);
}

//The fifos works as circular buffer (ie. overflow over writes data)
static void test_FIFO_overflow(void)
{
	int i;

	//write to FIFOs (32 deep)
	FORi(64) FIFORD_IN[0] = (i+1);

	//read from FIFOs
	FORi(32) g_buf1[i] = FIFORD_AHB[0];
}

static void test_FIFO_resetPtr(void)
{
	int i;

	FORi(128) {g_buf1[i] = 0; g_buf2[i] = 0;}

	//write to FIFOs (32 deep)
	FORi(10) FIFORD_IN[0] = (i+1);

	FIFOS_CR = 0x60000; //reset FIFOs ptrs

	FORi(32) FIFORD_IN[0] = (i+1) + 10;

	//read from FIFOs
	FORi(32) g_buf1[i] = FIFORD_AHB[0];

}

//--- test FIFO with DMABUS ------------------------------------------------------

//DMABUS transfers a burst of numt data
//WORKING
/*
static void test_FIFO2_DMABUS_FIFO1(void)
{
	int i, numt = 32; //max 32 !!!

	//MEM
	FORi(128) {g_buf1[i] = 0; g_buf2[i] = 0;}

	//write to FIFO2 (32 deep)
	FIFO_Reset(16, 16);
	FORi(numt) FIFOWR_AHB[0] = i+1;

	//DMABUS: transfer numt words from FIFO2 to FIFO1
	STA_DMAReset();
//	FORi(numt) STA_DMAAddTransfer(STA_DMA_FIFO_OUT, STA_DMA_FIFO_IN, 1);	//from FIFO2 to FIFO1
	FORi(numt) DMABUS_TX[i] = STA_DMA_FIFO_OUT << 16 | STA_DMA_FIFO_IN;	//from FIFO2 to FIFO1

	//HACK to self-stop the DMABUS after completed the previous transfers by writing to DMABUS_CR
	AIF->LPF0_DI[0].REG = 0; //value to be transfered to DMABUS_CR by itself (e.g. to self-disable the DMABUS)
	DMABUS_TX[numt] = STA_DMA_LPF0_DI << 16 | 0x1000;	//add as last dma transfer

	STA_DMAStart();
	SLEEP(1);
	STA_DMAReset();

	//Read back FIFO1
	FORi(numt) g_buf2[i] = FIFORD_AHB[0];

	//check
	FORi(numt) CHECK(g_buf2[i] == i+1);
}
*/

//same but with DMABUS transfering 1 data per frame
//WORKING
static void test_FIFO2_DMABUS_FIFO1(void)
{
	int i, numt = 32; //max 32 !!!

	//MEM
	FORi(128) {g_buf1[i] = 0; g_buf2[i] = 0;}

	//FIFO2: write (32 deep)
	FIFO_Reset(16, 16);
	FORi(numt) FIFOWR_AHB[0] = i+1;

	//DMABUS: set transfer numt words from FIFO2 to FIFO1
	STA_DMAReset();
	STA_DMAAddTransfer(STA_DMA_FIFO_OUT, STA_DMA_FIFO_IN, 1);	//from FIFO2 to FIFO1

	//start
	STA_DMAStart();
	SLEEP(10);

	//stop
	STA_DMAReset();

	//Read back FIFO1
	FORi(numt) g_buf2[i] = FIFORD_AHB[0];

	//check
	FORi(numt) CHECK(g_buf2[i] == i+1);
}

//--- test FIFOs with DMAC1 ------------------------------------------------------

//Can transfer to FIFO2 any number of data
//WORKING
/*
static void test_MEM_DMA1_FIFO2(void)
{
	const int numt = 66; //max 128 (size of buf1 and buf2)
	int i;

	//MEM
	FORi(128) {g_buf1[i] = i+1; g_buf2[i] = 0;}

	//FIFO2: set DMA write request
	FIFO_Reset(16, 16);
//	DPMBOX_CR = 0x60000; //reset FIFO rd/wr pointers, Fifo2MinFL = 32, Fifo1MaxFL = 32
	DPMBOX_CR = 0x61000; //reset FIFO rd/wr pointers, Fifo2MinFL = 16, Fifo1MaxFL = 32
	AUDCR->ACO_CR.BIT.EnableWRFIFO = 1;

	//DMA1: set transfer from buf to FIFO2
	DMA1_TCICR  = 0xFF;			//clear all TC irq
	DMA1_EICR   = 0xFF;			//clear all Err irq
	DMA1_C0CFG  = 0;			//disable C0
	DMA1_C0SADR = (u32)g_buf1 + MEM_TO_DMAC_OFFSET;
	DMA1_C0DADR = (u32)FIFOWR_AHB;
	DMA1_C0CR   = 0x07498000 | (numt&0xFFF);	//s++, M1 to M1, DB=16, transfer = numt

	DMA1_C0CFG  = 0x840;						//mem to perip(DMA ctr, perip req), MEM to FIFO2
	DMA1_C0CFG  |= 1;							//enable

	SLEEP(1);	//transfering the first burst (16)

	//Read FIFO2 (from AHB2DMA)
	//Note: while reading the first 32 data, the DMA writes more data
	FORi(numt) g_buf2[i] = FIFOWR_OUT[0];

	//check
	FORi(numt) CHECK(g_buf2[i] == g_buf1[i]);
}
*/

//Can transfer to FIFO2 any number of data
//WORKING
static void test_MEM_DMA1_FIFO2(void)
{
	const int numt = 66; //max 128 (size of buf1 and buf2)
	int i;

	//MEM
	FORi(128) {g_buf1[i] = i+1; g_buf2[i] = 0;}

	//FIFO2: set DMA write request
	FIFO2_SetDMA1C0TransferFrom(g_buf1, numt, 1);	//enable DMA transfer

	//Read FIFO2 (from AHB2DMA)
	//Note: while reading the first 16 data, the DMA writes more data
	FORi(numt) g_buf2[i] = FIFOWR_OUT[0];

	//check
	FORi(numt) CHECK(g_buf2[i] == g_buf1[i]);
}


//WORKING
//Can transfer any number of data (<128), with different Burst size
/*
static void test_FIFO1_DMA1_MEM(void)
{
	const int numt = 66; //max 128 (size of buf1 and buf2)
	int i, count, tmp;

	//MEM
	FORi(128) {g_buf1[i] = i+1; g_buf2[i] = 0;}

	DMA1_C1CFG  = 0;			//disable C1

	//FIFO1: set DMA read request when reach half fifo (16)
	FIFO_Reset(16, 16);
//	DPMBOX_CR = 0x60000; 		//reset FIFO rd/wr pointers, Fifo2MinFL = 32, Fifo1MaxFL = 32
	DPMBOX_CR = 0x60010; 		//reset FIFO rd/wr pointers, Fifo2MinFL = 32, Fifo1MaxFL = 16
	AUDCR->ACO_CR.BIT.EnableRDFIFO = 1;
//	AUDCR->ACO_CR.BIT.EnableWRFIFO = 1;


	//BUG? As soon as RdFIFO1 and DMA are enabled, the transfer happens even if the FIFO1 as not loaded.
	//HACK: prefetch FIFO1 into tmp
	DMA1_TCICR  = 0xFF;			//clear all TC irq
	DMA1_EICR   = 0xFF;			//clear all Err irq
	DMA1_C1CFG  = 0;			//disable C1
	DMA1_C1SADR = (u32)FIFORD_AHB;
	DMA1_C1DADR = (u32)&tmp + MEM_TO_DMAC_OFFSET;
	DMA1_C1CR   = 0x01484000 | (32);//M1 to M0, SB=32, DB=1,  transfer = 32
	DMA1_C1CFG  = 0x1001;			//perip to mem (DMA ctr, perip req), FIFO1 to MEM
	SLEEP(1);

	//DMA1_C1: set transfers from FIFO1 to buf2
	DMA1_TCICR  = 0xFF;			//clear all TC irq
	DMA1_EICR   = 0xFF;			//clear all Err irq
	DMA1_C1CFG  = 0;			//disable C1
	DMA1_C1SADR = (u32)FIFORD_AHB;
	DMA1_C1DADR = (u32)g_buf2 + MEM_TO_DMAC_OFFSET;
//	DMA1_C1CR   = 0x09484000 | (numt&0xFFF);	//d++, M1 to M0, SB=32, transfer = numt <-- WORKING
	DMA1_C1CR   = 0x09483000 | (numt&0xFFF);	//d++, M1 to M0, SB=16, transfer = numt <-- WORKING
//	DMA1_C1CR   = 0x09480000 | (numt&0xFFF);	//d++, M1 to M0, SB=1,  transfer = numt <--
	DMA1_C1CFG  = 0x1000;						//perip to mem (DMA ctr, perip req), FIFO1 to MEM
	DMA1_C1CFG |= 1;							//enable

	//fill FIFO1 by batch of 20 words
	count = 0;
	while (count < numt) {
		for (i = 0; i < 20 && count < numt; i++)
			FIFORD_IN[0] = g_buf1[count++];
		SLEEP(1);	//wait for the fifo to be read out.
	}

	//flush FIFO1
	//If channel still active (means transfer not complete), write dummy data to trigger last DMA burst.
	if (DMA1_C1CFG & 1) {
		FORi(16) FIFORD_IN[0] = 0;
		SLEEP(1);	//wait for the fifo to be read out.
	}
	DPMBOX_CR = 0x60000; 		//reset FIFO rd/wr pointers

	//check
	FORi(numt) CHECK(g_buf2[i] == g_buf1[i]);
}
*/

//WORKING
//Can transfer any number of data (<128), with different Burst size
static void test_FIFO1_DMA1_MEM(void)
{
	const int numt = 66; //max 128 (size of buf1 and buf2)
	int i, count;

	//MEM
	FORi(128) {g_buf1[i] = i+1; g_buf2[i] = 0;}

	//FIFO1
	FIFO_SetDMA1C1ReadTo(g_buf2, numt, 1);	//enable DMA transfer

	//fill FIFO1 by batch of 20 words
	count = 0;
	while (count < numt) {
		for (i = 0; i < 20 && count < numt; i++)
			FIFORD_IN[0] = g_buf1[count++];
		SLEEP(1);	//wait for the fifo to be read out.
	}

	FIFO1_FlushDMA1C1();

	//check
	FORi(numt) CHECK(g_buf2[i] == g_buf1[i]);
}


//--- test FIFO with DMAC1 and DMABUS in Full Duplex ----------------------------------------------

//WORKING
//Can transfer any number of data
static void test_MEM_DMA1_FIFO2_DMABUS_FIFO1(void)
{
	int i, numt = 66;	//max 128 (size of bufs)

	//MEM
	FORi(128) {g_buf1[i] = i+1; g_buf2[i] = 0;}

	DMA1_C0CFG  = 0;			//disable C0
	DMA1_C1CFG  = 0;			//disable C1

	//DMABUS: set transfers from FIFO2 to FIFO1
	STA_DMAReset();

	//BUG?: when DMABUS directly from FIFO_RX to FIFO_TX, FIFO1 doesnot send DMA request to DMA1.
	//HACK: detour by XIN (the real case in fact...)
//	STA_DMAAddTransfer(STA_DMA_FIFO_OUT, STA_DMA_FIFO_IN, 1);	//from FIFO2 to FIFO1
	DMABUS_TX[0] = STA_DMA_FIFO_OUT << 16 | STA_DMA_DSP0_XIN;	//from FIFO2 to XIN0 TMP
	DMABUS_TX[1] = STA_DMA_DSP0_XIN << 16 | STA_DMA_FIFO_IN;	//from XIN0 to FIFO1

	//HACK to self-stop the DMABUS after completed the previous transfers by writing to DMABUS_CR
//	AIF->LPF0_DI[0].REG = 0; //value to be transfered to DMABUS_CR by itself (e.g. to self-disable the DMABUS)
//	DMABUS_TX[numt] = STA_DMA_LPF0_DI << 16 | 0x1000;	//add as last dma transfer


	//FIFOs:
	FIFO_SetDMA1C1ReadTo(g_buf2, numt, 1);				//enable DMA C1 transfer
	FIFO2_SetDMA1C0TransferFrom(g_buf1, numt, 1);		//enable DMA C0 transfer

	//start DMABUS
	STA_DMAStart();

	SLEEP(20);	//transfering at 48kHz...

//	FORi(numt) FIFORD_IN[0] = g_buf1[i];	//TMP for debug

	//stop
	STA_DMAReset();
	FIFO1_FlushDMA1C1();

	//check
	FORi(numt) CHECK(g_buf2[i] == g_buf1[i]);

}

//TMP
//WORKING (but not very useful)
static void test_FIFO1_DMA1_FIFO2_from_yuan(void)
{
	int i, numt = 32;

	//MEM
	FORi(128) {g_buf1[i] = 0; g_buf2[i] = 0;}

	DMA1_C0CFG  = 0;			//disable C0

	FIFO_Reset(16, 16);
	FIFOS_CR = 0x60000; 		//reset FIFO rd/wr pointers, Fifo2MinFL = 32, Fifo1MaxFL = 32
//	AUDCR->ACO_CR.BIT.EnableRDFIFO = 1;
	AUDCR->ACO_CR.BIT.EnableWRFIFO = 1;

	FORi(numt) FIFORD_IN[0] = i+1;

	//DMA1: transfer from FIFO1 to FIFO2
	DMA1_C0CFG  = 0;			//disable C0
	DMA1_TCICR  = 0xFF;			//clear all TC irq
	DMA1_EICR   = 0xFF;			//clear all Err irq
	DMA1_C0SADR = (u32)FIFORD_AHB;
	DMA1_C0DADR = (u32)FIFOWR_AHB;
//	DMA1_C0CR   = 0x0349C000|(numt&0xFFF); 	//M1 to M1, SB=32, DB=16, transfer = numt (DMA ctr)  <--- WORKING
	DMA1_C0CR   = 0x03480000|(numt&0xFFF); 	//M1 to M1, SB=1,  DB=1,  transfer = numt (DMA ctr)  <--- WORKING
	DMA1_C0CFG  = 0x840;		//mem to perip(DMA ctr, perip req), FIFO1 to FIFO2(req)
	DMA1_C0CFG |= 1;			//enable

	SLEEP(1);

	//Read FIFO2 (from AHB2DMA)
	FORi(numt) g_buf2[i] = FIFOWR_OUT[0];

	//check
	FORi(numt) CHECK(g_buf2[i] == i+1);
}

//---------------------------------------------------------
// Tests FIFO1 (more... when tested ADCMic)
//---------------------------------------------------------

static void test_FIFO1_DMA1_MEM(void)
{
	const int numt = 66; //max 128 (size of buf1)
	int i, count;

	//reset buses (from upward)
	DMA1_C1CFG  = 0;			//disable DMA1 C1 (FIFOs only works with DMA1)
	STA_DMAStop();				//reset DMABus

	//MEM
	//set input data
	FORi(128) {g_buf1[i] = i+1; g_buf1[i] = 0;}

	//FIFO1
	FIFO_Reset(16, 16);			//reset FIFO
	FIFO_EnableDMARead(TRUE);

	//DMA1C1
	//read from FIFO1 to buf
	DMA_SetTransfer(DMA1, 1, (u32*)FIFORD_AHB, (u32*)g_buf1,
					DMA_DI | DMA_P2M | DMA_SBIT_32 | DMA_DBIT_32 | DMA_SBUSRT_1, //CR
					DMA_P2M_DMACTL | DMA_SREQ(DMA1_DSPFIFO1)| DMA_ENABLE, numt); //CFG


	//fill FIFO1 by batch of 20 words
	count = 0;
	while (count < numt) {
		for (i = 0; i < 20 && count < numt; i++)
			FIFORD_IN[0] = g_buf1[count++];
		SLEEP(1);	//wait for the fifo to be read out.
	}

	FIFO_FlushDMARead(1);
	SLEEP(1);
	FIFO_EnableDMARead(FALSE);

	//check
	FORi(numt) CHECK(g_buf1[i] == g_buf1[i]);
}

//						  LR		only L channel
//DMA BUF -> DMA0 -> MSP2 -> SAI2 -> DMABUS -> FIFO1 -> DMA1 -> DST_BUF
static void test_MSP2_SAI2_DMABUS_FIFO1()
{
	u32* dmabuf = (u32*)DMA_BUF; //src
	u32* dstbuf = (u32*)DST_BUF; //dst

	int numSamples = 200; //16 bit
	int numLoaded  = 0;
	int i, offset;

	//reset buses and masters
	DMA0_C0CFG  = 0;			//stop DMA0 C0
	DMA1_C1CFG  = 0;			//stop DMA1 C1 (FIFOs only works with DMA1)
	STA_DMAStop();				//reset DMABus

	//DMA BUF (src)
//	memset(DMA_BUF, 0, DMA_BUFSIZE);
	FORi(4096) dmabuf[i] = ((i+1) & 0xFFFF) << 16 | ((i+1) & 0xFFFF); //LLLLRRRR 16bits

	//DST BUF
	memset((void*)dstbuf, 0, numSamples * 2); //16 bits

	//SAI2 (slave, enable)
	AIF->SAI2_CR.REG = SAI_16_16VAL | SAI_SLV | SAI_EN;

	//MSP2 (master) -> SAI2
	MSP_Reset(MSP2);
	MSP_ConfigSampleRateGen(MSP2, 52000000, 48000, 32, 16);	//2x16 bit
	MSP2->MSP_GCR.REG = MSP_SCK_52MHZ | MSP_TFS_OUT | MSP_TCK_OUT | MSP_TCKPOL_DW | MSP_TFSPOL_L | MSP_TFEN; //master i2s
	MSP2->MSP_TCF.REG = MSP_P1_32BIT | MSP_P1LEN(1) | MSP_FSIG;	//1x32 bits
	MSP2->MSP_DMACR.REG = MSP_TDMAEN;

	//DMABUS: SAI2 -> FIFO1
	STA_DMAReset();
	STA_DMAAddTransfer(STA_DMA_SAI2_RX1,	STA_DMA_FIFO_IN, 1);	//SAI2 to FIFO1 (1 ch)

	//FIFO1
	FIFO_Reset(16, 16);			//reset FIFO
	FIFO_EnableDMARead(TRUE);

	//START
	STA_DMAStart();				//start DMABUS
	MSP_EnableTx(MSP2);			//start MSP2 (master)


//	while (numLoaded < numSamples)
	{
		int numDMAt = MIN(500, (numSamples-numLoaded)/2); //LR 16 bits (dma transfer in words)

		//DMA1_C1: FIFO1 -> DST_BUF (only the L channel)
//		while (DMA1_C1CFG & 1); //wait for the previous transfer to be completed...
		DMA_SetTransfer(DMA1, 1, (u32*)FIFORD_AHB, &dstbuf[numLoaded/2],
					DMA_DI | DMA_P2M | DMA_SBIT_32 | DMA_DBIT_32 | DMA_SBUSRT_1, //CR
					DMA_P2M_DMACTL | DMA_SREQ(DMA1_DSPFIFO1)| DMA_ENABLE, numDMAt); //CFG

		//DMA0_C0: DMA_BUF -> MSP2
		while (DMA0_C0CFG & 1);	//wait for the previous transfer to be completed...
		DMA_SetTransfer(DMA0, 0, dmabuf, (u32*)&MSP2_DR, DMA_SI | DMA_M2P | DMA_SBIT_32 | DMA_DBIT_32 | DMA_DBUSRT_1, DMA0_CFG_MEM2MSP2 | DMA_ENABLE, numDMAt);

		SLEEP(numDMAt/48); 		//avoid sticky loop (4095/48000 = 85 ms)
		while (DMA0_C0CFG & 1);	//wait for the previous transfer to be completed...

		//loaded samples
		numLoaded += numDMAt*2;
	}

	FIFO_FlushDMARead(1);
	SLEEP(1);
	FIFO_EnableDMARead(FALSE);

	//STOP
	DMA0_C0CFG  = 0;			//stop DMA0 C0
	DMA1_C1CFG  = 0;			//stop DMA1 C1 (FIFOs only works with DMA1)
	STA_DMAStop();				//reset DMABus
	MSP_DisableTx(MSP2);

	//check

	//find the first sample in dst buf
	//SRC = LLLLRRRR vs DST = LLLL00 (24 bits left aligned)
	offset = 0;
	while ((dstbuf[offset] >> 8) != ((dmabuf[0] >> 16)&0xFFFF) && (offset < numSamples / 2))
		offset++;

	for (i = 0; i < numSamples / 2;  i++)
		CHECK((dstbuf[offset + i] >> 8) == ((dmabuf[i] >> 16)&0xFFFF) );
}

//---------------------------------------------------------

void TestFIFO(void)
{
	PRINTF("TestFIFO... ");

//	test_DMABUS();
//	test_DMA1_mem_to_mem();				//ok
//	test_FIFOs_simple();				//ok
//	test_FIFO_overflow();				//tmp
//	test_FIFO_resetPtr();				//tmp
//	test_FIFO2_DMABUS_FIFO1();			//ok

//	test_MEM_DMA1_FIFO2();				//ok
//	test_FIFO1_DMA1_MEM();				//ok
	test_MEM_DMA1_FIFO2_DMABUS_FIFO1();	//ok

//	test_FIFO1_DMA1_FIFO2_from_yuan();	//ok, tmp

//	SLEEP(5); 		//wait 5ms for DSP initialisation
}

#endif //0

