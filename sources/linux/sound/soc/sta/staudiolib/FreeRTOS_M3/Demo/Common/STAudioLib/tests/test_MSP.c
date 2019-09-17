/*
 * STAudioLib - test_MSP.c
 *
 * Created on 2013/10/23 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	Testing

	MSP2 -> SAI2

 */

#if 0
#include "staudio_test.h"

#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

static s32 *g_buf_L, *g_buf_R;

#if 0
/** Default MSP_configuration for I2S protocol */
MSP_ConfigTy msp_basecfg_i2s = {

  MSP_CLOCK_SEL_EXT,                    /* srg_clock_sel                 */
  MSP_CLOCK_POL_RISE,                   /* sck_pol                       */
  MSP_LOOPBACK_MODE_DISABLE,            /* msp_loopback_mode             */
  MSP_DIRECT_COMPANDING_MODE_DISABLE,   /* msp_direct_companding_mode    */
  MSP_CLOCK_SEL_INT,                    /* rx_clock_sel                  */
  MSP_CLOCK_SEL_INT,                    /* tx_clock_sel                  */
  MSP_MODE_DMA_OFF,                      /* rx_msp_dma_mode               */
  MSP_MODE_DMA_OFF,                      /* tx_msp_dma_mode               */
  MSP_FRAME_SEL_RX_INTERNAL,            /* rx_frame_sync_sel             */
  MSP_FRAME_SEL_GEN_LOGIC_PERIOD,       /* tx_frame_sync_sel             */
  MSP_UNEXPEC_FRAME_SYNC_IGNORED,       /* rx_unexpect_frame_sync        */
  MSP_UNEXPEC_FRAME_SYNC_IGNORED,       /* tx_unexpect_frame_sync        */
  MSP_FIFO_ENABLE,                     /* rx_fifo_config                */
  MSP_FIFO_ENABLE,                     /* tx_fifo_config                */
  MSP_TX_EXTRA_DELAY_OFF                /* tx_extra_delay                */
};

MSP_ProtocolTy msp_baseprotocol_i2s = {

  MSP_DATA_TRANSFER_WIDTH_HALFWORD,   /* rx_data_transfer_width        */
  MSP_DATA_TRANSFER_WIDTH_HALFWORD,   /* tx_data_transfer_width        */
  MSP_PHASE_MODE_SINGLE,              /* rx_phase_mode                 */
  MSP_PHASE_MODE_SINGLE,              /* tx_phase_mode                 */
  MSP_2PH_IMMEDIATE,                  /* rx_phase2_start_mode          */
  MSP_2PH_IMMEDIATE,                  /* tx_phase2_start_mode          */
  MSP_MSB_FIRST,                      /* rx_endianess                  */
  MSP_MSB_FIRST,                      /* tx_endianess                  */
  2,                                  /* rx_frame_length_1             */
  1,                                  /* rx_frame_length_2             */
  2,                                  /* tx_frame_length_1             */
  1,                                  /* tx_frame_length_2             */
  MSP_32_BIT,                         /* rx_element_length_1           */
  MSP_16_BIT,                         /* rx_element_length_2           */
  MSP_32_BIT,                         /* tx_element_length_1           */
  MSP_16_BIT,                         /* tx_element_length_2           */
  MSP_1_CLOCK_DELAY,                  /* rx_data_delay                 */
  MSP_1_CLOCK_DELAY,                  /* tx_data_delay                 */
  MSP_CLOCK_POL_RISE,                 /* rx_clock_pol                  */
  MSP_CLOCK_POL_FALL,                 /* tx_clock_pol                  */
  MSP_FRAME_SYNC_POL_HIGH,            /* rx_msp_frame_pol              */
  MSP_FRAME_SYNC_POL_HIGH,            /* tx_msp_frame_pol              */
  MSP_NO_SWAP,                        /* rx_half_word_swap             */
  MSP_EACH_HALFWORD_SWAP,             /* tx_half_word_swap             */
  MSP_NO_COMPANDING,                  /* compression_mode              */
  MSP_NO_COMPANDING,                  /* expansion_mode                */
  MSP_NO_SPICLOCK,                    /* spi_clk_mode                  */
  MSP_SPI_BURST_MODE_DISABLE,         /* spi_burst_mode                */
  31,                                 /* frame_period                  */
  15,                                 /* frame_width                   */
  32,                                 /* total_clocks_for_one_frame    */
};
#endif

/*
void InitMSP(MSP_TypeDef *MSP_port)
{
	MSP_DisableTxRx(MSP_port);
	MSP_EmptyTxFifo(MSP_port);
	MSP_EmptyRxFifo(MSP_port);
	MSP_ResetReg(MSP_port);

//	MSP_Configure(MSP_port, &msp_basecfg_i2s, &msp_baseprotocol_i2s);
}
*/

//---------------------------------------------------------
// test MSP2 2x16 bit
//---------------------------------------------------------
//WORKING
static void Test_MSP2_2x16(void)
{
	u16 data[2] = {0x1234, 0x5678}; //34 12 78 56 -> 0x56781234

	//MSP2
	MSP_Reset(MSP2);
	MSP_ConfigSampleRateGen(MSP2, 52000000, 48000, 32, 16);	//LLLLRRRR
	MSP2->MSP_GCR.REG = MSP_SCK_52MHZ | MSP_TFS_OUT | MSP_TCK_OUT | MSP_TCKPOL_DW | MSP_TFSPOL_H /*MSP_TFEN*/;	//master i2s
	MSP2->MSP_TCF.REG = MSP_P1_32BIT | MSP_P1LEN(1) | MSP_FSIG;	//1x32 bits
//	MSP2->MSP_TCF.REG = MSP_P1_16BIT | MSP_P1LEN(2) | MSP_FSIG;	//1x32 bits

	//SAI2 (enable slave)
	AIF->SAI2_CR.REG = SAI_16_16VAL | SAI_EN;

	//start master
	MSP_EnableTx(MSP2);

	while (1){

		u32 L = 0, R = 0;

		//MSP2->MSP_DR = 0x12345678;
		MSP2->MSP_DR = 0x56781234;
		//MSP2->MSP_DR = (u32) *((u32*)data);

		L = AIF->SAI2_RX1[0];
		R = AIF->SAI2_RX1[1];

		SLEEP(100); //<--- put breakpoint here
	}

}

static void Test_MSP2_1x16(void)
{
	int i;

	g_buf_L = g_buf;
	g_buf_R = g_buf + 10;
	FORi(100) g_buf[i] = 0;

	//MSP2
	//note: use MSP_TFSPOL_H to swap LR
	MSP_Reset(MSP2);
	MSP_ConfigSampleRateGen(MSP2, 52000000, 48000, 32, 16);	//LLLL0000
	MSP2->MSP_GCR.REG = MSP_SCK_52MHZ | MSP_TFS_OUT | MSP_TCK_OUT | MSP_TCKPOL_DW | MSP_TFSPOL_L | MSP_TFEN;	//master i2s
	MSP2->MSP_TCF.REG = MSP_P1_16BIT | MSP_P1LEN(1) | MSP_FSIG;	//1x32 bits

	//SAI2 (enable slave)
	AIF->SAI2_CR.REG = SAI_16_16VAL | SAI_EN;

	MSP2->MSP_DR = 0xABC1;
	MSP2->MSP_DR = 0xABC2;
	MSP2->MSP_DR = 0xABC3;
	MSP2->MSP_DR = 0xABC4;
	MSP2->MSP_DR = 0xABC5;
	MSP2->MSP_DR = 0xABC6;
	MSP2->MSP_DR = 0xABC7;
	MSP2->MSP_DR = 0xABC8;

	MSP_EnableTx(MSP2);

	while (AIF->SAI2_RX1[0] == 0 && AIF->SAI2_RX1[1] == 0) ;

	#if 0
	//take the last from previous run
	g_buf_L[0] = AIF->SAI2_RX1[0]; //L
	while (g_buf_L[0] == AIF->SAI2_RX1[0]) ;
	#endif

	FORi(8/1){
		g_buf_L[i] = AIF->SAI2_RX1[0]; //L
		g_buf_R[i] = AIF->SAI2_RX1[1]; //R
		while (g_buf_L[i] == AIF->SAI2_RX1[0] && g_buf_R[i] == AIF->SAI2_RX1[1]) ;
	}


	MSP_DisableTx(MSP2);

}

//---------------------------------------------------------
// test MSP2 -> SAI2 32 (24 valid)
//---------------------------------------------------------
//note: SAIDATA does not support full 32bit
//OK
static void test_MSP2_32_24valid(int nch)
{
	int i;

	g_buf_L = g_buf;
	g_buf_R = g_buf + 10;
	FORi(100) g_buf[i] = 0;

	//MSP2 (master i2s)
	MSP_Reset(MSP2);
#if 0
	//sending 32 bit (24valid)
	MSP_ConfigSampleRateGen(MSP2, 52000000, 48000, 64, 32);	//00LLLLLL_00000000 bit
	MSP2->MSP_GCR.REG = MSP_SCK_52MHZ | MSP_TFS_OUT | MSP_TCK_OUT | MSP_TCKPOL_DW | MSP_TFSPOL_L | MSP_TFEN; //master i2s
	MSP2->MSP_TCF.REG = MSP_P1_32BIT | MSP_P1LEN(nch) | MSP_LSBIT;// | MSP_FSIG;	//1x32 bits

	//SAI2 (slave, enable)
	AIF->SAI2_CR.REG  = SAI_32_24VAL | SAI_EN | SAI_DIR_LSB ;
#else
	//sending 24 bit (24valid)
	//note: use MSP_TFSPOL_H to swap LR
	MSP_ConfigSampleRateGen(MSP2, 52000000, 48000, 48, 24);	//
	MSP2->MSP_GCR.REG = MSP_SCK_52MHZ | MSP_TFS_OUT | MSP_TCK_OUT | MSP_TCKPOL_DW | MSP_TFSPOL_H | MSP_TFEN; //master i2s
	MSP2->MSP_TCF.REG = MSP_P1_24BIT | MSP_P1LEN(nch);// | MSP_LSBIT;// | MSP_FSIG;	//1x32 bits

	//SAI2 (slave, enable)
	AIF->SAI2_CR.REG = SAI_24_24VAL | SAI_EN;// | SAI_DIR_LSB;
#endif


	FORi(8) MSP2->MSP_DR = 0xAABBCCD0 | i;

/*	MSP2->MSP_DR = 0xAABBCCD1;
	MSP2->MSP_DR = 0xAABBCCD2;
	MSP2->MSP_DR = 0xAABBCCD3;
	MSP2->MSP_DR = 0xAABBCCD4;
	MSP2->MSP_DR = 0xAABBCCD5;
	MSP2->MSP_DR = 0xAABBCCD6;
	MSP2->MSP_DR = 0xAABBCCD7;
	MSP2->MSP_DR = 0xAABBCCD8;
*/
	MSP_EnableTx(MSP2);

	while (AIF->SAI2_RX1[0] == 0 && AIF->SAI2_RX1[1] == 0) ;

	#if 0
	//take the last from previous run
	g_buf_L[0] = AIF->SAI2_RX1[0]; //L
	while (g_buf_L[0] == AIF->SAI2_RX1[0]) ;
	#endif

	FORi(8/nch){
		g_buf_L[i] = AIF->SAI2_RX1[0]; //L
		g_buf_R[i] = AIF->SAI2_RX1[1]; //R
		while (g_buf_L[i] == AIF->SAI2_RX1[0] && g_buf_R[i] == AIF->SAI2_RX1[1]) ;
	}


	MSP_DisableTx(MSP2);
}

//---------------------------------------------------------
// test MSP2 -> SAI2 24 bits
//---------------------------------------------------------
//note: can also be used for 32_24valid
/*
static void test_MSP2_24(int nch)
{
	int i;

	g_buf_L = g_buf;
	g_buf_R = g_buf + 10;
	FORi(100) g_buf[i] = 0;

	//MSP2 (master i2s)
	MSP_Reset(MSP2);
	MSP_ConfigSampleRateGen(MSP2, 52000000, 48000, 48, 24);	//
	MSP2->MSP_GCR.REG = MSP_SCK_52MHZ | MSP_TFS_OUT | MSP_TCK_OUT | MSP_TCKPOL_DW | MSP_TFSPOL_H | MSP_TFEN; //master i2s
	MSP2->MSP_TCF.REG = MSP_P1_32BIT | MSP_P1LEN(3);// | MSP_LSBIT;// | MSP_FSIG;	//4x24 = 3x32 bits

	//SAI2 (slave, enable)
	AIF->SAI2_CR.REG = SAI_24_24VAL | SAI_EN;// | SAI_DIR_LSB;


//	FORi(8) MSP2->MSP_DR = 0xAABBCCD0 | i;
	MSP2->MSP_DR = 0x00112233;
	MSP2->MSP_DR = 0x44556677;
	MSP2->MSP_DR = 0x8899AABB;
	MSP2->MSP_DR = 0xCCDDEEFF;
	MSP2->MSP_DR = 0x00112233;
	MSP2->MSP_DR = 0x44556677;
	MSP2->MSP_DR = 0x8899AABB;
	MSP2->MSP_DR = 0xCCDDEEFF;

	MSP_EnableTx(MSP2);

	while (AIF->SAI2_RX1[0] == 0 && AIF->SAI2_RX1[1] == 0) ;

	#if 0
	//take the last from previous run
	g_buf_L[0] = AIF->SAI2_RX1[0]; //L
	while (g_buf_L[0] == AIF->SAI2_RX1[0]) ;
	#endif

	FORi(4){
		g_buf_L[i] = AIF->SAI2_RX1[0]; //L
		g_buf_R[i] = AIF->SAI2_RX1[1]; //R
		while (g_buf_L[i] == AIF->SAI2_RX1[0] && g_buf_R[i] == AIF->SAI2_RX1[1]) ;
	}


	MSP_DisableTx(MSP2);
}
*/


//---------------------------------------------------------
// test MSP2 -> SAI2 8bit
//---------------------------------------------------------

/*
note: can't find the working setting. The best is

MSP2->MSP_DR = 0x2211; -> SAI2_RX1L = 0x110000   SAI2_RX1R = 0
MSP2->MSP_DR = 0x4433; -> SAI2_RX1L = 0x330000   SAI2_RX1R = 0x2200
MSP2->MSP_DR = 0x2211; -> SAI2_RX1L = 0x110000   SAI2_RX1R = 0x4400
MSP2->MSP_DR = 0x4433; -> SAI2_RX1L = 0x330000   SAI2_RX1R = 0x2200
*/
static void test_MSP2_8(int nch)
{
	int i;

	g_buf_L = g_buf;
	g_buf_R = g_buf + 10;
	FORi(200) g_buf[i] = 0;

	//MSP2:
	MSP_Reset(MSP2);
	MSP_ConfigSampleRateGen(MSP2, 52000000, 48000, 32, 8);	//
	MSP2->MSP_GCR.REG = MSP_SCK_52MHZ | MSP_TFS_OUT | MSP_TCK_OUT | MSP_TCKPOL_DW | MSP_TFSPOL_H | MSP_TFEN; //master i2s
	MSP2->MSP_TCF.REG = MSP_P1_16BIT | MSP_P1LEN(1);// | MSP_LSBIT;// | MSP_FSIG;	//1x32 bits

	//SAI2 (slave, enable)
	AIF->SAI2_CR.REG = SAI_16_16VAL | SAI_EN;// | SAI_DIR_LSB;


	MSP2->MSP_DR = 0x2211;
	MSP2->MSP_DR = 0x4433;
	MSP2->MSP_DR = 0x2211;
	MSP2->MSP_DR = 0x4433;
	MSP2->MSP_DR = 0x2211;
	MSP2->MSP_DR = 0x4433;
	MSP2->MSP_DR = 0x2211;
	MSP2->MSP_DR = 0x4433;

	MSP_EnableTx(MSP2);

	while (AIF->SAI2_RX1[0] == 0 && AIF->SAI2_RX1[1] == 0) ;

	FORi(8){
		g_buf_L[i] = AIF->SAI2_RX1[0]; //L
		g_buf_R[i] = AIF->SAI2_RX1[1]; //R
		while (g_buf_L[i] == AIF->SAI2_RX1[0] && g_buf_R[i] == AIF->SAI2_RX1[1]) ;
	}


	MSP_DisableTx(MSP2);
}

//---------------------------------------------------------

void TestMSP(void)
{
	PRINTF("TestMSP... ");

	//Audio Routing
	AUDMUX_CR = MUX_SAI2_TO_MSP2 | MUX_AUDREFCLK_TO_MSP2; // MUX_SAI4_TO_MSP1

//	Test_MSP2_2x16();
//	Test_MSP2_1x16();
//	test_MSP2_32_24valid(1);
//	test_MSP2_32_24valid(2);
//	test_MSP2_24(2);
	test_MSP2_8(2);

}

#endif //0

