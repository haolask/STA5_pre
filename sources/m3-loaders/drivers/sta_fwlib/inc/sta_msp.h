/**
 * @file sta_msp.h
 * @brief This file provides all the MSP firmware definitions
 *
 * Copyright (C); ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#ifndef _MSP_H_
#define _MSP_H_

#include "sta_map.h"

#define MSP_REGISTER_RESET_VALUE 0

#define MSP_DATA_TRANSFER_WIDTH_BYTE 0
#define MSP_DATA_TRANSFER_WIDTH_HALFWORD 1
#define MSP_DATA_TRANSFER_WIDTH_WORD 2

#define MSP_PHASE_MODE_SINGLE 0
#define MSP_PHASE_MODE_DUAL 1

/* For MSP0 HS controller STA_2062 */
#define MSP_IRQ_RECEIVE 0x1
#define MSP_IRQ_RECEIVE_OVERRUN_ERROR 0x2
#define MSP_IRQ_RECEIVE_FRAME_SYNC_ERROR 0x4
#define MSP_IRQ_RECEIVE_FRAME_SYNC 0x8
#define MSP_IRQ_TRANSMIT 0x10
#define MSP_IRQ_TRANSMIT_OVERRUN_ERROR 0x20
#define MSP_IRQ_TRANSMIT_FRAME_SYNC_ERROR 0x40
#define MSP_IRQ_TRANSMIT_FRAME_SYNC 0x80
#define MSP_IRQ_RX_CLEARABLE 0x0F
#define MSP_IRQ_TX_CLEARABLE 0xF0
#define MSP_IRQ_ALL_CLEARABLE 0xFF
#define MSP_IRQ_RECEIVE_FIFO_NOT_EMPTY 0x100
#define MSP_IRQ_TRANSMIT_FIFO_NOT_FULL 0x200

#define MSP_FRAME_SYNC_POL_HIGH 0
#define MSP_FRAME_SYNC_POL_LOW 1

#define MSP_FRAME_SEL_TX_EXTERNAL 0x0
#define MSP_FRAME_SEL_GEN_LOGIC 0x2
#define MSP_FRAME_SEL_GEN_LOGIC_PERIOD 0x3

#define MSP_FRAME_SEL_RX_EXTERNAL 0x0
#define MSP_FRAME_SEL_RX_INTERNAL 0x1

#define MSP_CLOCK_POL_FALL 0
#define MSP_CLOCK_POL_RISE 1

#define MSP_CLOCK_SEL_EXT 0x0
#define MSP_CLOCK_SEL_INT 0x1

#define MSP_CLOCK_FREE_EXT 0x2
#define MSP_CLOCK_RESYNCRO_EXT 0x3

#define MSP_NO_SPICLOCK 0x0
#define MSP_ZERO_DELAY_SPICLOCK 0x2
#define MSP_HALF_DELAY_SPICLOCK 0x3

#define MSP_SPI_BURST_MODE_DISABLE 0
#define MSP_SPI_BURST_MODE_ENABLE 1

#define MSP_8_BIT 0x0
#define MSP_10_BIT 0x1
#define MSP_12_BIT 0x2
#define MSP_14_BIT 0x3
#define MSP_16_BIT 0x4
#define MSP_20_BIT 0x5
#define MSP_24_BIT 0x6
#define MSP_32_BIT 0x7

#define MSP_NO_COMPANDING 0x0
#define MSP_SIGNED_COMPANDING 0x1
#define MSP_uLAW_COMPANDING 0x2
#define MSP_ALAW_COMPANDING 0x3

#define MSP_MSB_FIRST 0
#define MSP_LSB_FIRST 1

#define MSP_0_CLOCK_DELAY 0
#define MSP_1_CLOCK_DELAY 1
#define MSP_2_CLOCK_DELAY 2
#define MSP_3_CLOCK_DELAY 3

#define MSP_2PH_IMMEDIATE 0
#define MSP_2PH_DELAY 1

#define MSP_NO_SWAP 0
#define MSP_WORD_SWAP 1
#define MSP_EACH_HALFWORD_SWAP 2
#define MSP_HALFWORD_SWA 3

#define MSP_FLAG_RECEIVE_BUSY 0x1
#define MSP_FLAG_RECEIVE_FIFOEMPTY 0x2
#define MSP_FLAG_RECEIVE_FIFOFULL 0x4
#define MSP_FLAG_TRANSMIT_BUSY 0x8
#define MSP_FLAG_TRANSMIT_FIFOEMPTY 0x10
#define MSP_FLAG_TRANSMIT_FIFOFULL 0x20

#define MSP_SUBFRAME_0_TO_31 0
#define MSP_SUBFRAME_32_TO_63 1
#define MSP_SUBFRAME_64_TO_95 2
#define MSP_SUBFRAME_98_TO_127 3

#define MSP_MCH_COMP_DISABLED 0x0
#define MSP_MCH_COMP_b2b_FALSE 0x2
#define MSP_MCH_COMP_b2b_TRUE 0x3

#define MSP_LOOPBACK_MODE_DISABLE 0
#define MSP_LOOPBACK_MODE_ENABLE 1

#define MSP_DIRECT_COMPANDING_MODE_DISABLE 0
#define MSP_DIRECT_COMPANDING_MODE_ENABLE 1

#define MSP_MODE_DMA_OFF 0
#define MSP_MODE_DMA_ON 1

#define MSP_UNEXPEC_FRAME_SYNC_ABORT 0
#define MSP_UNEXPEC_FRAME_SYNC_IGNORED 1

#define MSP_FIFO_DISABLE 0
#define MSP_FIFO_ENABLE 1

#define MSP_TX_EXTRA_DELAY_OFF 0
#define MSP_TX_EXTRA_DELAY_ON 1

struct msp_protocol {
	int rx_data_transfer_width;
	int tx_data_transfer_width;
	int rx_phase_mode;
	int tx_phase_mode;
	int rx_phase2_start_mode;
	int tx_phase2_start_mode;
	int rx_bit_transfer_format;
	int tx_bit_transfer_format;
	uint8_t rx_frame_length_1;
	uint8_t rx_frame_length_2;
	uint8_t tx_frame_length_1;
	uint8_t tx_frame_length_2;
	int rx_element_length_1;
	int rx_element_length_2;
	int tx_element_length_1;
	int tx_element_length_2;
	int rx_data_delay;
	int tx_data_delay;
	int rx_clock_pol;
	int tx_clock_pol;
	int rx_frame_sync_pol;
	int tx_frame_sync_pol;
	int rx_half_word_swap;
	int tx_half_word_swap;
	int compression_mode;
	int expansion_mode;
	int spi_clk_mode;
	int spi_burst_mode;
	uint16_t frame_period;
	uint16_t frame_width;
	uint16_t total_clocks_for_one_frame;
};

struct msp_config {
	int srg_clock_sel;
	int sck_pol;
	int msp_loopback_mode;
	int msp_direct_companding_mode;
	int rx_clock_sel;
	int tx_clock_sel;
	int rx_msp_dma_mode;
	int tx_msp_dma_mode;
	int rx_frame_sync_sel;
	int tx_frame_sync_sel;
	int rx_unexpect_frame_sync;
	int tx_unexpect_frame_sync;
	int rx_fifo_config;
	int tx_fifo_config;
	int tx_extra_delay;
};

/**
 * @brief	Reset registers values
 * @param	msp: MSP register base address
 * @return	none
 */
void msp_reset_reg(t_msp *msp);

/**
 * @brief	Configure MSP device
 * @param	msp: MSP register base address
 * @param	cfg: MSP configuration
 * @param	prot_desc: MSP protocol descriptor
 * @return	none
 */
void msp_configure(t_msp *msp, struct msp_config *cfg,
		   struct msp_protocol *prot_desc);

/**
 * @brief	Write data in DATA register
 * @param	msp: MSP register base address
 * @return	none
 */
void msp_write_data(t_msp *msp, uint32_t data);

/**
 * @brief	Read data from DATA register
 * @param	msp: MSP register base address
 * @return	DATA register content
 */
uint32_t msp_read_data(t_msp *msp);

/**
 * @brief	Configure the Rx FIFO mode (enabled/disabled);
 * @param	msp: MSP register base address
 * @param	mode: MSP_FIFO_DISABLE or MSP_FIFO_ENABLE
 * @return	0 if no error, not 0 otherwise
 */
void msp_set_rx_fifo_mode(t_msp *msp, int mode);

/**
 * @brief	Provide the Rx FIFO mode (enabled/disabled);
 * @param	msp: MSP register base address
 * @return	Rx fifo mode
 */
int msp_get_rx_fifo_mode(t_msp *msp);

/**
 * @brief	Set RX frame synchro polarity
 * @param	msp: MSP register base address
 * @param	polarity: chosen polarity
 * @return	0 if no error, not 0 otherwise
 */
void msp_set_rx_frame_syncro_polarity(t_msp *msp,
				  int polarity);

/**
 * @brief	Provide RX frame synchro polarity
 * @param	msp: MSP register base address
 * @return	polarity: chosen polarity
 */
int msp_get_rx_frame_syncro_polarity(t_msp *msp);

/**
 * @brief	Set Direct Companding Mode
 * @param	msp: MSP register base address
 * @param	mode: chosen mode
 * @return	0 if no error, not 0 otherwise
 */
void msp_set_dir_comp_mode(t_msp *msp, int mode);

/**
 * @brief	Get Direct Companding Mode
 * @param	msp: MSP register base address
 * @return	mode: chosen mode
 */
int msp_get_dir_comp_mode(t_msp *msp);

/**
 * @brief	Set RX frame synchro selection
 * @param	msp: MSP register base address
 * @param	selection: external or internal source
 * @return	0 if no error, not 0 otherwise
 */
void msp_set_rx_frame_syncro_selection(t_msp *msp, int selection);

/**
 * @brief	Get RX frame synchro selection
 * @param	msp: MSP register base address
 * @return	selection mode (see Tx for details);
 */
int msp_get_rx_frame_syncro_selection(t_msp *msp);

/**
 * @brief	Set RX clock polarity
 * @param	msp: MSP register base address
 * @param	polarity: chosen polarity
 * @return	0 if no error, not 0 otherwise
 */
void msp_set_rx_clock_polarity(t_msp *msp, int polarity);

/**
 * @brief	Get RX clock polarity
 * @param	msp: MSP register base address
 * @return	polarity: chosen polarity
 */
int msp_get_rx_clock_polarity(t_msp *msp);

/**
 * @brief	Set RX clock
 * @param	msp: MSP register base address
 * @param	clock: clock value
 * @return	0 if no error, not 0 otherwise
 */
void msp_set_rx_clock(t_msp *msp, int clock);

/**
 * @brief	Get RX clock
 * @param	msp: MSP register base address
 * @return	clock: clock value
 */
int msp_get_rx_clock(t_msp *msp);

/**
 * @brief	Set Loopback mode
 * @param	msp: MSP register base address
 * @param	mode: mode value
 * @return	0 if no error, not 0 otherwise
 */
void msp_set_loop_back_mode(t_msp *msp, int mode);

/**
 * @brief	Get Loopback mode
 * @param	msp: MSP register base address
 * @return	mode: mode value
 */
int msp_get_loop_back_mode(t_msp *msp);

/**
 * @brief	Configure the Tx FIFO mode (enabled/disabled);
 * @param	msp: MSP register base address
 * @param	mode: MSP_FIFO_DISABLE or MSP_FIFO_ENABLE
 * @return	0 if no error, not 0 otherwise
 */
void msp_set_tx_fifo_mode(t_msp *msp, int mode);

/**
 * @brief	Provide the Tx FIFO mode (enabled/disabled);
 * @param	msp: MSP register base address
 * @return	Rx fifo mode
 */
int msp_get_tx_fifo_mode(t_msp *msp);

/**
 * @brief	Set TX frame synchro polarity
 * @param	msp: MSP register base address
 * @param	polarity: chosen polarity
 * @return	0 if no error, not 0 otherwise
 */
void msp_set_tx_frame_syncro_polarity(t_msp *msp,
				  int polarity);

/**
 * @brief	Provide TX frame synchro polarity
 * @param	msp: MSP register base address
 * @return	polarity: chosen polarity
 */
int msp_get_tx_frame_syncro_polarity(t_msp *msp);

/**
 * @brief	Set TX frame synchro selection
 * @param	msp: MSP register base address
 * @param	selection: external or internal source
 * @return	0 if no error, not 0 otherwise
 */
void msp_set_tx_frame_syncro_selection(t_msp *msp, int selection);

/**
 * @brief	Get TX frame synchro selection
 * @param	msp: MSP register base address
 * @return	selection mode (see Tx for details);
 */
int msp_get_tx_frame_syncro_selection(t_msp *msp);

/**
 * @brief	Set TX clock polarity
 * @param	msp: MSP register base address
 * @param	polarity: chosen polarity
 * @return	0 if no error, not 0 otherwise
 */
void msp_set_tx_clock_polarity(t_msp *msp, int polarity);

/**
 * @brief	Get TX clock polarity
 * @param	msp: MSP register base address
 * @return	polarity: chosen polarity
 */
int msp_get_tx_clock_polarity(t_msp *msp);

/**
 * @brief	Set TX clock
 * @param	msp: MSP register base address
 * @param	clock: clock value
 * @return	0 if no error, not 0 otherwise
 */
void msp_set_tx_clock(t_msp *msp, int clock);

/**
 * @brief	Get TX clock
 * @param	msp: MSP register base address
 * @return	clock: clock value
 */
int msp_get_tx_clock(t_msp *msp);

/**
 * @brief	Set TX Extra delay mode
 * @param	msp: MSP register base address
 * @param	mode: mode value
 * @return	0 if no error, not 0 otherwise
 */
void msp_set_tx_data_extra_delay_mode(t_msp *msp, int mode);

/**
 * @brief	Get TX Extra delay mode
 * @param	msp: MSP register base address
 * @return	mode: mode value
 */
int msp_get_tx_data_extra_delay_mode(t_msp *msp);

/**
 * @brief	Disable sampling rate generation
 * @param	msp: MSP register base address
 * @return	none
 */
void msp_dis_sample_rate_gen(t_msp *msp);

/**
 * @brief	Enable sampling rate generation
 * @param	msp: MSP register base address
 * @return	none
 */
void msp_en_sample_rate_gen(t_msp *msp);

/**
 * @brief	Set sample generator Clock Polarity
 * @param	msp: MSP register base address
 * @param	polarity: chosen polarity
 * @return	0 if no error, not 0 otherwise
 */
void msp_set_sample_rate_gen_clock_polarity(t_msp *msp,
				       int polarity);

/**
 * @brief	Get sample generator clock polarity
 * @param	msp: MSP register base address
 * @return	polarity: chosen polarity
 */
int msp_get_sample_rate_gen_clock_polarity(t_msp *msp);

/**
 * @brief	Set sample generator clock
 * @param	msp: MSP register base address
 * @param	clock: clock value
 * @return	0 if no error, not 0 otherwise
 */
void msp_set_sample_rate_gen_clock(t_msp *msp, int clock);

/**
 * @brief	Get sample generator clock
 * @param	msp: MSP register base address
 * @return	clock: clock value
 */
int msp_get_sample_rate_gen_clock(t_msp *msp);

/**
 * @brief	Disable frame generation
 * @param	msp: MSP register base address
 * @return	none
 */
void msp_dis_frame_gen(t_msp *msp);

/**
 * @brief	Enable frame generation
 * @param	msp: MSP register base address
 * @return	none
 */
void msp_en_frame_gen(t_msp *msp);

/**
 * @brief	Set SPI clock mode
 * @param	msp: MSP register base address
 * @param	mode: mode value
 * @return	none
 */
void msp_set_spi_clock_mode(t_msp *msp, int mode);

/**
 * @brief	Get SPI clock mode
 * @param	msp: MSP register base address
 * @return	mode: mode value
 */
int msp_get_spi_clock_mode(t_msp *msp);

/**
 * @brief	Set SPI burst mode
 * @param	msp: MSP register base address
 * @param	mode: mode value
 * @return	none
 */
void msp_set_spi_burst_mode(t_msp *msp, int mode);

/**
 * @brief	Get SPI burst mode
 * @param	msp: MSP register base address
 * @return	mode: mode value
 */
int msp_get_spi_burst_mode(t_msp *msp);

/**
 * @brief	Set Tx Element Length phase1
 * @param	msp: MSP register base address
 * @param	length: length value
 */
void msp_set_tx_elem_length1_ph(t_msp *msp, int length);

/**
 * @brief	Get Tx Element Length phase1
 * @param	msp: MSP register base address
 * @return	length: length value
 */
int msp_get_tx_elem_length1_ph(t_msp *msp);

/**
 * @brief	Set Tx Element Number phase1
 * @param	msp: MSP register base address
 * @param	amount: amount value
 */
void msp_set_tx_elem_numb1_ph(t_msp *msp, uint8_t amount);

/**
 * @brief	Get Tx Element Length phase1
 * @param	msp: MSP register base address
 * @return	amount: amount value
 */
uint8_t msp_get_tx_elem_numb1_ph(t_msp *msp);

/**
 * @brief	Set Tx data type
 * @param	msp: MSP register base address
 * @return	type: type value
 */
void msp_set_tx_data_type(t_msp *msp, int type);

/**
 * @brief	Get Tx data type
 * @param	msp: MSP register base address
 * @return	type: type value
 */
int msp_get_tx_data_type(t_msp *msp);

/**
 * @brief	Set Tx bit endian format
 * @param	msp: MSP register base address
 * @param	type: type value
 */
void msp_set_tx_endian_form(t_msp *msp, int type);

/**
 * @brief	Get Tx bit endian format
 * @param	msp: MSP register base address
 * @return	type: type value
 */
int msp_get_tx_endian_form(t_msp *msp);

/**
 * @brief	Set Tx data delay
 * @param	msp: MSP register base address
 * @param	type: type value
 */
void msp_set_tx_data_delay(t_msp *msp, int delay);

/**
 * @brief	Get Tx data delay
 * @param	msp: MSP register base address
 * @return	type: type value
 */
int msp_get_tx_data_delay(t_msp *msp);

/**
 * @brief	Set Tx unexpected frame bit endian format
 * @param	msp: MSP register base address
 * @param	mode: mode value
 */
void msp_set_tx_unexp_frame_sync_mode(t_msp *msp,
				 int mode);

/**
 * @brief	Get Tx unexpected frame bit endian format
 * @param	msp: MSP register base address
 * @return	mode: mode value
 */
int msp_get_tx_unexp_frame_sync_mode(t_msp *msp);

/**
 * @brief	Set Tx Element Length phase2
 * @param	msp: MSP register base address
 * @param	length: length value
 */
void msp_set_tx_elem_length2_ph(t_msp *msp, int length);

/**
 * @brief	Get Tx Element Length phase2
 * @param	msp: MSP register base address
 * @return	length: length value
 */
int msp_get_tx_elem_length2_ph(t_msp *msp);

/**
 * @brief	Set Tx Element Number phase2
 * @param	msp: MSP register base address
 * @param	amount: amount value
 */
void msp_set_tx_elem_numb2_ph(t_msp *msp, uint8_t amount);

/**
 * @brief	Get Tx Element Length phase2
 * @param	msp: MSP register base address
 * @return	amount: amount value
 */
uint8_t msp_get_tx_elem_numb2_ph(t_msp *msp);

/**
 * @brief	Set Tx Start Mode
 * @param	msp: MSP register base address
 * @param	mode: mode value
 */
void msp_set_tx_start_mode2_ph(t_msp *msp, int mode);

/**
 * @brief	Get Tx Start Mode
 * @param	msp: MSP register base address
 * @return	mode: mode value
 */
int msp_get_tx_start_mode2_ph(t_msp *msp);

/**
 * @brief	Set Tx phase Mode
 * @param	msp: MSP register base address
 * @param	mode: mode value
 */
void msp_set_tx2_ph(t_msp *msp, int mode);

/**
 * @brief	Get Tx phase Mode
 * @param	msp: MSP register base address
 * @return	mode: mode value
 */
int msp_get_tx2_ph(t_msp *msp);

/**
 * @brief	Set Tx half word swap
 * @param	msp: MSP register base address
 * @param	swap: swap value
 */
void msp_set_tx_half_word_swap(t_msp *msp, int swap);

/**
 * @brief	Get Tx half word swap
 * @param	msp: MSP register base address
 * @return	swap: swap value
 */
int msp_get_tx_half_word_swap(t_msp *msp);

/**
 * @brief	Set Rx Element Length phase1
 * @param	msp: MSP register base address
 * @param	length: length value
 */
void msp_set_rx_elem_length1_ph(t_msp *msp, int length);

/**
 * @brief	Get Tx Element Length phase1
 * @param	msp: MSP register base address
 * @return	length: length value
 */
int msp_get_rx_elem_length1_ph(t_msp *msp);

/**
 * @brief	Set Rx Element Number phase1
 * @param	msp: MSP register base address
 * @param	amount: amount value
 */
void msp_set_rx_elem_numb1_ph(t_msp *msp, uint8_t amount);

/**
 * @brief	Get Rx Element Length phase1
 * @param	msp: MSP register base address
 * @return	amount: amount value
 */
uint8_t msp_get_rx_elem_numb1_ph(t_msp *msp);

/**
 * @brief	Set Rx data type
 * @param	msp: MSP register base address
 * @return	type: type value
 */
void msp_set_rx_data_type(t_msp *msp, int type);

/**
 * @brief	Get Rx data type
 * @param	msp: MSP register base address
 * @return	type: type value
 */
int msp_get_rx_data_type(t_msp *msp);

/**
 * @brief	Set Rx bit endian format
 * @param	msp: MSP register base address
 * @param	type: type value
 */
void msp_set_rx_endian_form(t_msp *msp, int type);

/**
 * @brief	Get Rx bit endian format
 * @param	msp: MSP register base address
 * @return	type: type value
 */
int msp_get_rx_endian_form(t_msp *msp);

/**
 * @brief	Set Rx data delay
 * @param	msp: MSP register base address
 * @param	type: type value
 */
void msp_set_rx_data_delay(t_msp *msp, int delay);

/**
 * @brief	Get Rx data delay
 * @param	msp: MSP register base address
 * @return	type: type value
 */
int msp_get_rx_data_delay(t_msp *msp);

/**
 * @brief	Set Rx unexpected frame bit endian format
 * @param	msp: MSP register base address
 * @param	mode: mode value
 */
void msp_set_rx_unexp_frame_sync_mode(t_msp *msp,
				 int mode);

/**
 * @brief	Get Rx unexpected frame bit endian format
 * @param	msp: MSP register base address
 * @return	mode: mode value
 */
int msp_get_rx_unexp_frame_sync_mode(t_msp *msp);

/**
 * @brief	Set Rx Element Length phase2
 * @param	msp: MSP register base address
 * @param	length: length value
 */
void msp_set_rx_elem_length2_ph(t_msp *msp, int length);

/**
 * @brief	Get Rx Element Length phase2
 * @param	msp: MSP register base address
 * @return	length: length value
 */
int msp_get_rx_elem_length2_ph(t_msp *msp);

/**
 * @brief	Set Rx Element Number phase2
 * @param	msp: MSP register base address
 * @param	amount: amount value
 */
void msp_set_rx_elem_numb2_ph(t_msp *msp, uint8_t amount);

/**
 * @brief	Get Tx Element Length phase2
 * @param	msp: MSP register base address
 * @return	amount: amount value
 */
uint8_t msp_get_rx_elem_numb2_ph(t_msp *msp);

/**
 * @brief	Set Rx Start Mode
 * @param	msp: MSP register base address
 * @param	mode: mode value
 */
void msp_set_rx_start_mode2_ph(t_msp *msp, int mode);

/**
 * @brief	Get Rx Start Mode
 * @param	msp: MSP register base address
 * @return	mode: mode value
 */
int msp_get_rx_start_mode2_ph(t_msp *msp);

/**
 * @brief	Set Rx phase Mode
 * @param	msp: MSP register base address
 * @param	mode: mode value
 */
void msp_set_rx2_ph(t_msp *msp, int mode);

/**
 * @brief	Get Rx phase Mode
 * @param	msp: MSP register base address
 * @return	mode: mode value
 */
int msp_get_rx2_ph(t_msp *msp);

/**
 * @brief	Set Rx half word swap
 * @param	msp: MSP register base address
 * @param	swap: swap value
 */
void msp_set_rx_half_word_swap(t_msp *msp, int swap);

/**
 * @brief	Get Rx half word swap
 * @param	msp: MSP register base address
 * @return	swap: swap value
 */
int msp_get_rx_half_word_swap(t_msp *msp);

/**
 * @brief	Configure sample rate generator
 * @param	msp: MSP register base address
 * @return	none
 */
void msp_config_sample_rate_gen(t_msp *msp, uint32_t ClockFreq, uint32_t FrameFreq,
			     uint32_t DataClock, uint8_t width);

/**
 * @brief	Get sample freq
 * @param	msp: MSP register base address
 * @return	none
 */
void msp_get_sample_freq(t_msp *msp, uint32_t ClockFreq, uint32_t * SampleFreq,
		       uint32_t * period);

/**
 * @brief	Set active frame width
 * @param	msp: MSP register base address
 * @param	frame width
 */
void msp_set_active_frame_width(t_msp *msp, uint8_t width);

/**
 * @brief	Get active frame width
 * @param	msp: MSP register base address
 * @return	frame width
 */
uint8_t msp_get_frame_width(t_msp *msp);

/**
 * @brief	Set frame period
 * @param	msp: MSP register base address
 * @param	period
 */
void msp_set_frame_period(t_msp *msp, uint16_t period);

/**
 * @brief	Get frame period
 * @param	msp: MSP register base address
 * @return	period
 */
uint16_t msp_get_frame_period(t_msp *msp);

/**
 * @brief	Get flag status
 * @param	msp: MSP register base address
 * @param	flag
 * @return	flag status
 */
bool msp_get_flag_status(t_msp *msp, uint32_t flag);

/**
 * @brief	Set Tx DMA mode
 * @param	msp: MSP register base address
 * @param	mode
 */
void msp_set_tx_dma_mode(t_msp *msp, int mode);

/**
 * @brief	Get Tx DMA mode
 * @param	msp: MSP register base address
 * @return	dma mode
 */
int msp_get_tx_dma_mode(t_msp *msp);

/**
 * @brief	Set Rx DMA mode
 * @param	msp: MSP register base address
 * @param	mode
 */
void msp_set_rx_dma_mode(t_msp *msp, int mode);

/**
 * @brief	Get Rx DMA mode
 * @param	msp: MSP register base address
 * @return	dma mode
 */
int msp_get_rx_dma_mode(t_msp *msp);

/**
 * @brief	irq enable
 * @param	msp: MSP register base address
 * @param	irq
 */
void msp_irq_enable(t_msp *msp, int irq);

/**
 * @brief	irq disable
 * @param	msp: MSP register base address
 * @param	irq
 */
void msp_irq_disable(t_msp *msp, int irq);

uint16_t msp_get_irq_config(t_msp *msp);

uint16_t msp_get_raw_irq_status(t_msp *msp);

uint16_t msp_get_irq_status(t_msp *msp);

void msp_irq_clear(t_msp *msp, int irq);

void msp_en_rx_multi_ch(t_msp *msp);

void msp_dis_rx_multi_ch(t_msp *msp);

int msp_get_rx_multi_ch_sub_frame(t_msp *msp);

void msp_set_multi_ch_compare_md(t_msp *msp, int mode);

int msp_get_multi_ch_compare_md(t_msp *msp);

void msp_en_tx_multi_ch(t_msp *msp);

void msp_dis_tx_multi_ch(t_msp *msp);

int msp_get_tx_multi_ch_sub_frame(t_msp *msp);

/**
 * @brief	Compare Rx channel register
 * @param	msp: MSP register base address
 * @return	RCM value
 */
void msp_set_rx_multi_compare_reg(t_msp *msp, uint32_t mask);

/**
 * @brief	Get Rx channel register
 * @param	msp: MSP register base address
 * @return	RCM value
 */
uint32_t msp_get_rx_multi_compare_reg(t_msp *msp);

/**
 * @brief	Enable Rx channel register
 * @param	msp: MSP register base address
 * @param	channel
 */
void msp_en_rx_pin_multi_ch_compare(t_msp *msp, uint8_t channel);

/**
 * @brief	Disable Rx channel register
 * @param	msp: MSP register base address
 * @param	channel
 */
void msp_dis_rx_pin_multi_ch_compare(t_msp *msp, uint8_t channel);

/**
 * @brief	Compare Rx channel register
 * @param	msp: MSP register base address
 * @return	RCM value
 */
void msp_set_rx_multi_compare_mask(t_msp *msp, uint32_t mask);

/**
 * @brief	Get Rx channel register
 * @param	msp: MSP register base address
 * @return	RCM value
 */
uint32_t msp_get_rx_multi_compare_mask(t_msp *msp);

/**
 * @brief	Enable Rx channel register
 * @param	msp: MSP register base address
 * @param	channel
 */
void msp_en_rx_pin_multi_ch_compareMask(t_msp *msp, uint8_t channel);

/**
 * @brief	Disable Rx channel register
 * @param	msp: MSP register base address
 * @param	channel
 */
void msp_dis_rx_pin_multi_ch_compareMask(t_msp *msp, uint8_t channel);

/**
 * @brief	Set enable channel register
 * @param	msp: MSP register base address
 * @param	bank: bank to be addressed
 * @param	reg: new register value
 */
void msp_set_tx_multi_ch_enable(t_msp *msp, uint8_t bank, uint32_t reg);

/**
 * @brief	Get enable channel register
 * @param	msp: MSP register base address
 * @param	bank: bank to be addressed
 */
uint32_t msp_get_tx_multi_ch_enable(t_msp *msp, uint8_t bank);

/**
 * @brief	Enable channel
 * @param	msp: MSP register base address
 * @param	channel: channel to be enabled
 */
void msp_en_pin_tx_multi_ch(t_msp *msp, uint8_t channel);

/**
 * @brief	Disable channel
 * @param	msp: MSP register base address
 * @param	channel: channel to be disabled
 */
void msp_dis_pin_tx_multi_ch(t_msp *msp, uint8_t channel);

/**
 * @brief	Set enable channel register
 * @param	msp: MSP register base address
 * @param	bank: bank to be addressed
 * @param	reg: new register value
 */
void msp_set_rx_multi_ch_enable(t_msp *msp, uint8_t bank, uint32_t reg);

/**
 * @brief	Get enable channel register
 * @param	msp: MSP register base address
 * @param	bank: bank to be addressed
 */
uint32_t msp_get_rx_multi_ch_enable(t_msp *msp, uint8_t bank);

/**
 * @brief	Enable channel
 * @param	msp: MSP register base address
 * @param	channel: channel to be enabled
 */
void msp_en_pin_rx_multi_ch(t_msp *msp, uint8_t channel);

/**
 * @brief	Disable channel
 * @param	msp: MSP register base address
 * @param	channel: channel to be disabled
 */
void msp_dis_pin_rx_multi_ch(t_msp *msp, uint8_t channel);

/**
 * @brief	Wait Tx completion
 * @param	msp: MSP register base address
 */
void msp_wait_tx_complete(t_msp *msp);

/**
 * @brief	Wait Rx completion
 * @param	msp: MSP register base address
 */
void msp_wait_rx_complete(t_msp *msp);

/**
 * @brief	Empty Rx Fifo
 * @param	msp: MSP register base address
 */
void msp_empty_rx_fifo(t_msp *msp);

/**
 * @brief	Empty Tx Fifo
 * @param	msp: MSP register base address
 */
void msp_empty_tx_fifo(t_msp *msp);

/**
 * @brief	Disable Rx and Tx
 * @param	msp: MSP register base address
 */
void msp_disable_tx_rx(t_msp *msp);

/**
 * @brief	Disable Tx
 * @param	msp: MSP register base address
 */
void msp_disable_tx(t_msp *msp);

/**
 * @brief	Disable Rx
 * @param	msp: MSP register base address
 */
void msp_disable_rx(t_msp *msp);

/**
 * @brief	Enable Tx and Rx
 * @param	msp: MSP register base address
 */
void msp_enable_tx_rx(t_msp *msp);

/**
 * @brief	Enable Tx
 * @param	msp: MSP register base address
 */
void msp_enable_tx(t_msp *msp);

/**
 * @brief	Enable Rx
 * @param	msp: MSP register base address
 */
void msp_enable_rx(t_msp *msp);

/**
 * @brief	MSP irq handler SPI burst mode
 */
void msp0_irq_handler(void);

/**
 * @brief	MSP irq handler SPI burst mode
 */
void msp1_irq_handler(void);

/**
 * @brief	MSP irq handler SPI burst mode
 */
void msp2_irq_handler(void);

#endif /* _MSP_H_ */
