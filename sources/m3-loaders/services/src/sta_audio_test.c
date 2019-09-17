/******************** (C) COPYRIGHT 2013 STMicroelectronics ********************
* File Name          : STA_audio_test.c
* Author             : APG-MID Application Team
* Date First Issued  : 05/28/2013
* Description        : This file provides audio test functions.
********************************************************************************
* History:
* 05/28/2013: V0.1
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

#include "sta_audio.h"
#include "trace.h"

#ifdef __cplusplus
extern "C" {
#endif

/** defines **/


/** Default MSP_configuration for I2S protocol */
struct msp_config msp_basecfg_i2s = {

  MSP_CLOCK_SEL_EXT,                    /* srg_clock_sel                 */
  MSP_CLOCK_POL_RISE,                   /* sck_pol                       */
  MSP_LOOPBACK_MODE_DISABLE,            /* msp_loopback_mode             */
  MSP_DIRECT_COMPANDING_MODE_DISABLE,   /* msp_direct_companding_mode    */
  MSP_CLOCK_SEL_INT,                    /* rx_clock_sel                  */
  MSP_CLOCK_SEL_INT,                    /* tx_clock_sel                  */
  MSP_MODE_DMA_ON,                     /* rx_msp_dma_mode               */
  MSP_MODE_DMA_ON,                     /* tx_msp_dma_mode               */
  MSP_FRAME_SEL_RX_INTERNAL,            /* rx_frame_sync_sel             */
  MSP_FRAME_SEL_GEN_LOGIC_PERIOD,       /* tx_frame_sync_sel             */
  MSP_UNEXPEC_FRAME_SYNC_IGNORED,       /* rx_unexpect_frame_sync        */
  MSP_UNEXPEC_FRAME_SYNC_IGNORED,       /* tx_unexpect_frame_sync        */
  MSP_FIFO_DISABLE,                     /* rx_fifo_config                */
  MSP_FIFO_DISABLE,                     /* tx_fifo_config                */
  MSP_TX_EXTRA_DELAY_OFF                /* tx_extra_delay                */
};

struct msp_protocol msp_baseprotocol_i2s = {

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

/** function declaration **/
void vAudioInit(t_msp *MSP_port);
void vAudioConfig(t_msp *MSP_port, Audio_Mode mode);
void vAudioPushData(t_msp * MSP_port, uint32_t src_address);
void vAudioPopData(t_msp * MSP_port, uint32_t * dst_address);
void vAudioStop(t_msp * MSP_port);
void vAudioStart(t_msp * MSP_port, Audio_Mode mode);

/** function definition **/
void vAudioInit(t_msp *MSP_port)
{
    TRACE_INFO("++Audio Init\n");

    msp_disable_tx_rx(MSP_port);
    msp_empty_tx_fifo(MSP_port);
    msp_empty_rx_fifo(MSP_port);
    msp_reset_reg(MSP_port);

    TRACE_INFO("--Audio Init\n");
}

void vAudioConfig(t_msp * MSP_port, Audio_Mode mode)
{
    struct msp_config      *msp_config;
    struct msp_protocol    *msp_protocol;

    TRACE_INFO("++vAudioConfig\n");

    msp_config    = &msp_basecfg_i2s;
    msp_protocol  = &msp_baseprotocol_i2s;

    msp_configure(MSP_port, msp_config, msp_protocol);

    switch(mode)
    {
        case DEFAULT_MODE:
            break;
        case MSP_LOOPBACK_MODE:
            msp_set_loop_back_mode(MSP_port, MSP_LOOPBACK_MODE_ENABLE);
            break;
        case SAI4_LOOPBACK_MODE:
            *((volatile uint32_t*) (0x48060000 + 0x1000)) |= 0x00000001; //MUX_CR MSP1 to SAI4
            *((volatile uint32_t*) (0x48D00000 + 0x8C0C)) |= 0x00800400; //SAI4 configuration
            break;
        case SAI4_BYPASS:
            *((volatile uint32_t*) (0x48060000 + 0x1000)) |= 0x00000001; //MUX_CR MSP1 to SAI4
            *((volatile uint32_t*) (0x48D00000 + 0x8C0C)) |= 0x00000400; //SAI4 configuration
            *((volatile uint32_t*) (0x48D00000 + 0x8800)) |= 0x0000000A; //INMUX CH0 to SAI4RX1
            *((volatile uint32_t*) (0x48D00000 + 0x8400)) &= ~0x3;       //LPF CH0 set to bypass
            *((volatile uint32_t*) (0x48D00000 + 0x8804)) |= 0x0000000A; //OUTMUX CH0 to SAI4TX1
            *((volatile uint32_t*) (0x48D00000 + 0x8000)) |= 0x00000040; //ASRC CH0 to bypass
            break;
        case SAI4_DMABUS:
            *((volatile uint32_t*) (0x48060000 + 0x1000)) |= 0x00000001; //MUX_CR MSP1 to SAI4
            *((volatile uint32_t*) (0x48D00000 + 0x8C0C)) |= 0x00000400; //SAI4 configuration
            *((volatile uint32_t*) (0x48D00000 + 0x8800)) |= 0x0000000A; //INMUX CH0 to SAI4RX1
            *((volatile uint32_t*) (0x48D00000 + 0x8400)) &= ~0x3;       //LPF CH0 set to bypass
            *((volatile uint32_t*) (0x48D00000 + 0x8804)) |= 0x0000000E; //OUTMUX CH0 to SRCDO0
            *((volatile uint32_t*) (0x48D00000 + 0x8000)) |= 0x00000040;
            break;
        default:
            break;
    }

    msp_config_sample_rate_gen(MSP_port, 52000000, 48000, 32, 16);

    TRACE_INFO("--vAudioConfig\n");
}

void vAudioPopData(t_msp * MSP_port, uint32_t * dst_address)
{
    TRACE_INFO("++vAudioPopData\n");

    *dst_address = msp_read_data(MSP_port);

    TRACE_INFO("--vAudioPopData\n");
}

void vAudioPushData(t_msp * MSP_port, uint32_t src_address)
{
    TRACE_INFO("++vAudioPopData\n");

    msp_write_data(MSP_port, src_address);

    TRACE_INFO("--vAudioPopData\n");
}

void vAudioStart(t_msp * MSP_port, Audio_Mode mode)
{
    TRACE_INFO("++vAudioStart\n");

    switch(mode)
    {
        case DEFAULT_MODE:
            break;
        case MSP_LOOPBACK_MODE:
            msp_enable_tx_rx(MSP_port);
            break;
        case SAI4_LOOPBACK_MODE:
            msp_enable_tx_rx(MSP_port);
            *((volatile uint32_t*) (0x48D00000 + 0x8C0C)) |= 0x1; //enable SAI4
            break;
        case SAI4_DMABUS:
            msp_enable_tx_rx(MSP_port);
            *((volatile uint32_t*) (0x48D00000 + 0x8C0C)) |= 0x1; //enable SAI4
            break;
        default:
            break;
    }

    TRACE_INFO("--vAudioStart\n");
}

void vAudioStop(t_msp * MSP_port)
{
    TRACE_INFO("++vAudioStop\n");

    msp_disable_tx_rx(MSP_port);

    TRACE_INFO("--vAudioStop\n");
}

#ifdef __cplusplus
}
#endif

// End of file
