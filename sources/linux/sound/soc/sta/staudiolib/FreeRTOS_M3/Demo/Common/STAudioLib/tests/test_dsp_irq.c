/*
 * STAudioLib - test_dsp_irq.c
 *
 * Created on 2015/06/16 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	Testing
	. DSP2ARM IRQ
	. ARM2DSP IRQ
	. FSYNC2DSP IRQ
	. FSYNC2ARM IRQ


	ARM2DSP IRQ(line1):
	------------------
	DSP side:
	1. register isr: void _INTERRUPT arm_irq_1()
	2. register isr in interrupts.txt (EDE): #pragma interrupt(arm_irq_1, 1)
	3. enable global interrupt:   sr_mode(INTERRUPT_EN,0);
	4. enable specific interrupt: sr_irq(IRQ_ARM1_EN, 0);
	5. set IRQ_ARM2DSP_EN = IRQ_ARM1_EN

	ARM side:
	1. trigger: AUDCR->ACO_EIR0.BIT.ARM2DSP0_INT1 = 0;
	  (irq cleared automatically in hw)


	DSP2ARM IRQ(single line):
	--------------------------
	ARM side:
	1. set NVIC (M3) (done in STA_Init())
	2. register DSP ISR -> STA_SetDspISR()

	DSP side:
	1. trigger IRQ_DSP2ARM_SET = 1;
	2. clear irq (done by STAudio)


	FSYNC2DSP IRQ(line4):
	--------------------
	DSP side:
	1. register isr: void _INTERRUPT fsync_irq()
	2. register isr in interrupts.txt (EDE): #pragma interrupt(fsync_irq, 4)
	3. enable global interrupt:   sr_mode(INTERRUPT_EN,0);
	4. enable specific interrupt: sr_irq(IRQ_FSYNC_EN, 0);
	5. set IRQ_ARM2DSP_EN = IRQ_FSYNC_EN

	ARM side:
	1. program DMABUS
	(irq cleared automatically in hw)


	FSYNC2ARM IRQ
	-------------
	ARM side:
	1. set NVIC (M3) (done in STA_Init())
	2. register FSYNC ISR -> STA_SetFSyncISR()
	3. enable irq -> AUDCR->ACO_IMSC.BIT.FSYNC_IMSC = 1 (done in STAudioLib)
	4. program DMABUS
	5. clear irq: AUDCR->ACO_ICR.BIT.FSYNC_ICR = 1; (done in STAudioLib)

 */

#if 0
#include "staudio_test.h"


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

//---------
// MODULES
//---------
//for user interaction

//---------------------------------------------------------
// ISRs
//---------------------------------------------------------

static void dspISR(STA_Dsp dsp)
{
	PRINTF("dspISR(%d)\n", dsp);
}

static void FSyncISR(void)
{
	PRINTF("FSyncISR()\n");
}


//---------------------------------------------------------
// BUILD FLOW
//---------------------------------------------------------

static void initDSP(void)
{
	int i;

	STA_Reset();

	//register DSP ISR
	STA_SetDspISR(DSP_SEL, dspISR);

	//register and enable Fs irq
//	STA_SetFSyncISR(FSyncISR);

	//MODULE

	//CONNECTIONS
	//(just a pass-thru connection...)
	FORi(2) {
        STA_Connect(MOD_XIN, i,  MOD_XOUT,   i);
    }

	//BUILD
	STA_BuildFlow();
	ASSERT(STA_GetError() == STA_NO_ERROR);

	//START dsp
	//STA_Play(); //start dsps with at least one connection
	STA_StartDSP(DSP_SEL);
	SLEEP(100);					//wait 100ms for DSP initialisation

	//post initialisation
}


static void initDMABUS()
{
	int i;

	//DMABUS
	STA_DMAReset();

	//DSP input
	STA_DMAAddTransfer(STA_DMA_SAI2_RX1,	DMA_XIN, 2);			//SAI2 to XIN

	//DSP output
	STA_DMAAddTransfer(DMA_XOUT,  			STA_DMA_SAI3_TX1,  2);	//XOUT to SAI3

	//DSP swap trigger
	STA_DMAAddTransfer(STA_DMA_SAI2_RX1, 	DMA_XIN+127, 1);		//swap trigger
}


//---------------------------------------------------------
// KEY / ROTARY
//---------------------------------------------------------

static void rtxCallback(int steps)
{
}

static void keyCallback(int key, bool down)
{
}


//---------------------------------------------------------
// Main
//---------------------------------------------------------

void TestDspIRQ(void)
{
	PRINTF("TestDspIRQ... \n");

	//user control
	RTX_Init(rtxCallback);
	KPD_Init(keyCallback);

	//DSP / DMABUS
	initDSP();					//start DSP
	initDMABUS();


	//----- play (in loop)-------

	//SAI3 (master, enable)
	AIF->SAI3_CR.REG = SAI_I2S | SAI_MST | SAI_32_16VAL | SAI_RL | SAI_EN;

	//DMABUS
	STA_DMAStart();				//start DMABUS


	while (1) {

		SLEEP(10);	//avoid sticky loop
	}

	//stop and clean
	STA_Reset();
	STA_SetDspISR(DSP_SEL, 0);
	STA_SetFSyncISR(NULL);
//	PRINT_OK_FAILED;
}

#endif //0
