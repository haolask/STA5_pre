/*
 * STAudioLib - test_eq_response.c
 *
 * Created on 2016/11/22 by Olivier Douvenot
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	testing:

	- USE CASE 1: analyse EQ impulse response

*/

#if 1
#include "staudio_test.h"


#define DSP_LATENCY             5
#define NUM_DSP_IN_SAMPLES		10
#define NUM_DSP_OUT_SAMPLES		(NUM_DSP_IN_SAMPLES+DSP_LATENCY)


static q23 g_input_samples[NUM_DSP_IN_SAMPLES];
static q23 g_output_samples[NUM_DSP_OUT_SAMPLES];

static int g_running = 0;

//---------------------------------------------------------
// DMA1 for play/dump thru DSP FIFOs
//---------------------------------------------------------

//callback
static void dma1_isr(DMA_Typedef *dma, u32 ch)
{
	g_running = 0; // stop the infinite loop
}

static void dma1_set()
{
	//DMA1C2: in buffer -> FIFO2
	DMA_SetTransfer(DMA1, 2, (u32*)g_input_samples, (u32*)FIFO2_IN,
					DMA_SI | DMA_M2P | DMA_SBIT_32 | DMA_DBIT_32 | DMA_SBUSRT_1 | DMA_DBUSRT_1, // | DMA_TCIEN, 	//CR
					DMA_M2P_DMACTL | DMA_DREQ(DMA1_DSPFIFO2)| DMA_ENABLE, // | DMA_ITC,				//CFG
					NUM_DSP_IN_SAMPLES); 	//numTransfer

	//DMA1C1: FIFO1 -> out buffer
	DMA_SetTransfer(DMA1, 1, (u32*)FIFO1_OUT, (u32*)g_output_samples,
					DMA_DI | DMA_P2M | DMA_SBIT_32 | DMA_DBIT_32 | DMA_SBUSRT_1 | DMA_DBUSRT_1 | DMA_TCIEN, //CR
					DMA_P2M_DMACTL | DMA_SREQ(DMA1_DSPFIFO1)| DMA_ITC | DMA_ENABLE,				//CFG
					NUM_DSP_OUT_SAMPLES); 	//numTransfer
}

//---------------------------------------------------------
// Test no DSP
//---------------------------------------------------------
static void test_dump_noDSP()
{
	STA_Reset();	//stop DMABUS

	//DMABUS: FIFO2 -> FIFO1 (1ch)
	STA_DMAReset();
	STA_DMAAddTransfer(STA_DMA_FIFO_OUT,	STA_DMA_FIFO_IN, 1);

	//DMA
	dma1_set();

	//Start
	g_running = 1;
	STA_DMAStart();
}

//---------------------------------------------------------
// Test GAIN + EQ
//---------------------------------------------------------
static void test_EQ(void)
{
	STAModule gain, EQ;
	int i;

	//------ DSP INIT ------------

	STA_Reset();	//(also stops the DMABUS)

	//GAIN
	gain = STA_AddModule(STA_DSP0, STA_GAIN_STATIC_NCH);  //(2 gains by default)

	//EQ
	EQ = STA_AddModule(STA_DSP0, STA_EQUA_STATIC_3BANDS_SP);  //(2 ch by default)

	//CONNECTIONS
	FORi(2) {
		STA_Connect(STA_XIN0, i,  gain,      i);
		STA_Connect(gain,     i,  EQ,        i);
		STA_Connect(EQ,       i,  STA_XOUT0, i);
	}

	//BUILD
	STA_BuildFlow();
	ASSERT(STA_GetError() == STA_NO_ERROR);

	//DSP START
//	STA_Play();					//also starts the DMABUS...
	STA_StartDSP(STA_DSP0);
	SLEEP(10);					//wait 10ms for DSP initialisation

	//POST-INIT

	//Source Gain
	STA_GainSetGains(gain, 0xF, 0);		//0 dB

	//EQ
	STA_SetMode(EQ, STA_EQ_ON);
	STA_SetFilterType(EQ, 0, STA_BIQUAD_BYPASS);
	STA_SetFilterType(EQ, 1, STA_BIQUAD_BYPASS);
	STA_SetFilterType(EQ, 2, STA_BIQUAD_BYPASS);


	//------ DMABUS ---------------

	//DSP in
	STA_DMAAddTransfer(STA_DMA_FIFO_OUT,	STA_DMA_DSP0_XIN, 1);

	//DSP out
	STA_DMAAddTransfer(STA_DMA_DSP0_XOUT,	STA_DMA_FIFO_IN, 1);

	//DSP port swap
	STA_DMAAddTransfer(STA_DMA_SAI2_RX1, 	STA_DMA_DSP0_XIN+127, 1);		//swap trigger

	//------ DMA1 -----------------

	dma1_set();

	//------ Start ----------------

	g_running = 1;
	STA_DMAStart();
}

/*
void test_EQ()
{
	STAModule eq_fl;
	STA_FilterType type = STA_BIQUAD_LOWP_BUTTW_1;
	s32 G = 3*STA_BIQ_GAIN_SCALE; // dB*STA_BIQ_GAIN_SCALE
	s32 Q = 10;
	s32 F = 1000; // Hz


	//TODO


	//eq_fl = STA_AddModule(DSP_SEL, STA_EQUA_STATIC_7BANDS_SP);
	//eq_fl = STA_SetNumChannels(eq_fl, 1);
	STA_Connect(STA_XIN0, 0, STA_XOUT0, 0);
	//STA_Connect(eq_fl, 0, MOD_XOUT, 0);
	//STA_SetFilterType(eq_fl, 0, type);

	//STA_SetFilter(eq_fl, 0, G, Q, F);
	//STA_SetMode(eq_fl, STA_EQ_OFF);

	// build flow
	STA_BuildFlow();
	ASSERT(STA_GetError() == STA_NO_ERROR);
}
*/

//---------------------------------------------------------
// KEY / ROTARY
//---------------------------------------------------------

static void keyCallback(int key, bool down)
{
}

static void rtxCallback(int steps)
{
}

//---------------------------------------------------------
// Main
//---------------------------------------------------------

void Test_EQ_response(void)
{
	int i;

	PRINTF("Test_EQ_response... \n");

	//---- some inits ------------

	//user control
	//RTX_Init(rtxCallback);
	//KPD_Init(keyCallback);

	//DMA1C1/C2
	DMA1_C1CFG  = 0;			//stop DMA1_C1 (FIFOs only works with DMA1)
	DMA1_C2CFG  = 0;			//stop DMA1_C2 (FIFOs only works with DMA1)
	DMA_SetTCIRQHandler(DMA1, 1, dma1_isr);
	#ifdef CORTEX_M3
	NVIC_EnableIRQChannel(EXT9_IRQChannel,  0, 0);	//enable DMA1 IRQ to M3
	#endif

	//FIFOs
	FIFO_Reset(16, 16);			//reset FIFO
	FIFO_SetDMAReadRequest(TRUE);
	FIFO_SetDMAWriteRequest(TRUE);


	//---- inputs ------------

	//input
	// prepare impulse input signal
    for(i = 0; i < NUM_DSP_IN_SAMPLES; i++) {
        g_input_samples[i] = i+2;
    }
    for(i = 0; i < NUM_DSP_OUT_SAMPLES; i++) {
        g_output_samples[i] = 0xBAD;
    }

	//---- test cases ------------

//	test_dump_noDSP();
	test_EQ();

	//---- run ------------------

	while (g_running)
	{

		SLEEP(100);	//avoid sticky loop
	}

	//FIFO_FlushDMARead(1);

	//---- post processing ----

	PRINTF("test finished.\n");
    for(i = 0; i < NUM_DSP_IN_SAMPLES; i++) {
        PRINTF("in[%d] = %d\n", i, g_input_samples[i]);
    }
    for(i = 0; i < NUM_DSP_OUT_SAMPLES; i++) {
        PRINTF("out[%d] = %d\n", i, g_output_samples[i]);
    }

	//stop and clean
	STA_Reset();
}

#endif //0
